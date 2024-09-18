#include "pico/stdlib.h"
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string>

#include "../../lib/DL1416/dlsiemens.hpp"

using namespace std;

// GPIO pin configuration for the display
// modulos[CE1,CE2,CE3], wri, ds0, ds1, d0, d1, d2, d3, d4, d5, d6
DL1416::DLSIEMENS displayDL({5, 4, 3}, 2, 8, 7, 13, 12, 11, 10, 14, 15, 9);

int main() {
  gpio_init(1);
  gpio_set_dir(1, GPIO_OUT);
  gpio_put(1, 1);
  // Write character to display
  displayDL.characterSet('A', 0, 0);
  displayDL.characterSet('B', 1, 0);
  displayDL.characterSet('C', 2, 0);
  displayDL.characterSet('D', 3, 0);
  displayDL.characterSet('E', 0, 1);
  displayDL.characterSet('F', 1, 1);
  displayDL.characterSet('G', 2, 1);
  displayDL.characterSet('H', 3, 1);
  displayDL.characterSet('I', 0, 2);
  displayDL.characterSet('J', 1, 2);
  displayDL.characterSet('K', 2, 2);
  displayDL.characterSet('L', 3, 2);

  sleep_ms(5000);
  while (true) {
    // Write word to display
    displayDL.writDisplay("<TESTE+1234>");
    sleep_ms(5000);
    displayDL.writDisplay("HELLO-WORLD");
    for (int i = 0; i < 30; ++i) {
      displayDL.characterSet('1', 3, 2);
      sleep_ms(100);
      displayDL.characterSet('/', 3, 2);
      sleep_ms(100);
      displayDL.characterSet('-', 3, 2);
      sleep_ms(100);
      displayDL.characterSet('\\', 3, 2);
      sleep_ms(100);
    }
  }
  return 0;
}
