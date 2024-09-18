/*
dlsiemens.cpp 
This file is part of the DL1416 Display Project with Raspberry Pi Pico.
 
@author André Araújo
@details <https://github.com/AndreFPDA/picolib>


This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
 
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
 */

#ifndef DLSIEMENS_h
#define DLSIEMENS_h

#include <array>
#include <stdio.h>
#include <string>
#include <vector>

#include "hardware/gpio.h"
#include "pico/stdlib.h"

namespace DL1416 {

class DLSIEMENS {
public:
  DLSIEMENS(const std::vector<int> &modulos, int wri, int ds0, int ds1, int d0,
            int d1, int d2, int d3, int d4, int d5, int d6);

  //=================================================================================================
  void characterSet(const char &character, const int &digit,
                    const int &ModuleDisplay);

  //=================================================================================================
  void writDisplay(const std::string &str);

  //=================================================================================================
  void clearDisplay();

  //=================================================================================================
private:
  const std::vector<int> _modulos;
  const int _wri;
  const int _ds0;
  const int _ds1;
  const std::array<int, 7> _dataCode;
  std::array<int, 128> charset;

  // Mapping the values ​​of _ds0 and _ds1 to each digit
  const std::array<std::pair<int, int>, 4> digit_map = {{
      {1, 1}, // Digit 0
      {0, 1}, // Digit 1
      {1, 0}, // Digit 2
      {0, 0}  // Digit 3
  }};

  //=================================================================================================
  void setDigit(const int &digit);
};

} // end namespace DL1416
#endif
