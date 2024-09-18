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
