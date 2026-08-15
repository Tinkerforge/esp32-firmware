/* esp32-firmware
 * Copyright (C) 2024-2026 Olaf Lüke <olaf@tinkerforge.com>
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
#include "generated/node_state.enum.h"
#include "generated/ship_discovery_state.enum.h"
#include "tools/malloc.h"
#include "mdns.h"
#include "module.h"
#include "modules/web_server/cert.h"
#include "modules/ws/web_sockets.h"
#include "ship_connection.h"
#include "ship_peer_handler.h"
#include "ship_rx_queue.h"
#include "tools/string_builder.h"
#include <TFJson.h>
#include <vector>

static constexpr uint16_t SHIP_PORT = 4712;

class Ship
{
public:
    Ship() = default;

    using RxEvent = ShipRxQueue::RxEvent;

    bool push_rx_event(RxEvent &&event);

    bool ws_send(int fd, const char *payload, size_t payload_len);
    void ws_close(int fd);

    void pre_setup();
    void setup();

    void enable_ship();
    void disable_ship();

    void remove(const ShipConnection &ship_connection);
    // mDNS discovery of all SHIP devices followed by connect_trusted_peers
    void discover_ship_peers();
    void print_skis(StringBuilder *sb);

    void close_connections_by_ski(const String &ski, const String &reason) const;

    // Notify active connections that a peer's trust status may have changed.
    // Connections in the hello pending state will immediately re-check trust.
    void notify_peer_updated(const String &ski) const;

    void connect_trusted_peers();

    void resolve_duplicate_connections(const ShipNode *node);

    ShipPeerHandler peer_handler{};
    ShipDiscoveryState discovery_state = ShipDiscoveryState::Ready;
    std::vector<unique_ptr_any<ShipConnection>> ship_connections;
    bool is_enabled{};

private:
    static void setup_mdns();
    void setup_wss();

    void close_all_but_most_recent(const ShipNode *node);

    void drain_rx_events();
    void process_rx_event(RxEvent &event);
    void handle_connected(const RxEvent &event);
    ShipConnection *find_connection(const RxEvent &event);

    ShipRxQueue rx_queue;

    // MDNS
    static void check_mdns_results_cb(mdns_search_once_t *);
    void check_mdns_results();
    mdns_search_once_t *mdns_scan;
    void update_discovery_state(ShipDiscoveryState state);

    WebSockets web_sockets;
    Cert cert;
    bool wss_registered = false;
    uint64_t autoconnect_timer = 0;
};
