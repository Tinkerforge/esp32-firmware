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

#include "config.h"

#define MAX_CONNECT_ATTEMPT_INTERVAL_MS (5 * 60 * 1000)

enum class EthernetState {
    NOT_CONFIGURED,
    NOT_CONNECTED,
    CONNECTING,
    CONNECTED
};

class Ethernet
{
public:
    Ethernet();
    void setup();
    void register_urls();
    void loop();

    bool initialized = false;

    bool was_connected = false;

    EthernetState get_connection_state();

private:
    ConfigRoot ethernet_config;
    ConfigRoot ethernet_state;
    ConfigRoot ethernet_force_reset;

    ConfigRoot ethernet_config_in_use;

    uint32_t connect_attempt_interval_ms;
};
