# Raspberry Pico SSD1306 + GFX Library -> Modified

Based onmbober1 Library
-> https://github.com/mbober1/RPi-Pico-SSD1306-library

Based on Adafruit GFX Library
->  https://github.com/adafruit/Adafruit-GFX-Library

# Usage

## Hardware
Connect your SSD1306 oled display to i2c1 port (you can change it during initialization)
```
SDA->GPIO2
SCL->GPIO3
GND->GND
VCC->3V3(OUT)
```

## Software
The library needs only two files to run `SSD1306.hpp` and `SSD1306.cpp`.
This gives you the ability to display a bitmap or array of pixels. 
You need the GFX library to make it easier to create images for your display. (`oledfx.hpp` and `oledfx.cpp`)

`logo.hpp` is an example showing how to create bitmaps.
`OLED_font.cpp` contains one font.

## Example
Repo provides an example of how a library can be used.

```
git clone https://github.com/AndreFPDA/picolib.git
cd /picolib/exemples/my_tutorial_exemple/src
mkdir build
cd build
```

```
export PICO_SDK_PATH=../../../pico-sdk
```

```
cmake ..
```
```
make -j4
```
