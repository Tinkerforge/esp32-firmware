/* esp32-firmware
 * Copyright (C) 2022 Erik Fleckstein <erik@tinkerforge.com>
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

#include <WireGuard-ESP32.h>

#include "module.h"
#include "config.h"

class Wireguard final : public IModule
{
public:
    Wireguard(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    bool port_used(uint32_t port);

private:
    void start_wireguard();

    ConfigRoot config;
    ConfigRoot state;
    WireGuard wg;

    String private_key;
    String remote_host;

    micros_t last_connected = 0_us;
};
