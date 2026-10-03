// Fade the Pico W onboard LED using software PWM

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define PWM_PERIOD_US 5000

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Failed to initialize Wi-Fi chip\n");
        return -1;
    }

    int brightness = 0;  
    int step = 50;        

    while (true) {
        if (brightness > 0) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_us(brightness); 
        }
        
        if (brightness < PWM_PERIOD_US) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_us(PWM_PERIOD_US - brightness); 
        }

        brightness += step;

        if (brightness >= PWM_PERIOD_US) {
            brightness = PWM_PERIOD_US;
            step = -step;
        } else if (brightness <= 0) {
            brightness = 0;
            step = -step;
            sleep_ms(10);
        }
    }
}