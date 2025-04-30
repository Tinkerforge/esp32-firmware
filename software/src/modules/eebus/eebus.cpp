/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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
#include "eebus.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "ship.h"
#include <LittleFS.h>
#include <TFJson.h>

void EEBus::pre_setup()
{
    config_peers_prototype = Config::Object({
        {"ip", Config::Str("", 0, 64)},
        {"port", Config::Uint16(0)},
        {"trusted", Config::Bool(false)},
        {"dns_name", Config::Str("", 0, 64)},
        {"id", Config::Str("", 0, 64)},
        {"wss_path", Config::Str("", 0, 64)},
        {"ski", Config::Str("", 0, 64)},
        {"autoregister", Config::Bool(false)},
        {"model_brand", Config::Str("", 0, 64)},
        {"model_model", Config::Str("", 0, 64)},
        {"mode_type", Config::Str("", 0, 64)},
        {"state", Config::Uint8(0)},
    });

    state_connections_prototype = Config::Object({
        {"ski", Config::Str("", 0, 64)},
        {"ship_state", Config::Str("", 0, 64)},
    });

    config = ConfigRoot{Config::Object({
                            {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
                            {"key_id", Config::Int(-1, -1, MAX_CERT_ID)},
                            {"peers",
                             Config::Array({Config::Object({
                                               {"ip", Config::Str("", 0, 64)},
                                               {"port", Config::Uint16(0)},
                                               {"trusted", Config::Bool(false)},
                                               {"dns_name", Config::Str("", 0, 64)},
                                               {"id", Config::Str("", 0, 64)},
                                               {"wss_path", Config::Str("", 0, 64)},
                                               {"ski", Config::Str("", 0, 64)},
                                               {"autoregister", Config::Bool(false)},
                                               {"model_brand", Config::Str("", 0, 64)},
                                               {"model_model", Config::Str("", 0, 64)},
                                               {"mode_type", Config::Str("", 0, 64)},
                                               {"state", Config::Uint8(0)},
                                           })},
                                           &config_peers_prototype,
                                           0,
                                           MAX_PEER_REMEMBERED,
                                           Config::type_id<Config::ConfObject>())},
                        }),
                        [this](Config &config, ConfigSource source) -> String {
                            
                            return "";
                        }};
    add_peer = ConfigRoot{Config::Object({{"ip", Config::Str("", 0, 64)},
                                          {"port", Config::Uint16(0)},
                                          {"trusted", Config::Bool(false)},
                                          {"dns_name", Config::Str("", 0, 64)},
                                          {"wss_path", Config::Str("", 0, 64)},
                                          {"ski", Config::Str("", 0, 64)}}),
                          [this](Config &add_peer, ConfigSource source) -> String {
                              if (add_peer.get("ski")->asString().isEmpty()) {
                                  return "Can't add peer. Ski is missing.";
                              }
                              if (config.get("peers")->count() == MAX_PEER_REMEMBERED) {
                                  return "Can't add peer. Already have the maximum number of peers.";
                              }
                              return "";
                          }};

    add_peer.set_permit_null_updates(false);

    remove_peer = ConfigRoot{Config::Object({{"ski", Config::Str("", 0, 64)}}),
                             [this](Config &remove_peer, ConfigSource source) -> String {
                                 if (remove_peer.get("ski")->asString().isEmpty()) {
                                     return "Can't remove peer. Ski is missing.";
                                 }
                                 return "";
                             }};

    

    state = Config::Object({
        {"ski", Config::Str("", 0, 64)},
        {"connections", Config::Array(
            {Config::Object({
                {"ski", Config::Str("", 0, 64)},
                {"ship_state", Config::Str("", 0, 64)},
            })},
            &state_connections_prototype,
            0,
            MAX_PEER_REMEMBERED,
            Config::type_id<Config::ConfObject>())},
    });

    ship.pre_setup();
}

void EEBus::setup()
{
    api.restorePersistentConfig("eebus/config", &config);
    update_peers_config();

    // All peers are unknown at startup
    for(size_t i= 0; i < config.get("peers")->count(); i++) {
       config.get("peers")->get(i)->get("state")->updateUint(0);
    }

    state.get("connections")->removeAll();

    task_scheduler.scheduleWithFixedDelay(
        [this]() {
            if (ship.discovery_state == Ship_Discovery_State::READY) {
                ship.discover_ship_peers();
                update_peers_config();
            }
        },
        SHIP_AUTODISCOVER_INTERVAL,
        SHIP_AUTODISCOVER_INTERVAL);

    initialized = true;
}

void EEBus::register_urls()
{
    
    api.addPersistentConfig("eebus/config", &config);
    api.addState("eebus/state", &state);


    

    api.addCommand(
        "eebus/addPeer",
        &add_peer,
        {"ip", "port", "trusted", "dns_name", "wss_path", "ski"},
        [this](String &errmsg) {
            if (!errmsg.isEmpty()) {
                logger.printfln("Error adding or Updating peer: %s", errmsg.c_str());
                return;
            }
            Config::Wrap peer = nullptr;
            bool found = false;
            for (size_t i = 0; i < config.get("peers")->count(); i++) {
                auto p = config.get("peers")->get(i);
                if (p->get("ski")->asString() == add_peer.get("ski")->asString()) {
                    peer = p;
                    found = true;
                    logger.printfln("Updating ship peer %s with ip %s", peer->get("ski")->asString().c_str(), peer->get("ip")->asString().c_str());
                    break;
                }
            }
            if (!found) {
                peer = config.get("peers")->add();
                logger.printfln("Adding ship peer %s with ip %s", peer->get("ski")->asString().c_str(), peer->get("ip")->asString().c_str());
            }
            peer->get("ip")->updateString(add_peer.get("ip")->asString());
            peer->get("port")->updateUint(add_peer.get("port")->asUint());
            peer->get("trusted")->updateBool(add_peer.get("trusted")->asBool());
            peer->get("dns_name")->updateString(add_peer.get("dns_name")->asString());
            peer->get("wss_path")->updateString(add_peer.get("wss_path")->asString());
            peer->get("ski")->updateString(add_peer.get("ski")->asString());            
            api.writeConfig("eebus/config", &config);
            
        },
        true);

    api.addCommand(
        "eebus/removePeer",
        &remove_peer,
        {"ski"},
        [this](String &errmsg) {
            if (!errmsg.isEmpty()) {
                logger.printfln("Error removing peer: %s", errmsg.c_str());
                return;
            }
            for (size_t i = 0; i < config.get("peers")->count(); i++) {
                auto peer = config.get("peers")->get(i);
                if (peer->get("ski")->asString() == remove_peer.get("ski")->asString()) {
                    logger.printfln("Removing ship peer %s with ip %s", peer->get("ski")->asString().c_str(), peer->get("ip")->asString().c_str()); 
                    config.get("peers")->remove(i);
                    break;
                }
            }
            api.writeConfig("eebus/config", &config);
            
        },
        true);
  
    // Yes i realize this is not the best way
    server.on("/eebus/scan", HTTP_PUT, [this](WebServerRequest request) {
        if (ship.discovery_state == Ship_Discovery_State::SCANNING) {
            return request.send(200, "text/plain; charset=utf-8", "scan in progress");
        }
        if (ship.discovery_state == Ship_Discovery_State::READY || ship.discovery_state == Ship_Discovery_State::SCAN_DONE) {
            ship.discover_ship_peers();
            update_peers_config();
            return request.send(200, "text/plain; charset=utf-8", "scan started");
        }
        if (ship.discovery_state == Ship_Discovery_State::ERROR) {
            ship.discovery_state = Ship_Discovery_State::READY;
            return request.send(200, "text/plain; charset=utf-8", "scan error");
        }
        return request.send(200, "text/plain; charset=utf-8", "scan done");
    });

    ship.setup();
}

int EEBus::get_connection_id_by_ski(const String &ski)
{
    for (size_t i = 0; i < state.get("connections")->count(); i++) {
        if (state.get("connections")->get(i)->get("ski")->asString() == ski) {
            return i;
        }
    }
    return -1;
}

void EEBus::update_peers_config()
{
    size_t currently_configured_count = config.get("peers")->count();
    logger.printfln("Updating peers to config");

    for (size_t i= 0; i< config.get("peers")->count(); i++) {
        if(config.get("peers")->get(i)->get("ski")->asString().isEmpty() || config.get("peers")->get(i)->get("ski")->asString().length() < 1) {
            config.get("peers")->remove(i);
        }
    }

    for (ShipNode node : ship.mdns_results) {
        bool found = false;
        // Update existing peer
        for (size_t i = 0; i < currently_configured_count; i++) {
            auto peer = config.get("peers")->get(i);
            if (peer->get("ski")->asString() == node.txt_ski) {
                peer->get("port")->updateUint(node.port);
                peer->get("ip")->updateString(node.ip_addresses[0].toString());
                peer->get("dns_name")->updateString(node.dns_name);
                peer->get("id")->updateString(node.txt_id);
                peer->get("wss_path")->updateString(node.txt_wss_path);
                peer->get("ski")->updateString(node.txt_ski);
                peer->get("autoregister")->updateBool(node.txt_autoregister);
                peer->get("model_brand")->updateString(node.txt_brand);
                peer->get("model_model")->updateString(node.txt_model);
                peer->get("mode_type")->updateString(node.txt_type);
                if (peer->get("state")->asUint() != (uint32_t)NodeState::Connected) {
                    peer->get("state")->updateUint((uint32_t)NodeState::Discovered);
                }
                found = true;
                break;
            }

        }
        // Add new peer
        if (!found) {
            auto peer = config.get("peers")->add();
            peer->get("ip")->updateString(node.ip_addresses[0].toString());
            peer->get("port")->updateUint(node.port);
            peer->get("trusted")->updateBool(node.trusted);
            peer->get("dns_name")->updateString(node.dns_name);
            peer->get("id")->updateString(node.txt_id);
            peer->get("wss_path")->updateString(node.txt_wss_path);
            peer->get("ski")->updateString(node.txt_ski);
            peer->get("autoregister")->updateBool(node.txt_autoregister);
            peer->get("model_brand")->updateString(node.txt_brand);
            peer->get("model_model")->updateString(node.txt_model);
            peer->get("mode_type")->updateString(node.txt_type);
            peer->get("state")->updateUint((uint32_t)NodeState::Discovered);
        }
    }
}

