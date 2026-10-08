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


using namespace std;

#define SCL_OLED 3
#define SDA_OLED 2

// baudrate lowered to 100kHz (I2C standard mode): weak RP2040 internal
// pull-ups + breadboard wiring without external pull-up resistors tend to
// corrupt data during the large full-buffer transfer at 400kHz, which shows
// up as broken/interleaved rows on screen
static oledfx oled(0x3C, size_display::W128xH64, i2c1, SDA_OLED, SCL_OLED, 100000);

void setup() {
    //setup display
    stdio_init_all();
}

int main() {
    setup();


    // if you are using 128x32 oled try size::W128xH32
    oled.setFont(pFontArialRound);
    oled.drawString(0, 0, "TESTE");
    // oled.drawString(0, 0, "Picolib");
    oled.display();

    getchar(); 
    printf("COM?\n");
    getchar(); 
    oled.clear();
    oled.drawString(0, 0, "COM-ACK");
    oled.display();
    printf("COM-ACK\n");

    char serialImput;

    while (true) {
        serialImput = getchar();
        if (serialImput != '\0') { 
            oled.clear();
            oled.drawString(0, 0, "Receved:");
            oled.drawChar(3, 10, serialImput);
            oled.display();
            printf("Receved: %c\n", serialImput);
        }
    }
}
