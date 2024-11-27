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

#pragma once

#include "module.h"
#include "config.h"

class Network final : public IModule
{
public:
    Network(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    ConfigRoot config;
    ConfigRoot state;

    inline bool is_connected() { return connected; }

private:
    void update_connected();

    bool connected = false;

    bool ethernet_connected = false;
    bool wifi_sta_connected = false;
    uint32_t wifi_ap_sta_count = 0;
};
