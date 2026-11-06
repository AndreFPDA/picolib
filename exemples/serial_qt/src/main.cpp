#include <stdio.h>
#include "pico/stdlib.h"
#include <iostream>
#include <string>

int main() {
    stdio_init_all();

    std::string input_buffer = "";
    
    // Configura o LED onboard para testes visuais
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        // --- LEITURA (RX) ---
        // '0' como argumento faz a função ler o buffer instantaneamente sem esperar
        int c = getchar_timeout_us(0); 
        
        while (c != PICO_ERROR_TIMEOUT) {
            if (c == '\n' || c == '\r') {
                if (!input_buffer.empty()) {
                    // Processamento básico de comandos recebidos
                    if (input_buffer == "LIGAR") {
                        gpio_put(LED_PIN, 1);
                        printf("ACK: LED Ativado\n");
                    } else if (input_buffer == "DESLIGAR") {
                        gpio_put(LED_PIN, 0);
                        printf("ACK: LED Desativado\n");
                    } else {
                        printf("ERRO: Comando desconhecido (%s)\n", input_buffer.c_str());
                    }
                    input_buffer.clear();
                }
            } else {
                input_buffer += (char)c;
            }
            c = getchar_timeout_us(0); // Varre os próximos caracteres que vieram no pacote
        }

        // --- ENVIO ASSÍNCRONO (TX) ---
        // Exemplo: O Pico pode enviar dados de telemetria de forma autônoma a cada 5 segundos
        static uint32_t last_time = 0;
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_time > 5000) { 
            printf("TELEMETRIA: Status OK (Tempo de atividade: %u ms)\n", current_time);
            last_time = current_time;
        }

        sleep_ms(1); // Proteção contra consumo excessivo de recursos do core
    }
    return 0;
}
