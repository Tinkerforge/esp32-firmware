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
#include "mdns.h"
#include "cert.h"
#include "modules/ws/web_sockets.h"
#include "ship_connection.h"
#include "ship_discovery_state.enum.h"
#include "node_state.enum.h"
#include <TFJson.h>
//#include "string_builder.h"

struct ShipNode
{

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

class Ship
{
public:
    Ship()
    {
    }

    void pre_setup();
    void setup();

    void disable_ship();

    void remove(const ShipConnection &ship_connection);
    ShipDiscoveryState discover_ship_peers();
    void print_skis(StringBuilder *sb);

    /**
     * Attempts to connect to all unconnected peers that are marked as trusted.
     */
    void connect_trusted_peers();

    //ConfigRoot config;
    //ConfigRoot state;

    std::vector<ShipNode> mdns_results;
    ShipDiscoveryState discovery_state;
    std::vector<unique_ptr_any<ShipConnection>> ship_connections;
    bool is_enabled;

private:
    void setup_mdns();
    void setup_wss();

    WebSockets web_sockets;
    unique_ptr_any<Cert> cert = nullptr;
    httpd_handle_t httpd = nullptr;

};