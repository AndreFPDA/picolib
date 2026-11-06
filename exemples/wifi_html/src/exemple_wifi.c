#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

// Insira as credenciais da sua rede Wi-Fi aqui
#define WIFI_SSID "fiona"
#define WIFI_PASSWORD "gatoDe,3otas"

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
        
        // Exemplo de acesso ao endereço IP via lwIP (se necessário)
        ip4_addr_t ip = cyw43_state.netif[0].ip_addr;
        printf("IP obtido: %s\n", ip4addr_ntoa(&ip));
    }

    while (true) {
        // Realiza o processamento em segundo plano da pilha lwIP e CYW43
        cyw43_arch_poll();
        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}