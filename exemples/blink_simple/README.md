# Blink Simple with Raspberry Pi Pico W

This project blinks the onboard LED of the Raspberry Pi Pico W every 500 ms.

## Requirements

- Raspberry Pi Pico W
- Pico SDK

## Hardware Connections

No external connections are required. On the Pico W the onboard LED is wired to the CYW43 Wi-Fi chip, not to an RP2040 GPIO. That is why the code uses `cyw43_arch_init()` and `cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ...)`.

> **Warning**
> `CMakeLists.txt` must contain `set(PICO_BOARD pico_w)` and link `pico_cyw43_arch_none`.

## Compilation and Execution

1. Compile the code:
    ```shell
    cd src;
    mkdir build;
    cd build;
    export PICO_SDK_PATH=~/pico/pico-sdk;
    cmake ..;
    make;
    ```

2. Upload the code to the Raspberry Pi Pico W:
    - Connect the Pico W to your computer while holding the BOOTSEL button.
    - Drag and drop the generated `blink_simple.uf2` file onto the mounted USB drive.

3. Run the code:
    - The onboard LED should turn on for 500 ms and off for 500 ms, repeatedly.
