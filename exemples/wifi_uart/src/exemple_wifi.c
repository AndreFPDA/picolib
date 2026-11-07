#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

int main() {
    stdio_init_all();
    sleep_ms(2000);

    // Initialize the CYW43 Wi-Fi driver architecture.
    if (cyw43_arch_init()) {
        printf("Failed to initialize the CYW43 driver\n");
        return 1;
    }

    // Enable station (Wi-Fi client) mode.
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi '%s'...\n", WIFI_SSID);

    // Try to connect to the Wi-Fi network with a 10-second timeout.
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Failed to connect or connection timed out.\n");
        cyw43_arch_deinit();
        return 1;
    } else {
        printf("Connected successfully!\n");
        
        // Get the IP address assigned by lwIP.
        ip4_addr_t ip = cyw43_state.netif[0].ip_addr;
        printf("IP address: %s\n", ip4addr_ntoa(&ip));
    }

    while (true) {
        // Process pending lwIP and CYW43 work.
        cyw43_arch_poll();
        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}