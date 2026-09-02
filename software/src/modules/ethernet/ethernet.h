/* esp32-firmware
 * Copyright (C) 2021 Erik Fleckstein <erik@tinkerforge.com>
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

#pragma once

#include <lwip/ip_addr.h>

#include "config.h"
#include "generated/ethernet_state.enum.h"
#include "module.h"

class Ethernet final : public IModule
{
public:
    Ethernet()
    {
    }
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    EthernetState get_connection_state() const;
    bool is_enabled() const;
    bool is_enabled_in_config() const;

    void print_con_duration();

private:
    void apply_config();
    void apply_ipv6_config();
    void apply_ip_to_interface();
    void apply_ipv6_to_interface();

    static constexpr size_t MAX_STATIC_IPV6_ADDRESSES = 4;
    static_assert(MAX_STATIC_IPV6_ADDRESSES < CONFIG_LWIP_IPV6_NUM_ADDRESSES); // must leave room for link-local address

    struct ip6_addr_with_flags {
        esp_ip6_addr_t addr;
        uint16_t flags;
    };

    struct eth_runtime {
        micros_t last_connected;

        // IPv4 config (from top-level fields). ip4 == 0.0.0.0 means DHCP.
        ip4_addr_t ip4;
        ip4_addr_t gateway4;
        ip4_addr_t dns4;
        ip4_addr_t dns24;
        uint8_t subnet4_cidr;

        // IPv6 config (from ipv6 sub-object)
        ip6_addr_with_flags ip6[MAX_STATIC_IPV6_ADDRESSES];
        uint8_t ip6_address_count;
        bool want_ipv6;
        bool ipv6_enabled;

        EthernetState connection_state;
        bool was_connected;
    };

    Config ip6_prototype;
    ConfigRoot config;
    ConfigRoot state;

    eth_runtime *runtime_data = nullptr;
    bool eth_started = false;

    uint64_t reconnect_task_id = 0;
    uint64_t revert_countdown_task_id = 0;
};
