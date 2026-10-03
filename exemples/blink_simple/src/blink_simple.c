
// Simple example to blink an LED on the Raspberry Pi Pico W

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main() {
    stdio_init_all();

    // Inicializa a arquitetura do chip Wi-Fi (obrigatório para usar o LED)
    if (cyw43_arch_init()) {
        printf("Falha ao inicializar o chip Wi-Fi\n");
        return -1;
    }

    while (true) {
        // Liga o LED usando a API do CYW43
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(500);

        // Desliga o LED
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(500);
    }
}