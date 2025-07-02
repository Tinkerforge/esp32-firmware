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

#include "config.h"
#include "mdns.h"
#include "module.h"
#include "modules/ws/web_sockets.h"
#include "ship_connection.h"
#include <TFJson.h>
//#include "string_builder.h"

enum class NodeState { Unknown = 0, Discovered, Connected };

struct ShipNode {

    // Basic information about the node
    std::vector<IPAddress> ip_addresses;
    uint16_t port = 0;
    bool trusted = false;
    NodeState state = NodeState::Unknown;
    // Stuff that is Mandatory in the TXT record
    String dns_name = "";
    String txt_vers = ""; //Maybe change to number?
    String txt_id = "";
    String txt_wss_path = "";
    String txt_ski = "";
    bool txt_autoregister = false;
    // Stuff that is Optional in the TXT record
    String txt_brand = "";
    String txt_model = "";
    String txt_type = "";

    // TODO Add more stuff that might be relevant like last seen, features, etc.

    void as_json(StringBuilder *sb); /* */
};

enum Ship_Discovery_State { READY = 0, SCANNING, SCAN_DONE, ERROR };

class Ship
{
private:
    void setup_mdns();
    void setup_wss();

    WebSockets web_sockets;

public:
    Ship()
    {
    }

    void pre_setup();
    void setup();
    void remove(const ShipConnection &ship_connection);
    Ship_Discovery_State discover_ship_peers();
    void print_skis(StringBuilder *sb);

    ConfigRoot config;
    ConfigRoot state;

    std::vector<ShipNode> mdns_results;
    Ship_Discovery_State discovery_state;
    std::vector<ShipConnection> ship_connections;
};
