/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "generated/module_dependencies.h"

#if MODULE_ETHERNET_AVAILABLE()
#include <lwip/netif.h>

extern "C" netif *__real_ip4_route_src_hook(const ip4_addr_t *source, const ip4_addr_t *destination);

// ESP-IDF normally selects the interface owning the source address first.
// Replies from the Ethernet management address to NAT AP clients must instead
// use the directly connected AP. Wrap the hook so the precompiled lwIP and its
// original routing policy can otherwise stay unchanged.
extern "C" netif *__wrap_ip4_route_src_hook(const ip4_addr_t *source, const ip4_addr_t *destination)
{
#if IP_NAPT
    if (source && !ip4_addr_isany(source) && destination && !ip4_addr_ismulticast(destination)) {
        bool local_source = false;
        netif *port;
        NETIF_FOREACH(port) {
            if (netif_is_up(port) && netif_is_link_up(port) && ip4_addr_eq(source, netif_ip4_addr(port))) {
                local_source = true;
                break;
            }
        }

        if (local_source) {
            NETIF_FOREACH(port) {
                if (port->napt && netif_is_up(port) && netif_is_link_up(port)
                    && !ip4_addr_isany(netif_ip4_addr(port))
                    && ip4_addr_netcmp(destination, netif_ip4_addr(port), netif_ip4_netmask(port))
                    && !ip4_addr_eq(destination, netif_ip4_addr(port))
                    && !ip4_addr_isbroadcast(destination, port)) {
                    return port;
                }
            }
        }
    }
#endif
    return __real_ip4_route_src_hook(source, destination);
}
#endif
