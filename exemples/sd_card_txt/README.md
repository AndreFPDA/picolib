## References

- DigiKey tutorial:
	https://www.digikey.com.br/en/maker/projects/raspberry-pi-pico-rp2040-sd-card-example-with-micropython-and-cc/e472c7f578734bfd96d437e68e670050
- no-OS-FatFS-SD-SPI-RPi-Pico repository:
	https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico

# sd_card_txt Example (Raspberry Pi Pico + microSD + FatFs)

This example writes and then reads a text file (`test02.txt`) on a microSD card using SPI on a Raspberry Pi Pico.

## Requirements

- Raspberry Pi Pico
- SPI microSD module
- microSD card formatted as FAT/FAT32
- Pico SDK configured (`PICO_SDK_PATH`)
- `cmake` and ARM toolchain installed

## 1) Clone no-OS-FatFS-SD-SPI-RPi-Pico

Clone the library in:

`/home/andre-araujo/Documentos/projetos/picolib/lib/`

Commands:

```sh
cd /home/andre-araujo/Documentos/projetos/picolib/lib
git clone https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico.git
```

## 2) Configure hw_config.c

Use this hardware configuration file (same directory as `main.cpp`):

`src/hw_config.c`

Adjust SPI and CS pins to match your wiring.

Current configuration in this example:

- SPI: `spi1`
- MISO: GPIO `8`
- MOSI: GPIO `11`
- SCK: GPIO `10`
- CS/SS: GPIO `9`
- Card detect: disabled (`use_card_detect = false`)

Corresponding snippet:

```c
.hw_inst = spi1,
.miso_gpio = 8,
.mosi_gpio = 11,
.sck_gpio = 10,
.baud_rate = 1000 * 1000,
...
.ss_gpio = 9,
.use_card_detect = false,
.card_detected_true = -1
```

If your pins are different, change only these fields.

Important:

- Use `src/hw_config.c` as the active configuration file.
- Do not use the `hw_config.c` inside the cloned library for this example.

## 3) Build

From the `src` folder:

```sh
cd src
mkdir -p build
cd build
cmake ..
make -j
```

Generated UF2 binary:

`src/build/main.uf2`

## 4) Flash to Pico

1. Press and hold BOOTSEL on the Pico.
2. Connect the USB cable.
3. Release BOOTSEL.
4. Copy `main.uf2` to the mounted USB drive (`RPI-RP2`).

## 5) Install picocom (Linux)

Use the command for your distribution:

```sh
# Debian/Ubuntu
sudo apt update && sudo apt install -y picocom

# Fedora
sudo dnf install -y picocom

# Arch
sudo pacman -S picocom
```

If needed, add your user to the serial group and log out/log in:

```sh
sudo usermod -aG dialout $USER
```

## 6) Find the Pico serial port

With the Pico connected, use one of these methods:

```sh
ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null
```

or monitor kernel messages:

```sh
dmesg -w
```

It usually appears as `/dev/ttyACM0`.

## 7) Use picocom

Open serial terminal at 115200 baud (replace the device path):

```sh
picocom -b 115200 /dev/ttyACM0
```

In the terminal:

1. Wait for `SD card test. Press 'enter' to start.`
2. Press Enter.
3. The program writes and reads the file from the SD card.

To exit picocom:

- `Ctrl + A`, then `Ctrl + X`

## 8) Expected output

Expected serial output:

```text
SD card test. Press 'enter' to start.
Reading from file 'test02.txt':
---
This is another test
of writing to an SD card.

---
```

If you see errors like `Could not mount filesystem` or `Could not initialize SD card`, check:

- SPI/CS wiring
- `src/hw_config.c` configuration
- SD card formatted as FAT/FAT32
- microSD module power supply
