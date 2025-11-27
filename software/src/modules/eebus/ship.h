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

#include "config.h"
#include "malloc.h"
#include "mdns.h"
#include "module.h"
#include "modules/web_server/cert.h"
#include "modules/ws/web_sockets.h"
#include "node_state.enum.h"
#include "ship_connection.h"
#include "ship_discovery_state.enum.h"
#include "tools/string_builder.h"
#include <TFJson.h>
#include <vector>

struct ShipNode {
    // Basic information about the node
    std::vector<String> ip_address{};
    uint16_t port = 0;
    bool trusted = false;
    NodeState state = NodeState::Disconnected;
    // Stuff that is Mandatory in the TXT record
    String dns_name = "";
    String txt_vers = "";
    String txt_id = "";
    String txt_wss_path = "";
    String txt_ski = "";
    bool txt_autoregister = false;
    // Stuff that is Optional in the TXT record
    String txt_brand = "";
    String txt_model = "";
    String txt_type = "";

    // Can add more stuff here that might be relevant like last seen, features, etc.

    void as_json(StringBuilder *sb); /* */
    [[nodiscard]] String ip_address_as_string() const;
    [[nodiscard]] bool contains_ip(const String &ip) const
    {
        for (const String &addr : ip_address) {
            if (addr == ip) {
                return true;
            }
        }
        return false;
    }
    [[nodiscard]] String node_name() const;
};

class ShipConnection;

class ShipPeerHandler
{
public:
    ShipPeerHandler();
    std::vector<std::shared_ptr<ShipNode>> get_peers()
    {
        return peers;
    }

    void reset()
    {
        peers.clear();
    }

    /* Updates identified by SKI (txt_ski). Will add a new peer if no existing one was found with the ip */
    std::shared_ptr<ShipNode> get_peer_by_ski(const String &ski);
    void remove_peer_by_ski(const String &ski);
    void update_ip_by_ski(const String &ski, const String &ip);
    void update_port_by_ski(const String &ski, uint16_t port);
    void update_trusted_by_ski(const String &ski, bool trusted);
    void update_state_by_ski(const String &ski, NodeState state);
    void update_dns_name_by_ski(const String &ski, const String &dns_name);
    void update_vers_by_ski(const String &ski, const String &txt_vers);
    void update_id_by_ski(const String &ski, const String &txt_id);
    void update_wss_path_by_ski(const String &ski, const String &txt_wss_path);
    void update_autoregister_by_ski(const String &ski, bool autoregister);
    void update_brand_by_ski(const String &ski, const String &brand);
    void update_model_by_ski(const String &ski, const String &model);
    void update_type_by_ski(const String &ski, const String &type);

    /* Updates identified by IP (matches any entry in ShipNode::ip_address). Return true on success. */
    std::shared_ptr<ShipNode> get_peer_by_ip(const String &ip);
    void remove_peer_by_ip(const String &ip);
    void update_port_by_ip(const String &ip, uint16_t port);
    void update_trusted_by_ip(const String &ip, bool trusted);
    void update_state_by_ip(const String &ip, NodeState state);
    void update_dns_name_by_ip(const String &ip, const String &dns_name);
    void update_vers_by_ip(const String &ip, const String &txt_vers);
    void update_id_by_ip(const String &ip, const String &txt_id);
    void update_wss_path_by_ip(const String &ip, const String &txt_wss_path);
    void update_ski_by_ip(const String &ip, const String &txt_ski);
    void update_autoregister_by_ip(const String &ip, bool autoregister);
    void update_brand_by_ip(const String &ip, const String &brand);
    void update_model_by_ip(const String &ip, const String &model);
    void update_type_by_ip(const String &ip, const String &type);
    void update_ip_by_ip(const String &ip, const String &new_ip);

    void new_peer_from_ski(const String &ski);
    void new_peer_from_ip(const String &ip);

private:
    std::vector<std::shared_ptr<ShipNode>> peers{};
};

class Ship
{
public:
    Ship() = default;

    void pre_setup();
    void setup();

    void enable_ship();
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

    ShipPeerHandler peer_handler{};
    //std::vector<ShipNode> ship_nodes_discovered;
    ShipDiscoveryState discovery_state = ShipDiscoveryState::Ready;
    std::vector<unique_ptr_any<ShipConnection>> ship_connections;
    bool is_enabled{};

private:
    static void setup_mdns();
    void setup_wss();

    WebSockets web_sockets;
    Cert cert;
    bool wss_registered = false;
};
