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
#include "../../lib/oledfx/oledfx.hpp"
#include "../../lib/oledfx/OLED_font.hpp"

#include "src/t-rex-duino.hpp"

using namespace std;

oledfx oled(
    0x3C,
    size_display::W128xH64,
    i2c1,
    14,
    15);

void setup()
{
    // setup display
    printf("=== DINO GAME - BUTTON BUILD v3 - GPIO5 ===\n");
    stdio_init_all();
    gpio_init(5);
    gpio_set_dir(5, GPIO_IN);
    gpio_pull_up(5);
}

int main()
{
    setup();

    // if you are using 128x32 oled try size::W128xH32
    oled.clear();
    oled.setFont(pFontDefault);
    oled.drawString(0, 0, "Picolib");
    oled.display();

    getchar(); // Aguarda receber qualquer coisa
    printf("COM?\n");
    getchar(); // Aguarda receber qualquer coisa
    oled.clear();
    oled.drawString(0, 0, "COM-ACK");
    oled.display();
    printf("COM-ACK\n");

    char serialImput;

    while (true)
    {
        serialImput = getchar();
        if (serialImput != '\0')
        { // diferente de vasio
            oled.clear();
            oled.drawString(0, 0, "Recebido:");
            oled.drawChar(3, 10, serialImput);
            oled.display();
            printf("Recebido: %c\n", serialImput);
        }
    }
}
