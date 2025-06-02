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

#include "main_dependencies.h"
#include "net.h"

#include "event_log_prefix.h"

#include "gcc_warnings.h"

bool is_in_subnet(const IPAddress &ip, const IPAddress &subnet, const IPAddress &to_check)
{
    return (static_cast<uint32_t>(ip) & static_cast<uint32_t>(subnet)) == (static_cast<uint32_t>(to_check) & static_cast<uint32_t>(subnet));
}

bool is_valid_subnet_mask(const IPAddress &subnet)
{
    // IPAddress is in network byte order!
    const uint32_t inverted = ~ntohl(static_cast<uint32_t>(subnet));
    return ((inverted + 1) & inverted) == 0;
}

uint8_t tf_ip4addr_mask2cidr(const ip4_addr_t subnet)
{
    return static_cast<uint8_t>(__builtin_clz(~ntohl(subnet.addr)));
}

ip4_addr_t tf_ip4addr_cidr2mask(uint32_t cidr)
{
    ip4_addr_t mask;

    if (cidr >= 32) {
        mask.addr = ~0ul;
    } else {
        mask.addr = htonl(~0ul << (32 - cidr));
    }

    return mask;
}

#if defined(__GNUC__)
    #pragma GCC diagnostic push

    // IPADDR_LOOPBACK expands to an old-style cast that is also useless
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    #pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

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

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

void poke_localhost()
{
    esp_netif_tcpip_exec(poke_localhost_fn, nullptr);
}
