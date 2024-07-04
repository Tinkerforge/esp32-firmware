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

    bool was_connected = false;
    uint32_t last_connected = 0;

    EthernetState get_connection_state() const;
    bool is_enabled() const;

    void print_con_duration();

private:
    ConfigRoot config;
    ConfigRoot state;

    OwnedConfig config_in_use;
    String hostname;
    EthernetState connection_state = EthernetState::NotConfigured;
};
