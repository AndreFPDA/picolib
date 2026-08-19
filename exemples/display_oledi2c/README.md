# Display OLED I2C (SSD1306) with Raspberry Pi Pico

This project demonstrates how to connect and control an SSD1306-based I2C OLED display (128x64 or 128x32) using a Raspberry Pi Pico and the `oledfx` library, and echo back characters received over USB serial.

## Requirements

- Raspberry Pi Pico
- SSD1306 I2C OLED display (128x64 or 128x32)

## Hardware Connections

| Display Pin | Pico Pin | Function        |
|-------------|----------|-----------------|
| SDA         | GPIO 14  | I2C1 SDA        |
| SCL         | GPIO 15  | I2C1 SCL        |
| VCC         | 3V3      | Power           |
| GND         | GND      | Ground          |

I2C pins, bus speed and the I2C instance (`i2c0`/`i2c1`) can be customized when creating the `oledfx` object:

```cpp
// DevAddr, size, i2c instance, sda_pin, scl_pin, baudrate
oledfx oled(0x3C, size_display::W128xH64, i2c1, 14, 15, 400000);
```

> **Warning**
> Check how the `oledfx` library is declared in `CMakeLists.txt` and copy `picolib/lib/oledfx` to the project if reusing this example elsewhere.

## What the Example Does

1. Initializes the display and shows "Picolib".
2. Waits for any byte over USB serial, then shows "COM-ACK".
3. Loops forever: for every byte received over serial, displays "Recebido:" followed by the received character, and echoes it back over serial.

## Compilation and Execution

1. Compile the code:
    ```sh
    cd src
    mkdir build
    cd build
    cmake ..
    make
    ```
2. Upload the code to the Raspberry Pi Pico:
    - Connect the Pico to your computer while holding the BOOTSEL button.
    - Drag and drop the generated `main.uf2` file onto the mounted USB drive.

3. Run the code:
    - Open a serial terminal to the Pico's USB port.
    - Send any character; the display should show "Picolib", then "COM-ACK", and afterward echo each character you send.
