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
#include <../../lib/oledfx/oledfx.hpp>

using namespace std;

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
    string mensagem = "COM-ACK\n\n";
    setup();

    oledfx oled(0x3C, size::W128xH64, i2c0);   //Declare oled instance
    // if you are using 128x32 oled try size::W128xH32

    printf("COM-ACK\n\n");
    getchar();

    oled.clear();
    oled.drawString(0, 0, mensagem);
    oled.display();
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
