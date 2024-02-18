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

    i2c_init(i2c1, 400000);                 //Initialize I2C on i2c0 or i2c1 port with 400kHz
    gpio_set_function(14, GPIO_FUNC_I2C);    // as I2C  SDA
    gpio_set_function(15, GPIO_FUNC_I2C);    // as I2C  SCL
    gpio_pull_up(14);                        //Pull up GPIO16 
    gpio_pull_up(15);  

}

int main() {
    string mensagem = "COM-ACK";
    setup();

    oledfx oled(0x3C, size::W128xH64, i2c1);   //Declare oled instance
    // if you are using 128x32 oled try size::W128xH32
    oled.clear();
    oled.drawString(0, 0, "Picolib");
    oled.display();

    getchar();
    printf("COM?\n");
    getchar();
    oled.clear();
    oled.drawString(0, 0, mensagem);
    oled.display();
    printf("COM-ACK\n");

    char serialImput;

    while (true) {
        serialImput = getchar();
        if (serialImput != '\0') { //deiferente de vasio
            oled.clear();
            oled.drawString(0, 0, "Recebido:");
            oled.drawChar(3, 10, serialImput);
            oled.display();
            printf("Recebido: %c\n", serialImput);
        }
    }
}
