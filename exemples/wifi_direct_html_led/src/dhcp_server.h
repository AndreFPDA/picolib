#ifndef DHCP_SERVER_H
#define DHCP_SERVER_H

#include <stdbool.h>

#include "lwip/netif.h"

bool dhcp_server_start(struct netif *netif);
void dhcp_server_stop(void);

#endif
