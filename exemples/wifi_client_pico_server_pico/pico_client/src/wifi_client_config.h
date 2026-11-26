#ifndef WIFI_CLIENT_CONFIG_H
#define WIFI_CLIENT_CONFIG_H

// Keep these values in sync with pico_server/src/wifi_ap_config.h.
#define WIFI_SSID "Pico-LED"
#define WIFI_PASSWORD "picoled123"

// The server access point uses the default address from dhcp_server.c.
#define SERVER_IP_OCTET_1 192
#define SERVER_IP_OCTET_2 168
#define SERVER_IP_OCTET_3 4
#define SERVER_IP_OCTET_4 1

#endif
