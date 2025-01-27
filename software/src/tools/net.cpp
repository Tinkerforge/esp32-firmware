/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <esp_netif.h>
#include <lwip/udp.h>

#include "net.h"

#include "event_log_prefix.h"
#include "main_dependencies.h"

bool is_in_subnet(IPAddress ip, IPAddress subnet, IPAddress to_check)
{
    return (((uint32_t)ip) & ((uint32_t)subnet)) == (((uint32_t)to_check) & ((uint32_t)subnet));
}

bool is_valid_subnet_mask(IPAddress subnet)
{
    bool zero_seen = false;
    // IPAddress is in network byte order!
    uint32_t addr = ntohl((uint32_t)subnet);
    for (int i = 31; i >= 0; --i) {
        bool bit_is_one = (addr & (1 << i));
        if (zero_seen && bit_is_one) {
            return false;
        } else if (!zero_seen && !bit_is_one) {
            zero_seen = true;
        }
    }
    return true;
}

static esp_err_t poke_localhost_fn(void * /*ctx*/)
{
    udp_pcb *l_udp_pcb = udp_new();
    if (l_udp_pcb) {
        //udp_bind(l_udp_pcb, IP_ADDR_ANY, 0); // pcb will be bound in udp_sendto()

        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_ROM); // PBUF_ROM because we have no payload
        if (p) {
            p->payload = nullptr; // payload can be nullptr because length is 0 and pbuf type is PBUF_ROM
            p->len = 0;
            p->tot_len = 0;

            ip_addr_t dst_addr;
            dst_addr.type = IPADDR_TYPE_V4;
            dst_addr.u_addr.ip4.addr = htonl(IPADDR_LOOPBACK);

            errno = 0;
            err_t err = udp_sendto(l_udp_pcb,p, &dst_addr, 9);
            if (err != ERR_OK) {
                logger.printfln("udp_sendto failed: %i | %s (%i)", err, strerror(errno), errno);
            }

            pbuf_free(p);
        }
        udp_remove(l_udp_pcb);
    }
    return ESP_OK; // Don't care about errors.
}

void poke_localhost()
{
    esp_netif_tcpip_exec(poke_localhost_fn, nullptr);
}
