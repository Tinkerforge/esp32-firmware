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

#include "ship_peer_handler.h"

#include <algorithm>

#include "eebus.h"
#include "generated/module_dependencies.h"

void ShipNode::as_json(StringBuilder *sb)
{
    size_t strs_len = dns_name.length() + txt_id.length() + txt_wss_path.length() + txt_ski.length() + txt_brand.length() + txt_model.length() + txt_type.length();
    size_t ips_len = 0;
    for (const String &ip : ip_address) {
        ips_len += ip.length();
    }
    const size_t capacity = JSON_OBJECT_SIZE(12) + JSON_ARRAY_SIZE(ip_address.size()) + strs_len + ips_len + 128;
    DynamicJsonDocument doc(capacity);
    doc["name"] = dns_name.c_str();
    doc["id"] = txt_id.c_str();
    doc["ws_path"] = txt_wss_path.c_str();
    doc["ski"] = txt_ski.c_str();
    doc["allow_autoregister"] = txt_autoregister;
    doc["device_manufacturer"] = txt_brand.c_str();
    doc["device_model"] = txt_model.c_str();
    doc["device_type"] = txt_type.c_str();
    doc["trusted"] = trusted;
    doc["port"] = port;
    doc["state"] = static_cast<uint8_t>(state);

    JsonArray arr = doc.createNestedArray("ip_address");
    for (const String &ip : ip_address) {
        arr.add(ip.c_str());
    }

    size_t len = measureJson(doc);
    char *buf = new char[len + 1];
    serializeJson(doc, buf, len + 1);

    sb->puts(buf);
    delete[] buf;
}

String ShipNode::ip_address_as_string() const
{
    // "0.0.0.0" is the config/state API placeholder for "no IP known yet"
    if (ip_address.empty()) {
        return "0.0.0.0";
    }
    String ip_concat;
    size_t len = ip_address.size();
    if (len > 3) {
        len = 3;
    }
    for (size_t i = 0; i < len; ++i) {
        if (i > 0) {
            ip_concat += ";";
        }
        ip_concat += ip_address[i];
    }
    return ip_concat;
}

String ShipNode::node_name() const
{
    if (!dns_name.isEmpty() && !txt_ski.isEmpty()) {
        return dns_name + ", SKI: " + txt_ski;
    }
    if (!ip_address.empty()) {
        return ip_address.front();
    }
    return "";
}

ShipNode *ShipPeerHandler::get_peer_by_ski(const String &ski)
{
    // An empty SKI must never match anything
    if (ski.isEmpty()) {
        return nullptr;
    }
    for (const auto &n : peers) {
        if (!n->pending_removal && n->txt_ski.equalsIgnoreCase(ski))
            return n.get();
    }
    return nullptr;
}

void ShipPeerHandler::mark_for_removal(ShipNode *node)
{
    if (node == nullptr) {
        return;
    }
    node->pending_removal = true;
    for (const auto &conn : eebus.ship.ship_connections) {
        if (conn->peer_node == node) {
            // Still referenced: Ship::remove erases it when the last connection is gone
            return;
        }
    }
    erase_node(node);
}

void ShipPeerHandler::erase_node(const ShipNode *node)
{
    eebus.trace_fmtln("Erasing removed peer %s", node->node_name().c_str());
    peers.erase(std::ranges::remove_if(peers,
                                       [node](const unique_ptr_any<ShipNode> &n) {
                                           return n.get() == node;
                                       })
                    .begin(),
                peers.end());
}

ShipNode *ShipPeerHandler::get_or_create_by_ski(const String &ski)
{
    if (auto *peer = get_peer_by_ski(ski))
        return peer;
    return new_peer_from_ski(ski);
}

void ShipPeerHandler::update_ip_by_ski(const String &ski, const String &ip, const bool force_front)
{
    // Never let the "0.0.0.0" placeholder enter the address list,
    // e.g. when the UI echoes it back via the eebus/add command
    if (ski.isEmpty() || ip.isEmpty() || ip == "0.0.0.0") {
        return;
    }
    auto *peer = get_or_create_by_ski(ski);
    if (!peer->contains_ip(ip)) {
        if (force_front) {
            peer->ip_address.insert(peer->ip_address.begin(), ip);
        } else {
            peer->ip_address.push_back(ip);
        }
    } else if (force_front && !peer->ip_address.empty() && peer->ip_address[0] != ip) {
        auto it = std::find(peer->ip_address.begin(), peer->ip_address.end(), ip);
        if (it != peer->ip_address.end()) {
            std::rotate(peer->ip_address.begin(), it, it + 1);
        }
    }
}

ShipNode *ShipPeerHandler::new_peer_from_ski(const String &ski)
{
    unique_ptr_any<ShipNode> node = make_unique_psram<ShipNode>();
    node->txt_ski = ski;
    peers.push_back(std::move(node));
    return peers.back().get();
}

void ShipPeerHandler::initialize_from_config()
{
    auto config_peers = eebus.config.get("peers");

    const size_t peer_count = config_peers->count();
    for (size_t i = 0; i < peer_count; i++) {
        auto peer = config_peers->get(i);
        String ski = peer->get("ski")->asString();
        if (ski.isEmpty()) {
            continue;
        }
        if (get_peer_by_ski(ski) != nullptr) {
            continue;
        }
        ShipNode *node = new_peer_from_ski(ski);
        node->trusted = peer->get("trusted")->asBool();
        node->port = static_cast<uint16_t>(peer->get("port")->asUint());
        node->state = NodeState::LoadedFromConfig;
        node->dns_name = peer->get("dns_name")->asString();
        node->txt_id = peer->get("id")->asString();
        node->txt_wss_path = peer->get("wss_path")->asString();
        node->txt_autoregister = peer->get("autoregister")->asBool();

        node->txt_brand = peer->get("model_brand")->asString();
        node->txt_model = peer->get("model_model")->asString();
        node->txt_type = peer->get("model_type")->asString();

        node->persistent = true;

        String ip_list = peer->get("ip")->asString();
        size_t start = 0;
        int end = ip_list.indexOf(';');
        while (end != ip_list.lastIndexOf(';')) {
            node->ip_address.push_back(ip_list.substring(start, end));
            start = end + 1;
            end = ip_list.indexOf(';', start);
        }
        if (start < ip_list.length()) {
            node->ip_address.push_back(ip_list.substring(start));
        }
        // Drop the "0.0.0.0" placeholder, it is not a real address
        node->ip_address.erase(std::remove(node->ip_address.begin(), node->ip_address.end(), String("0.0.0.0")), node->ip_address.end());
    }
    eebus.update_peers_state();
}
