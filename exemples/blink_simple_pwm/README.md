# Blink Simple PWM with Raspberry Pi Pico W

This project fades the onboard LED of the Raspberry Pi Pico W in and out using software PWM (bit-banged through the CYW43 chip, so the hardware PWM slices are not used).

## How it works

- The PWM period is `PWM_PERIOD_US` (5000 us, i.e. 200 Hz).
- In each period the LED is on for `brightness` us and off for the remaining time.
- `brightness` changes by `step` (50 us) per period, ramping from 0 to `PWM_PERIOD_US` and back. A full fade in/out takes about 1 s.
- At minimum brightness the loop pauses 10 ms before fading in again.

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
    - Drag and drop the generated `blink_simple_pwm.uf2` file onto the mounted USB drive.

3. Run the code:
    - The onboard LED should smoothly fade in and out, repeatedly.
