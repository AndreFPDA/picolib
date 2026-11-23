#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "dhcp_server.h"
#include "wifi_ap_config.h"

#define HTTP_PORT 80
#define REQUEST_BUFFER_SIZE 4096
#define RESPONSE_BUFFER_SIZE 1024


static bool led_on = true;

// Static buffers keep large HTTP data off the stack.
static char http_body[1024];
static char http_response[RESPONSE_BUFFER_SIZE];

typedef struct {
    char request[REQUEST_BUFFER_SIZE];
    size_t length;
} http_client_t;

static int build_html_response(char *response, size_t response_size) {
    const char *status_text = led_on ? "ON" : "OFF";
    const char *status_color = led_on ? "#28a745" : "#dc3545";
    const char *btn_text = led_on ? "TURN LED OFF" : "TURN LED ON";
    const char *btn_color = led_on ? "#dc3545" : "#28a745";

    int body_length = snprintf(
        http_body,
        sizeof(http_body),
        "<!doctype html>"
        "<html lang=\"en\"><head><meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        "<title>Pico W</title>"
        "<style>"
        "body{font-family:sans-serif;text-align:center;padding:40px;background:#f0f2f5;}"
        ".card{background:#fff;padding:30px;border-radius:12px;display:inline-block;box-shadow:0 4px 10px rgba(0,0,0,0.15);}"
        ".status{font-size:24px;font-weight:bold;color:%s;margin:20px 0;}"
        ".btn{display:inline-block;padding:16px 32px;font-size:20px;font-weight:bold;color:#fff;background:%s;text-decoration:none;border-radius:8px;}"
        "</style></head>"
        "<body><div class=\"card\">"
        "<h1>Raspberry Pi Pico W / Pico 2 W</h1>"
        "<div class=\"status\">Onboard LED: %s</div>"
        "<a href=\"/toggle\" class=\"btn\">%s</a>"
        "</div></body></html>",
        status_color,
        btn_color,
        status_text,
        btn_text);

    if (body_length < 0 || (size_t)body_length >= sizeof(http_body)) {
        return -1;
    }

    return snprintf(
        response,
        response_size,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: %d\r\n"
        "Cache-Control: no-store, no-cache, must-revalidate\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        body_length,
        http_body);
}

static void http_error(void *arg, err_t err) {
    printf("HTTP connection error: %d\n", err);
    free(arg);
}

static err_t http_close(struct tcp_pcb *tpcb, http_client_t *client) {
    tcp_arg(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_err(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);
    free(client);
    err_t err = tcp_close(tpcb);
    if (err != ERR_OK) {
        printf("HTTP close failed: %d\n", err);
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    return ERR_OK;
}

static err_t http_poll(void *arg, struct tcp_pcb *tpcb) {
    printf("HTTP request timed out\n");
    return http_close(tpcb, arg);
}

static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    http_client_t *client = arg;
    if (err != ERR_OK || p == NULL) {
        if (p) pbuf_free(p);
        if (err != ERR_OK) printf("HTTP receive failed: %d\n", err);
        return http_close(tpcb, client);
    }

    size_t available = sizeof(client->request) - 1 - client->length;
    size_t count = p->tot_len < available ? p->tot_len : available;
    pbuf_copy_partial(p, client->request + client->length, count, 0);
    client->length += count;
    client->request[client->length] = '\0';
    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);

    bool headers_complete = strstr(client->request, "\r\n\r\n") != NULL;
    if (!headers_complete && client->length < sizeof(client->request) - 1) {
        return ERR_OK;
    }

    const char *response;
    int response_length;
    if (!headers_complete) {
        response = "HTTP/1.1 431 Request Header Fields Too Large\r\n"
                   "Content-Length: 0\r\nConnection: close\r\n\r\n";
        response_length = strlen(response);
    } else if (strncmp(client->request, "GET /toggle ", 12) == 0 ||
        strncmp(client->request, "GET /on ", 8) == 0 ||
        strncmp(client->request, "GET /off ", 9) == 0) {
        if (client->request[5] == 't') led_on = !led_on;
        else led_on = client->request[6] == 'n';
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
        printf("Onboard LED: %s\n", led_on ? "ON" : "OFF");
        response = "HTTP/1.1 303 See Other\r\nLocation: /\r\n"
                   "Cache-Control: no-store\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";
        response_length = strlen(response);
    } else if (strncmp(client->request, "GET / ", 6) == 0) {
        response = http_response;
        response_length = build_html_response(http_response, sizeof(http_response));
        if (response_length < 0 || (size_t)response_length >= sizeof(http_response)) {
            printf("HTTP response buffer too small\n");
            tcp_arg(tpcb, NULL);
            tcp_err(tpcb, NULL);
            free(client);
            tcp_abort(tpcb);
            return ERR_ABRT;
        }
    } else {
        response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        response_length = strlen(response);
    }

    err = tcp_write(tpcb, response, (u16_t)response_length, TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) err = tcp_output(tpcb);
    if (err != ERR_OK) {
        printf("HTTP send failed: %d\n", err);
        tcp_arg(tpcb, NULL);
        tcp_err(tpcb, NULL);
        free(client);
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    return http_close(tpcb, client);
}

static err_t http_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    (void)arg;

    if (err != ERR_OK || client_pcb == NULL) {
        return ERR_VAL;
    }

    http_client_t *client = calloc(1, sizeof(*client));
    if (!client) {
        printf("HTTP: failed to allocate client\n");
        tcp_abort(client_pcb);
        return ERR_ABRT;
    }
    tcp_arg(client_pcb, client);
    tcp_err(client_pcb, http_error);
    tcp_recv(client_pcb, http_recv);
    tcp_poll(client_pcb, http_poll, 20);
    return ERR_OK;
}

static bool http_server_open(void) {
    struct tcp_pcb *server_pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (server_pcb == NULL) {
        printf("Failed to create the TCP server\n");
        return false;
    }

    err_t bind_error = tcp_bind(server_pcb, IP_ANY_TYPE, HTTP_PORT);
    if (bind_error != ERR_OK) {
        printf("Failed to bind to port %d: %d\n", HTTP_PORT, bind_error);
        tcp_close(server_pcb);
        return false;
    }

    struct tcp_pcb *listener = tcp_listen(server_pcb);
    if (listener == NULL) {
        printf("Failed to start the HTTP server\n");
        tcp_close(server_pcb);
        return false;
    }

    tcp_accept(listener, http_accept);
    printf("HTTP server listening on port %d\n", HTTP_PORT);
    return true;
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    // Initialize the CYW43 Wi-Fi driver architecture.
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
    printf("Wi-Fi access point: %s\n", AP_SSID);
    printf("Open http://%s/\n", ip4addr_ntoa(netif_ip4_addr(ap)));

    if (!dhcp_server_start(ap)) {
        cyw43_arch_disable_ap_mode();
        cyw43_arch_deinit();
        return 1;
    }

    // Start with the LED on.
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
    if (!http_server_open()) {
        dhcp_server_stop();
        cyw43_arch_disable_ap_mode();
        cyw43_arch_deinit();
        return 1;
    }

    while (true) {
        cyw43_arch_poll();
        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}