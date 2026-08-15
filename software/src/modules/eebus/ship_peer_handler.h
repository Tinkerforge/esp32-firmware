/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include <WString.h>
#include <vector>

#include "generated/node_state.enum.h"
#include "tools/malloc.h"
#include "tools/string_builder.h"

struct ShipNode {
    std::vector<String> ip_address{};
    uint16_t port = 0;
    bool trusted = false;
    NodeState state = NodeState::Disconnected;
    // Mandatory TXT record fields
    String dns_name = "";
    String txt_vers = "";
    String txt_id = "";
    String txt_wss_path = "";
    String txt_ski = "";
    bool txt_autoregister = false;
    // Optional TXT record fields
    String txt_brand = "";
    String txt_model = "";
    String txt_type = "";
    // If true, peer is stored in persistent config
    bool persistent = false;
    // Removed by the user but still referenced by closing connections.
    // Invisible to lookups; erased when the last connection is gone.
    bool pending_removal = false;

    void as_json(StringBuilder *sb);
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

// Sole owner of all ShipNodes; every ShipNode pointer handed out is non-owning.
// Nodes are erased on user removal, deferred until no connection references them.
// Never capture a ShipNode pointer in a delayed lambda: capture the SKI and re-resolve.
class ShipPeerHandler
{
public:
    const std::vector<unique_ptr_any<ShipNode>> &get_peers()
    {
        return peers;
    }

    // Returns nullptr for unknown, empty or pending_removal SKIs
    ShipNode *get_peer_by_ski(const String &ski);
    // Get existing peer or create a new one. Never returns nullptr.
    // Must not be called with an empty SKI.
    ShipNode *get_or_create_by_ski(const String &ski);

    // Tombstone the node; erases it immediately if no connection references it
    void mark_for_removal(ShipNode *node);
    void erase_node(const ShipNode *node);

    void update_ip_by_ski(const String &ski, const String &ip, bool force_front = false);

    void initialize_from_config();

private:
    ShipNode *new_peer_from_ski(const String &ski);

    std::vector<unique_ptr_any<ShipNode>> peers{};
};
