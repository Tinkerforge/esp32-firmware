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

#include <lwip/ip4_addr.h>

#include "module.h"
#include "config.h"
#include "ethernet_state.enum.h"

class Ethernet final : public IModule
{
public:
    Ethernet(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    EthernetState get_connection_state() const;
    bool is_enabled() const;

    void print_con_duration();

private:

    struct eth_runtime {
        ip4_addr_t ip;
        ip4_addr_t gateway;
        ip4_addr_t dns;
        ip4_addr_t dns2;
        uint8_t subnet_cidr;

        EthernetState connection_state;
        bool was_connected;
        uint32_t last_connected_s;
    };

    ConfigRoot config;
    ConfigRoot state;

    eth_runtime *runtime_data = nullptr;
};
