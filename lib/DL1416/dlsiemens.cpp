/*
 * dlsiemens.cpp
 * 
 * This file is part of the DL1416 Display Project with Raspberry Pi Pico.
 * 
 * Author: André Araújo
 * 
 * This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "dlsiemens.hpp"

#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string>
#include <vector>

namespace DL1416 {

//=================================================================================================
DLSIEMENS::DLSIEMENS(const std::vector<int> &modulos, int wri, int ds0, int ds1,
                     int d0, int d1, int d2, int d3, int d4, int d5, int d6)
    : _modulos(modulos), _wri(wri), _ds0(ds0), _ds1(ds1),
      _dataCode({d0, d1, d2, d3, d4, d5, d6}) {
  // Init and desable all GPIOs modules's enable_pins

  gpio_init(_wri);
  gpio_set_dir(_wri, GPIO_OUT);
  gpio_put(_wri, 1);

  gpio_init(_ds0);
  gpio_set_dir(_ds0, GPIO_OUT);
  gpio_init(_ds1);
  gpio_set_dir(_ds1, GPIO_OUT);

  for (int num : _modulos) {
    gpio_init(num);
    gpio_set_dir(num, GPIO_OUT);
    gpio_put(num, 1);
  }

  for (int ngpio : _dataCode) {
    gpio_init(ngpio);
    gpio_set_dir(ngpio, GPIO_OUT);
    gpio_put(ngpio, 0);
  }

  const std::array<std::pair<char, int>, 64> charset_init = {
      {{'!', 0b0100001},  {' ', 0b0100000},  {'"', 0b0100010},
       {'#', 0b0100011},  {'$', 0b0100100},  {'%', 0b0100101},
       {'&', 0b0100110},  {'\'', 0b0100111}, {'(', 0b0101000},
       {')', 0b0101001},  {'*', 0b0101010},  {'+', 0b0101011},
       {',', 0b0101100},  {'-', 0b0101101},  {'.', 0b0101110},
       {'/', 0b0101111},  {'0', 0b0110000},  {'1', 0b0110001},
       {'2', 0b0110010},  {'3', 0b0110011},  {'4', 0b0110100},
       {'5', 0b0110101},  {'6', 0b0110110},  {'7', 0b0110111},
       {'8', 0b0111000},  {'9', 0b0111001},  {':', 0b0111010},
       {';', 0b0111011},  {'<', 0b0111100},  {'=', 0b0111101},
       {'>', 0b0111110},  {'?', 0b0111111},  {'@', 0b1000000},
       {'A', 0b1000001},  {'B', 0b1000010},  {'C', 0b1000011},
       {'D', 0b1000100},  {'E', 0b1000101},  {'F', 0b1000110},
       {'G', 0b1000111},  {'H', 0b1001000},  {'I', 0b1001001},
       {'J', 0b1001010},  {'K', 0b1001011},  {'L', 0b1001100},
       {'M', 0b1001101},  {'N', 0b1001110},  {'O', 0b1001111},
       {'P', 0b1010000},  {'Q', 0b1010001},  {'R', 0b1010010},
       {'S', 0b1010011},  {'T', 0b1010100},  {'U', 0b1010101},
       {'V', 0b1010110},  {'W', 0b1010111},  {'X', 0b1011000},
       {'Y', 0b1011001},  {'Z', 0b1011010},  {'[', 0b1011011},
       {'\\', 0b1011100}, {']', 0b1011101},  {'^', 0b1011110},
       {'_', 0b1011111}}};

  for (const auto &pair : charset_init) {
    charset[pair.first] = pair.second;
  }

  clearDisplay();
}

//==============================================================================================================================================
void DLSIEMENS::setDigit(const int &digit) {

  if (digit >= 0 && digit < digit_map.size()) {
    gpio_put(_ds0, digit_map[digit].first);
    gpio_put(_ds1, digit_map[digit].second);
  } else {
    gpio_put(_ds0, 1);
    gpio_put(_ds1, 1);
  }
}

//==============================================================================================================================================
void DLSIEMENS::characterSet(const char &character, const int &digit,
                             const int &ModuleDisplay) {

  gpio_put(_modulos[ModuleDisplay], 1);
  gpio_put(_wri, 1);
  setDigit(digit);
  sleep_ms(1);

  // Get the character value from the charset
  uint32_t charValue = charset[character];

  // Configure GPIO pins according to character value
  for (size_t i = 0; i < _dataCode.size(); ++i) {
    gpio_put(_dataCode[i], (charValue & (1 << i)) ? 1 : 0);
  }

  gpio_put(_modulos[ModuleDisplay], 0);
  sleep_ms(1);
  gpio_put(_wri, 0);
  sleep_ms(1);
  gpio_put(_wri, 1);
  gpio_put(_modulos[ModuleDisplay], 1);
  sleep_ms(10);
  return;
}

//==============================================================================================================================================
void DLSIEMENS::writDisplay(const std::string &str) {
  const std::vector<char> vectorChars(str.begin(), str.end());

  int setDigit = 0;
  int setModuleDisplay = 0;

  for (size_t i = 0; i < vectorChars.size(); i++) {
    characterSet(vectorChars[i], setDigit, setModuleDisplay);
    if (++setDigit > 3) {
      setDigit = 0;
      ++setModuleDisplay;
    }
  }
}

//==============================================================================================================================================
void DLSIEMENS::clearDisplay() {
  for (int j = 0; j < _modulos.size(); j++) {
    for (int i = 0; i < 4; i++) {
      characterSet(' ', i, j);
    }
  }
}

} // end namespace DL1416
