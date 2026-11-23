# LED Control over a Direct Wi-Fi Connection

The Raspberry Pi Pico W or Pico 2 W creates its own Wi-Fi network and a web
page to turn the onboard LED on and off. A computer or phone connects to the
Pico without a router.

**Note:** the connection uses a Wi-Fi access point (soft AP), not the
Wi-Fi Direct/P2P protocol. It does not provide internet access. Some phones
warn that the network has no internet: choose to stay connected.

## Configuration

Edit `src/wifi_ap_config.h` to set the network name and password:

```c
#define AP_SSID "Pico-LED"
#define AP_PASSWORD "picoled123"
```

Change the example password before use; it is public. The WPA2 password must
be 8 to 63 characters long and the network name at most 32 bytes. The network
uses 2.4 GHz. Anyone connected to the network can control the LED; the page
uses unencrypted HTTP and has no authentication of its own. Do not reuse
personal passwords or publish your chosen password in the repository.

## Building

Install the Pico SDK and the ARM compiler as described in the
[main README](../../README.md). Use Pico SDK 2.0.0 or later for Pico 2 W.
From the project root:

```bash
export PICO_SDK_PATH=~/pico/pico-sdk
cd exemples/wifi_direct_html_led/src
```

For Pico W:

```bash
cmake -S . -B build_picow -DPICO_BOARD=pico_w
cmake --build build_picow -j4
```

The generated file is `build_picow/picow_wifi_direct_led.uf2`.

For Pico 2 W:

```bash
cmake -S . -B build_pico2w -DPICO_BOARD=pico2_w
cmake --build build_pico2w -j4
```

The generated file is `build_pico2w/pico2w_wifi_direct_led.uf2`. Use
separate, new build directories for each example and board; if a directory is
already configured for another example, choose another name for `-B`.

## Flashing and connecting

1. Connect the board to USB while holding **BOOTSEL** and copy the
   corresponding UF2 file to the drive that appears. The board will restart.
2. On the PC or phone, connect to the configured network and choose to stay
   connected if the system warns that there is no internet.
3. Open **http://192.168.4.1/** in a browser and use the button to control
   the onboard LED. The page shows its current state.

The Pico automatically assigns IPv4 addresses via DHCP. The page address also
appears in the USB serial output (115200 baud). The `/on` and `/off` routes
set the state explicitly; `/toggle` changes the LED state and redirects to the
home page, preventing another toggle when the page is refreshed.
The LED starts on after each restart.

## If the network does not appear

- Make sure you flashed `pico2w_wifi_direct_led.uf2` for the Pico 2 W.
  The file `pico2w_wifi_led.uf2` belongs to the `wifi_html_led` example:
  it attempts to connect to an existing network and does not create the
  `Pico-LED` network.
- Check the name in `wifi_ap_config.h`, then rebuild and flash again after
  any changes. Look for 2.4 GHz networks near the board.
- Open the USB serial output at 115200 baud and restart the board with the
  monitor connected. Look for messages such as
  `Failed to initialize the CYW43 driver`, `Invalid AP name or WPA2 password`,
  or DHCP/HTTP server failures. The message `Wi-Fi access point: Pico-LED`
  indicates that the program attempted to start the AP, but does not by itself
  confirm that the network is being broadcast.
