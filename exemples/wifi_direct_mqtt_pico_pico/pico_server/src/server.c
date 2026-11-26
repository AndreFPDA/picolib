#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "pico/error.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "dhcp_server.h"
#include "wifi_ap_config.h"
#include "led_command.h"

#define PEER_PORT 4242
#define SERIAL_LINE_SIZE 16

static bool led_on;
static bool pending_state;
static struct tcp_pcb *peer_pcb;
static char peer_line[SERIAL_LINE_SIZE];
static size_t peer_line_length;
static char serial_line[SERIAL_LINE_SIZE];
static size_t serial_line_length;
static bool serial_line_overflow;

static void set_led(led_command_t command, const char *source) {
    if (command == LED_COMMAND_ON) {
        led_on = true;
    } else if (command == LED_COMMAND_OFF) {
        led_on = false;
    } else if (command == LED_COMMAND_TOGGLE) {
        led_on = !led_on;
    } else {
        return;
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
    printf("%s: onboard LED %s\n", source, led_on ? "ON" : "OFF");
}

static void send_led_state(void) {
    if (peer_pcb == NULL) {
        pending_state = true;
        printf("Peer is not connected; latest LED state queued\n");
        return;
    }

    char message[10];
    int length = snprintf(
        message, sizeof(message), "%s\n",
        led_command_text(led_on ? LED_COMMAND_ON : LED_COMMAND_OFF));
    err_t error = tcp_write(peer_pcb, message, (u16_t)length, TCP_WRITE_FLAG_COPY);
    if (error == ERR_OK) {
        error = tcp_output(peer_pcb);
    }
    if (error != ERR_OK) {
        pending_state = true;
        printf("Wi-Fi send failed: %d\n", error);
    } else {
        pending_state = false;
    }
}

static void handle_serial_line(void) {
    serial_line[serial_line_length] = '\0';
    led_command_t command = led_command_parse(serial_line);
    if (command == LED_COMMAND_INVALID) {
        printf("Invalid command. Use ON, OFF, or TOGGLE.\n");
    } else {
        set_led(command, "USB serial");
        send_led_state();
    }
    serial_line_length = 0;
}

static void read_serial(void) {
    int character;
    while ((character = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        if (character == '\r') {
            continue;
        }
        if (character == '\n') {
            if (serial_line_overflow) {
                printf("Serial command is too long\n");
            } else {
                handle_serial_line();
            }
            serial_line_length = 0;
            serial_line_overflow = false;
        } else if (!serial_line_overflow) {
            if (serial_line_length < sizeof(serial_line) - 1) {
                serial_line[serial_line_length++] = (char)character;
            } else {
                serial_line_overflow = true;
            }
        }
    }
}

static void handle_peer_line(void) {
    peer_line[peer_line_length] = '\0';
    led_command_t command = led_command_parse(peer_line);
    if (command == LED_COMMAND_INVALID) {
        printf("Ignored invalid command received over Wi-Fi\n");
    } else {
        set_led(command, "Wi-Fi");
    }
    peer_line_length = 0;
}

static void peer_error(void *arg, err_t error) {
    (void)arg;
    printf("Wi-Fi peer disconnected: %d\n", error);
    peer_pcb = NULL;
    peer_line_length = 0;
}

static err_t peer_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t error) {
    (void)arg;
    if (error != ERR_OK) {
        if (p != NULL) {
            pbuf_free(p);
        }
        return error;
    }
    if (p == NULL) {
        peer_pcb = NULL;
        peer_line_length = 0;
        tcp_arg(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        err_t close_error = tcp_close(pcb);
        if (close_error != ERR_OK) {
            tcp_abort(pcb);
            return ERR_ABRT;
        }
        printf("Wi-Fi peer disconnected\n");
        return ERR_OK;
    }

    for (u16_t offset = 0; offset < p->tot_len; offset++) {
        char character;
        pbuf_copy_partial(p, &character, 1, offset);
        if (character == '\n') {
            handle_peer_line();
        } else if (character != '\r') {
            if (peer_line_length < sizeof(peer_line) - 1) {
                peer_line[peer_line_length++] = character;
            } else {
                peer_line_length = 0;
                printf("Discarded oversized Wi-Fi command\n");
            }
        }
    }
    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

static err_t peer_accept(void *arg, struct tcp_pcb *pcb, err_t error) {
    (void)arg;
    if (error != ERR_OK || pcb == NULL) {
        return ERR_VAL;
    }
    if (peer_pcb != NULL) {
        tcp_abort(pcb);
        return ERR_ABRT;
    }

    peer_pcb = pcb;
    peer_line_length = 0;
    tcp_arg(pcb, NULL);
    tcp_err(pcb, peer_error);
    tcp_recv(pcb, peer_recv);
    printf("Wi-Fi client connected\n");
    if (pending_state) {
        send_led_state();
    }
    return ERR_OK;
}

static bool peer_server_open(void) {
    struct tcp_pcb *server = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (server == NULL) {
        printf("Failed to create TCP server\n");
        return false;
    }

    err_t error = tcp_bind(server, IP_ADDR_ANY, PEER_PORT);
    if (error != ERR_OK) {
        printf("Failed to bind TCP port %d: %d\n", PEER_PORT, error);
        tcp_close(server);
        return false;
    }

    struct tcp_pcb *listener = tcp_listen(server);
    if (listener == NULL) {
        printf("Failed to listen for Wi-Fi client\n");
        tcp_close(server);
        return false;
    }
    tcp_accept(listener, peer_accept);
    printf("TCP peer server listening on port %d\n", PEER_PORT);
    return true;
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);

    if (cyw43_arch_init()) {
        printf("Failed to initialize the CYW43 driver\n");
        return 1;
    }
    if (strlen(AP_SSID) == 0 || strlen(AP_SSID) > 32 ||
        strlen(AP_PASSWORD) < 8 || strlen(AP_PASSWORD) > 63) {
        printf("Invalid AP name or WPA2 password\n");
        cyw43_arch_deinit();
        return 1;
    }

    cyw43_arch_enable_ap_mode(AP_SSID, AP_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
    struct netif *ap = &cyw43_state.netif[CYW43_ITF_AP];
    printf("Wi-Fi access point: %s (%s)\n", AP_SSID, ip4addr_ntoa(netif_ip4_addr(ap)));
    if (!dhcp_server_start(ap)) {
        cyw43_arch_disable_ap_mode();
        cyw43_arch_deinit();
        return 1;
    }

    led_on = false;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
    if (!peer_server_open()) {
        dhcp_server_stop();
        cyw43_arch_disable_ap_mode();
        cyw43_arch_deinit();
        return 1;
    }

    printf("USB serial commands: ON, OFF, TOGGLE\n");
    while (true) {
        cyw43_arch_poll();
        read_serial();
        sleep_ms(10);
    }
}
