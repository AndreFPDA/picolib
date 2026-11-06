# Wi-Fi Connection with Raspberry Pi Pico W and Pico 2 W

This example connects a **Raspberry Pi Pico W** or **Raspberry Pi Pico 2 W** to
a Wi-Fi network and prints the IP address assigned by the network to the USB
serial monitor.

## Board overview

| Feature | Raspberry Pi Pico W | Raspberry Pi Pico 2 W |
|---|---|---|
| Microcontroller | RP2040 | RP2350 |
| Architecture | Dual-core Arm Cortex-M0+ | Dual-core Arm Cortex-M33 |
| Wireless chip | CYW43439 (2.4 GHz Wi-Fi and Bluetooth) | CYW43439 (2.4 GHz Wi-Fi and Bluetooth) |
| CMake board name | `pico_w` | `pico2_w` |
| Generated UF2 file | `picow_wifi_exemplo.uf2` | `pico2w_wifi_exemplo.uf2` |

## 1. Prepare the development environment

### Linux dependencies

On Ubuntu or Debian, install CMake and the ARM compiler toolchain:

```bash
sudo apt update
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential -y
```

### Pico SDK

Use **Pico SDK 2.0.0 or later** for Pico 2 W support. If the SDK is already
installed at `~/pico/pico-sdk`, initialize its submodules:

```bash
cd ~/pico/pico-sdk
git submodule update --init --recursive
```

If you do not have the SDK yet, clone it and initialize the submodules:

```bash
mkdir -p ~/pico
cd ~/pico
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init --recursive
```

Set `PICO_SDK_PATH` in each terminal used to build the example:

```bash
export PICO_SDK_PATH=~/pico/pico-sdk
```

## 2. Configure Wi-Fi credentials

Open `src/exemple_wifi.c` and set your network credentials:

```c
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

Do not commit real Wi-Fi credentials to a public repository. The example uses
`CYW43_AUTH_WPA2_AES_PSK` and requires a 2.4 GHz Wi-Fi network; 5 GHz networks
are not supported.

## 3. Build the firmware

Build each board in a separate directory to keep the CMake configuration for
the RP2040 and RP2350 targets separate.

From the repository root, enter the example's `src` directory:

```bash
cd exemples/wifi_html/src
```

### Raspberry Pi Pico 2 W

```bash
cmake -B build_pico2w -DPICO_BOARD=pico2_w
cmake --build build_pico2w -j4
```

Output: `build_pico2w/pico2w_wifi_exemplo.uf2`

### Raspberry Pi Pico W

```bash
cmake -B build_picow -DPICO_BOARD=pico_w
cmake --build build_picow -j4
```

Output: `build_picow/picow_wifi_exemplo.uf2`

## 4. Flash the board

1. Disconnect the board from USB.
2. Hold down **BOOTSEL** while connecting it to the computer.
3. Release **BOOTSEL** when the `RPI-RP2` drive appears.
4. Copy the UF2 file for your board to that drive.

For Pico 2 W:

```bash
cp build_pico2w/pico2w_wifi_exemplo.uf2 /media/$USER/RPI-RP2/
```

For Pico W:

```bash
cp build_picow/picow_wifi_exemplo.uf2 /media/$USER/RPI-RP2/
```

## 5. Monitor serial output

Open the board's USB serial port at **115200 baud** using PuTTY, `minicom`,
`screen`, or the VS Code serial monitor. The output should look similar to:

```text
Connecting to Wi-Fi 'YOUR_WIFI_SSID'...
Connected successfully!
IP address: 192.168.1.123
```
