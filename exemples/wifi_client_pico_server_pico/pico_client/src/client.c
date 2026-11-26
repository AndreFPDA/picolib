#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "pico/error.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/ip4_addr.h"
#include "wifi_client_config.h"
#include "led_command.h"

#define PEER_PORT 4242
#define SERIAL_LINE_SIZE 16

static bool led_on;
static bool pending_state;
static bool connecting;
static bool wifi_connected;
static struct tcp_pcb *peer_pcb;
static struct tcp_pcb *connecting_pcb;
static absolute_time_t next_wifi_attempt;
static absolute_time_t next_peer_attempt;
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
        printf("Sent LED state %s to server\n", led_on ? "ON" : "OFF");
    }
}

static void handle_serial_line(void) {
    serial_line[serial_line_length] = '\0';
    if (strcmp(serial_line, "??") == 0) {
        printf("Sou o CLIENT\n");
    } else {
        led_command_t command = led_command_parse(serial_line);
        if (command == LED_COMMAND_INVALID) {
            printf("Invalid command. Use ON, OFF, or TOGGLE.\n");
        } else {
            set_led(command, "USB serial");
            send_led_state();
        }
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
    connecting_pcb = NULL;
    connecting = false;
    peer_line_length = 0;
    pending_state = true;
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
        connecting_pcb = NULL;
        connecting = false;
        pending_state = true;
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

static err_t peer_connected(void *arg, struct tcp_pcb *pcb, err_t error) {
    (void)arg;
    connecting = false;
    connecting_pcb = NULL;
    if (error != ERR_OK) {
        peer_pcb = NULL;
        printf("TCP connection failed: %d\n", error);
        tcp_abort(pcb);
        return ERR_ABRT;
    }

    peer_pcb = pcb;
    peer_line_length = 0;
    tcp_arg(pcb, NULL);
    tcp_err(pcb, peer_error);
    tcp_recv(pcb, peer_recv);
    printf("Connected to Wi-Fi peer\n");
    if (pending_state) {
        send_led_state();
    }
    return ERR_OK;
}

static void connect_to_peer(void) {
    ip_addr_t address;
    IP4_ADDR(&address, SERVER_IP_OCTET_1, SERVER_IP_OCTET_2,
             SERVER_IP_OCTET_3, SERVER_IP_OCTET_4);
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (pcb == NULL) {
        printf("Failed to create TCP connection\n");
        return;
    }

    tcp_arg(pcb, NULL);
    tcp_err(pcb, peer_error);
    connecting = true;
    connecting_pcb = pcb;
    err_t error = tcp_connect(pcb, &address, PEER_PORT, peer_connected);
    if (error != ERR_OK) {
        connecting = false;
        connecting_pcb = NULL;
        printf("Failed to start TCP connection: %d\n", error);
        tcp_abort(pcb);
    }
}

static void start_wifi_connection(void) {
    int error = cyw43_arch_wifi_connect_async(
        WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
    if (error != 0) {
        printf("Could not start Wi-Fi connection: %d\n", error);
    } else {
        printf("Connecting to Wi-Fi network %s\n", WIFI_SSID);
    }
    next_wifi_attempt = make_timeout_time_ms(5000);
}

static void close_peer_connections(void) {
    if (peer_pcb != NULL) {
        struct tcp_pcb *pcb = peer_pcb;
        peer_pcb = NULL;
        tcp_arg(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_abort(pcb);
    }
    if (connecting_pcb != NULL) {
        struct tcp_pcb *pcb = connecting_pcb;
        connecting_pcb = NULL;
        tcp_arg(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_abort(pcb);
    }
    connecting = false;
}

static void update_wifi_connection(void) {
    int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    if (status == CYW43_LINK_UP) {
        if (!wifi_connected) {
            wifi_connected = true;
            printf("Connected to Wi-Fi network %s\n", WIFI_SSID);
            next_peer_attempt = get_absolute_time();
        }
        return;
    }

    if (wifi_connected) {
        wifi_connected = false;
        close_peer_connections();
        pending_state = true;
        next_peer_attempt = make_timeout_time_ms(3000);
        printf("Wi-Fi connection lost (status %d)\n", status);
    }

    if (time_reached(next_wifi_attempt) &&
        status != CYW43_LINK_JOIN && status != CYW43_LINK_NOIP) {
        printf("Wi-Fi unavailable (status %d); retrying\n", status);
        start_wifi_connection();
    }
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);
    printf("Pico Wi-Fi client started\n");

    if (cyw43_arch_init()) {
        printf("Failed to initialize the CYW43 driver\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    led_on = false;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
    printf("USB serial commands: ON, OFF, TOGGLE\n");

    next_wifi_attempt = get_absolute_time();
    next_peer_attempt = get_absolute_time();
    start_wifi_connection();
    while (true) {
        cyw43_arch_poll();
        read_serial();
        update_wifi_connection();
        if (wifi_connected && peer_pcb == NULL && !connecting &&
            time_reached(next_peer_attempt)) {
            connect_to_peer();
            next_peer_attempt = make_timeout_time_ms(3000);
        }
        sleep_ms(10);
    }
}
