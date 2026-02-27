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
    // Persistence flag - if true, peer is stored in persistent config
    bool persistent = false;

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

    std::shared_ptr<ShipNode> get_peer_by_ski(const String &ski);
    std::shared_ptr<ShipNode> get_peer_by_ip(const String &ip);
    // Get existing peer or create a new one. Never returns nullptr.
    ShipNode *get_or_create_by_ski(const String &ski);
    ShipNode *get_or_create_by_ip(const String &ip);

    void remove_peer_by_ski(const String &ski);
    void remove_peer_by_ip(const String &ip);

    void update_ip_by_ski(const String &ski, const String &ip, boolean force_front = false);
    void update_ip_by_ip(const String &ip, const String &new_ip);
    void update_state_by_ip(const String &ip, NodeState state);

    void new_peer_from_ski(const String &ski);
    void new_peer_from_ip(const String &ip);

    void initialize_from_config();

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
    void discover_ship_peers();
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

    // MDNS
    static void check_mdns_results_cb(mdns_search_once_t *);
    void check_mdns_results();
    mdns_search_once_t *mdns_scan;
    void update_discovery_state(ShipDiscoveryState state);

    WebSockets web_sockets;
    Cert cert;
    bool wss_registered = false;
};
