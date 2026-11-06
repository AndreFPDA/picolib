#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "wifi_credentials.h"

#define HTTP_PORT 80
#define REQUEST_BUFFER_SIZE 512
#define RESPONSE_BUFFER_SIZE 1024


static bool led_on = true;

// Buffers estáticos para evitar estouro da pilha (stack overflow)
static char http_request[REQUEST_BUFFER_SIZE];
static char http_body[1024];
static char http_response[RESPONSE_BUFFER_SIZE];

static int build_html_response(char *response, size_t response_size) {
    const char *status_text = led_on ? "LIGADO" : "DESLIGADO";
    const char *status_color = led_on ? "#28a745" : "#dc3545";
    const char *btn_text = led_on ? "DESLIGAR LED" : "LIGAR LED";
    const char *btn_color = led_on ? "#dc3545" : "#28a745";

    int body_length = snprintf(
        http_body,
        sizeof(http_body),
        "<!doctype html>"
        "<html lang=\"pt-BR\"><head><meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        "<title>Pico 2 W</title>"
        "<style>"
        "body{font-family:sans-serif;text-align:center;padding:40px;background:#f0f2f5;}"
        ".card{background:#fff;padding:30px;border-radius:12px;display:inline-block;box-shadow:0 4px 10px rgba(0,0,0,0.15);}"
        ".status{font-size:24px;font-weight:bold;color:%s;margin:20px 0;}"
        ".btn{display:inline-block;padding:16px 32px;font-size:20px;font-weight:bold;color:#fff;background:%s;text-decoration:none;border-radius:8px;}"
        "</style></head>"
        "<body><div class=\"card\">"
        "<h1>Raspberry Pi Pico 2 W</h1>"
        "<div class=\"status\">LED onboard: %s</div>"
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

static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    (void)arg;

    if (err != ERR_OK) {
        if (p != NULL) {
            pbuf_free(p);
        }
        return err;
    }

    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    size_t request_length = p->tot_len;
    if (request_length >= sizeof(http_request)) {
        request_length = sizeof(http_request) - 1;
    }
    pbuf_copy_partial(p, http_request, request_length, 0);
    http_request[request_length] = '\0';

    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);

    // Responde 404 para requisições de favicon
    if (strncmp(http_request, "GET /favicon.ico", 16) == 0) {
        const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        tcp_write(tpcb, not_found, (u16_t)strlen(not_found), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Ao acionar /toggle, alterna o LED e redireciona para "/" (HTTP 303)
    // Isso evita que atualizar a página (F5) reenvie o comando de toggle
    if (strncmp(http_request, "GET /toggle", 11) == 0) {
        led_on = !led_on;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on ? 1 : 0);
        printf("LED onboard alterado para: %s\n", led_on ? "LIGADO" : "DESLIGADO");

        const char *redirect =
            "HTTP/1.1 303 See Other\r\n"
            "Location: /\r\n"
            "Cache-Control: no-store\r\n"
            "Connection: close\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        tcp_write(tpcb, redirect, (u16_t)strlen(redirect), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        tcp_close(tpcb);
        return ERR_OK;
    }

    if (strncmp(http_request, "GET /on", 7) == 0) {
        led_on = true;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        printf("LED onboard: LIGADO\n");

        const char *redirect = "HTTP/1.1 303 See Other\r\nLocation: /\r\nConnection: close\r\n\r\n";
        tcp_write(tpcb, redirect, (u16_t)strlen(redirect), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        tcp_close(tpcb);
        return ERR_OK;
    }

    if (strncmp(http_request, "GET /off", 8) == 0) {
        led_on = false;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        printf("LED onboard: DESLIGADO\n");

        const char *redirect = "HTTP/1.1 303 See Other\r\nLocation: /\r\nConnection: close\r\n\r\n";
        tcp_write(tpcb, redirect, (u16_t)strlen(redirect), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        tcp_close(tpcb);
        return ERR_OK;
    }

    int response_length = build_html_response(http_response, sizeof(http_response));
    if (response_length < 0 || (size_t)response_length >= sizeof(http_response)) {
        tcp_abort(tpcb);
        return ERR_ABRT;
    }

    err_t write_error = tcp_write(
        tpcb, http_response, (u16_t)response_length, TCP_WRITE_FLAG_COPY);
    if (write_error != ERR_OK) {
        tcp_abort(tpcb);
        return ERR_ABRT;
    }

    tcp_output(tpcb);
    tcp_sent(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);
    tcp_err(tpcb, NULL);

    err_t close_err = tcp_close(tpcb);
    if (close_err != ERR_OK) {
        tcp_abort(tpcb);
    }
    return ERR_OK;
}

static err_t http_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    (void)arg;

    if (err != ERR_OK || client_pcb == NULL) {
        return ERR_VAL;
    }

    tcp_recv(client_pcb, http_recv);
    return ERR_OK;
}

static bool http_server_open(void) {
    struct tcp_pcb *server_pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (server_pcb == NULL) {
        printf("Falha ao criar o servidor TCP\n");
        return false;
    }

    err_t bind_error = tcp_bind(server_pcb, IP_ANY_TYPE, HTTP_PORT);
    if (bind_error != ERR_OK) {
        printf("Falha ao abrir a porta %d: %d\n", HTTP_PORT, bind_error);
        tcp_close(server_pcb);
        return false;
    }

    server_pcb = tcp_listen(server_pcb);
    if (server_pcb == NULL) {
        printf("Falha ao iniciar o servidor HTTP\n");
        return false;
    }

    tcp_accept(server_pcb, http_accept);
    printf("Servidor HTTP disponível na porta %d\n", HTTP_PORT);
    return true;
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    // Inicializa a arquitetura do driver CYW43 (Wi-Fi)
    if (cyw43_arch_init()) {
        printf("Falha ao inicializar o driver CYW43\n");
        return 1;
    }

    // Ativa o modo Station (cliente Wi-Fi)
    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi '%s'...\n", WIFI_SSID);

    // Tenta conectar à rede Wi-Fi com timeout de 10 segundos
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha ao conectar ou tempo esgotado.\n");
        cyw43_arch_deinit();
        return 1;
    } else {
        printf("Conectado com sucesso!\n");

        ip4_addr_t ip = cyw43_state.netif[0].ip_addr;
        printf("IP obtido: %s\n", ip4addr_ntoa(&ip));
    }

    // Inicia com o LED ligado
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
    if (!http_server_open()) {
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