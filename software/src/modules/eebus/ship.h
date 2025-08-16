/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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

#include <vector>

#include "module.h"
#include "config.h"
#include "modules/ws/web_sockets.h"
#include "ship_connection.h"
#include "mdns.h"

class Ship
{
private:
    void setup_mdns();
    void setup_wss();

    WebSockets web_sockets;
    std::vector<ShipConnection> ship_connections;



public:
    Ship(){}
    
    void pre_setup();
    void setup();
    void remove(const ShipConnection &ship_connection);
    void scan_skis();
    void print_skis(StringBuilder *sb);

    ConfigRoot config;
    ConfigRoot state;

    std::vector<mdns_result_t> mdns_results;
};
