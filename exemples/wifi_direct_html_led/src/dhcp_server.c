#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "lwip/ip4_addr.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "dhcp_server.h"

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
#define DHCP_FIXED_SIZE 240
#define DHCP_MAX_SIZE 548
#define LEASE_COUNT 8
#define LEASE_FIRST 16
#define LEASE_SECONDS 3600

typedef struct {
    uint8_t mac[6];
    absolute_time_t expires;
    bool used;
} lease_t;

static lease_t leases[LEASE_COUNT];
static struct netif *ap_netif;
static struct udp_pcb *dhcp_pcb;

static void dhcp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                      const ip_addr_t *source, u16_t port) {
    (void)arg;
    (void)source;
    if (port != DHCP_CLIENT_PORT || p->tot_len < DHCP_FIXED_SIZE ||
        p->tot_len > DHCP_MAX_SIZE) {
        pbuf_free(p);
        return;
    }

    uint8_t msg[DHCP_MAX_SIZE] = {0};
    size_t len = pbuf_copy_partial(p, msg, p->tot_len, 0);
    pbuf_free(p);
    const uint8_t cookie[] = {99, 130, 83, 99};
    if (len < DHCP_FIXED_SIZE || msg[0] != 1 || msg[1] != 1 ||
        msg[2] != 6 || memcmp(msg + 236, cookie, sizeof(cookie)) != 0) {
        return;
    }

    uint8_t type = 0;
    const uint8_t *requested = NULL;
    const uint8_t *server_id = NULL;
    for (size_t i = DHCP_FIXED_SIZE; i < len;) {
        uint8_t option = msg[i++];
        if (option == 255) break;
        if (option == 0) continue;
        if (i >= len || msg[i] > len - i - 1) return;
        size_t option_len = msg[i++];
        if (option == 53 && option_len == 1) type = msg[i];
        if (option == 50 && option_len == 4) requested = msg + i;
        if (option == 54 && option_len == 4) server_id = msg + i;
        i += option_len;
    }

    const ip4_addr_t *ip = netif_ip4_addr(ap_netif);
    const ip4_addr_t *mask = netif_ip4_netmask(ap_netif);
    if ((type != 1 && type != 3) ||
        (server_id && memcmp(server_id, &ip->addr, 4) != 0)) {
        return;
    }

    int slot = -1;
    for (int i = 0; i < LEASE_COUNT; i++) {
        if (leases[i].used && absolute_time_diff_us(get_absolute_time(), leases[i].expires) <= 0) {
            leases[i].used = false;
        }
        if (leases[i].used && memcmp(leases[i].mac, msg + 28, 6) == 0) {
            slot = i;
        }
    }

    if (type == 3) {
        const uint8_t *address = requested ? requested : msg + 12;
        const uint8_t *ap_address = (const uint8_t *)&ip->addr;
        if (memcmp(address, ap_address, 3) != 0 ||
            address[3] < LEASE_FIRST || address[3] >= LEASE_FIRST + LEASE_COUNT) {
            return;
        }
        int requested_slot = address[3] - LEASE_FIRST;
        if ((slot >= 0 && slot != requested_slot) ||
            (leases[requested_slot].used &&
             memcmp(leases[requested_slot].mac, msg + 28, 6) != 0)) {
            return;
        }
        slot = requested_slot;
    } else if (slot < 0) {
        for (int i = 0; i < LEASE_COUNT; i++) {
            if (!leases[i].used) {
                slot = i;
                break;
            }
        }
    }
    if (slot < 0) {
        printf("DHCP: no free addresses\n");
        return;
    }

    leases[slot].used = true;
    memcpy(leases[slot].mac, msg + 28, 6);
    leases[slot].expires = make_timeout_time_ms(LEASE_SECONDS * 1000);

    uint8_t client_address[4];
    memcpy(client_address, msg + 12, sizeof(client_address));
    msg[0] = 2;
    memset(msg + 16, 0, 12);
    memcpy(msg + 16, &ip->addr, 4);
    msg[19] = LEASE_FIRST + slot;
    memset(msg + 240, 0, sizeof(msg) - 240);
    size_t n = 240;
    msg[n++] = 53; msg[n++] = 1; msg[n++] = type == 1 ? 2 : 5;
    msg[n++] = 54; msg[n++] = 4; memcpy(msg + n, &ip->addr, 4); n += 4;
    msg[n++] = 1; msg[n++] = 4; memcpy(msg + n, &mask->addr, 4); n += 4;
    msg[n++] = 51; msg[n++] = 4;
    msg[n++] = (LEASE_SECONDS >> 24) & 255;
    msg[n++] = (LEASE_SECONDS >> 16) & 255;
    msg[n++] = (LEASE_SECONDS >> 8) & 255;
    msg[n++] = LEASE_SECONDS & 255;
    msg[n++] = 255;

    struct pbuf *reply = pbuf_alloc(PBUF_TRANSPORT, n, PBUF_RAM);
    if (!reply) {
        printf("DHCP: failed to allocate reply\n");
        return;
    }
    pbuf_take(reply, msg, n);
    ip_addr_t destination;
    if (type == 3 && memcmp(client_address, "\0\0\0\0", 4) != 0) {
        memcpy(&destination.addr, client_address, 4);
    } else {
        IP_ADDR4(&destination, 255, 255, 255, 255);
    }
    err_t error = udp_sendto_if(pcb, reply, &destination, DHCP_CLIENT_PORT, ap_netif);
    if (error != ERR_OK) printf("DHCP: failed to send reply: %d\n", error);
    pbuf_free(reply);
}

bool dhcp_server_start(struct netif *netif) {
    ap_netif = netif;
    struct udp_pcb *pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    if (!pcb) {
        printf("DHCP: failed to create socket\n");
        return false;
    }
    ip_set_option(pcb, SOF_BROADCAST);
    err_t error = udp_bind(pcb, IP_ANY_TYPE, DHCP_SERVER_PORT);
    if (error != ERR_OK) {
        printf("DHCP: failed to bind: %d\n", error);
        udp_remove(pcb);
        return false;
    }
    udp_bind_netif(pcb, netif);
    udp_recv(pcb, dhcp_recv, NULL);
    dhcp_pcb = pcb;
    return true;
}

void dhcp_server_stop(void) {
    if (dhcp_pcb) {
        udp_remove(dhcp_pcb);
        dhcp_pcb = NULL;
    }
}
