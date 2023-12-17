/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <lib/GFX.hpp>

using namespace std;


void acionarAtuadorTecla(int tecla) {

    int32_t mask = bits_map_GPIO_dmux[tecla] << FIRST_GPIO;

    // Set all our GPIOs in one go!
    // If something else is using GPIO, we might want to use gpio_put_masked()
    gpio_set_mask(mask);
    
    gpio_put(signal, 1);
    sleep_ms(250);
    gpio_put(signal, 0);    

    gpio_clr_mask(mask);
    printf("ACK-%d\n", tecla);
    printf("bits_map_GPIO-%d\n", bits_map_GPIO_dmux[tecla]);
    printf("mascara-%d\n", mask);
}


void setup() {
    //setup display
    stdio_init_all();

    i2c_init(i2c0, 400000);                 //Initialize I2C on i2c0 port with 400kHz
    gpio_set_function(16, GPIO_FUNC_I2C);    //Use GPIO16 as I2C  SDA
    gpio_set_function(17, GPIO_FUNC_I2C);    //Use GPIO17 as I2C  SCL
    gpio_pull_up(16);                        //Pull up GPIO16 
    gpio_pull_up(17);  

}

int main() {
    setup();

    GFX oled(0x3C, size::W128xH64, i2c0);   //Declare oled instance
    // if you are using 128x32 oled try size::W128xH32
    getchar();

    string mensagem = "COM-ACK\n\n";
    oled.clear();
    oled.drawString(0, 0, mensagem);
    oled.display();
    printf("COM-ACK\n\n");
    sleep_ms(1000);

    char serialImput;

    while (true) {
        serialImput = getchar();
        oled.clear();
        oled.drawString(0, 0, "Recebido:");
        oled.drawChar(3, 10, serialImput);
        oled.display();
    }
}
