# Bidirectional LED control between two Pico W boards

This example connects a Pico W or Pico 2 W client to a Wi-Fi access point
created by a Pico W or Pico 2 W server. A single TCP connection carries
commands in both directions. MQTT is not required: each board can send and
receive simple LED commands directly.

This is a soft access point connection, not the Wi-Fi Direct/P2P protocol.
The server creates a local 2.4 GHz network; no router or internet connection
is needed.

## Configuration

Set the access point name and password in
`pico_server/src/wifi_ap_config.h`. Set matching values in
`pico_client/src/wifi_client_config.h`. The WPA2 password must be 8 to 63
characters long and the SSID at most 32 bytes. The client connects to the
server at `192.168.4.1`; update the address constants in
`wifi_client_config.h` if the server network address changes.

Change the example password before use; it is public. Do not reuse personal
passwords or publish your chosen password in the repository.

## Build

Install the Pico SDK and ARM compiler as described in the
[main README](../../README.md), then set `PICO_SDK_PATH`, for example:

```bash
export PICO_SDK_PATH="$HOME/pico/pico-sdk"
```

### Pico W (RP2040)

Build the server:

```bash
cd exemples/wifi_direct_mqtt_pico_pico/pico_server/src
cmake -S . -B build_peer_picow -DPICO_BOARD=pico_w
cmake --build build_peer_picow -j4
```

Build the client in a separate terminal, starting from the repository root:

```bash
cd exemples/wifi_direct_mqtt_pico_pico/pico_client/src
cmake -S . -B build_peer_picow -DPICO_BOARD=pico_w
cmake --build build_peer_picow -j4
```

`PICO_BOARD=pico_w` selects the Pico W board and RP2040 platform. The UF2
files are generated at:

- Server: `pico_server/src/build_peer_picow/picow_wifi_peer_server.uf2`
- Client: `pico_client/src/build_peer_picow/picow_wifi_peer_client.uf2`

Use separate build directories per project and board. To build for Pico 2 W
instead, use `-DPICO_BOARD=pico2_w` and a distinct directory such as
`build_peer_pico2w`; its UF2 files are named
`pico2w_wifi_peer_server.uf2` and `pico2w_wifi_peer_client.uf2`.

## Use

1. Flash the server UF2 to one Pico and the client UF2 to the other.
2. Start the server first. It creates the Wi-Fi network and listens for the
   client. The client joins that network and reconnects to the server if the
   TCP connection is lost.
3. Connect a PC to either Pico's USB port and open its serial monitor at
   **115200 baud**. Enter `ON`, `OFF`, or `TOGGLE`, followed by Enter.
   `LED ON`, `LED OFF`, and `LED TOGGLE` are also accepted.
   Enter `??` to identify the connected board as `CLIENT` or `SERVER`.
4. The command changes the LED on the USB-connected Pico and is forwarded to
   the other Pico. Each board can therefore be connected to the PC to control
   both LEDs.

Commands received over Wi-Fi are not forwarded again, preventing an echo
loop. If the peer is not connected yet, the most recent LED state set over
USB is queued and sent when the TCP connection becomes available.

## Troubleshooting

- The client USB serial should print `Pico Wi-Fi client started`, then
  `Connected to Wi-Fi network Pico-LED` and `Connected to Wi-Fi peer`.
  `Wi-Fi unavailable (status ...)` means it is still retrying; verify the
  server is running and both Wi-Fi configuration headers match.
- Check the SSID and password in both configuration headers.
- Keep both boards close together and use 2.4 GHz Wi-Fi.
- Open both USB serial monitors to see the server's `Wi-Fi client connected`
  and client's `Connected to Wi-Fi peer` messages.
- Confirm each board was flashed with the correct client or server UF2 for
  its board model.
