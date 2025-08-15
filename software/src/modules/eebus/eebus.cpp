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

    this->trace_buffer_index = logger.alloc_trace_buffer(
        "eebus",
        32768
            * 4); // This makes the PSRAM usage gigantic even when the module is disabled but we cant set this anywhere else. Maybe make smaller?

    // TODO: Fix string lengths. Spec says they are shorter

    // TOOD: Rework API so this lot is a bit cleaner
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

    switch_enable_config =
        ConfigRoot{Config::Object({{"enable", Config::Bool(false)}}), [this](Config &config, ConfigSource source) -> String {
                       return "";
                   }};

    config = ConfigRoot{Config::Object({
                            {"enabled", Config::Bool(false)},
                            {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
                            {"key_id", Config::Int(-1, -1, MAX_CERT_ID)},
                            {"peers",
                             Config::Array({config_peers_prototype},
                                           &config_peers_prototype,
                                           0,
                                           MAX_PEER_REMEMBERED,
                                           Config::type_id<Config::ConfObject>())},
                        }),
                        [this](Config &config, ConfigSource source) -> String {
                            logger.printfln("Updating config");
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

    remove_peer = ConfigRoot{Config::Object({{"ski", Config::Str("", 0, 64)}}), [this](Config &remove_peer, ConfigSource source) -> String {
                                 if (remove_peer.get("ski")->asString().isEmpty()) {
                                     return "Can't remove peer. Ski is missing.";
                                 }
                                 return "";
                             }};
    scan_command = ConfigRoot(Config::Object({}));

    state = Config::Object({
        {"enabled", Config::Bool(false)},
        {"ski", Config::Str("", 0, 64)},
        {"discovery_state", Config::Uint8(0)},
        {"connections",
         Config::Array({Config::Object({
                           {"ski", Config::Str("", 0, 64)},
                           {"ship_state", Config::Str("", 0, 64)},
                       })},
                       &state_connections_prototype,
                       0,
                       MAX_PEER_REMEMBERED,
                       Config::type_id<Config::ConfObject>())},
    });

    device_name = DNS_SD_UUID;
    ship.pre_setup();
}

void EEBus::setup()
{
    api.restorePersistentConfig("eebus/config", &config);

    is_enabled = config.get("enabled")->asBool();

    toggle_module(is_enabled);
    update_peers_config();

    // All peers are unknown at startup
    for (size_t i = 0; i < config.get("peers")->count(); i++) {
        config.get("peers")->get(i)->get("state")->updateUint(0);
    }

    state.get("connections")->removeAll();

    initialized = true;
    logger.printfln("EEBUS initialized");
    logger.tracefln(this->trace_buffer_index, "EEBUS initialized");
}

void EEBus::register_urls()
{

    api.addPersistentConfig("eebus/config", &config);
    api.addState("eebus/state", &state, {}, {}, true);

    api.addCommand(
        "eebus/addPeer",
        &add_peer,
        {"ip", "port", "trusted", "dns_name", "wss_path", "ski"},
        [this](String &errmsg) {
            if (!initialized) {
                logger.tracefln(this->trace_buffer_index, "Tried adding or editing peer while EEBUS is disabled");
                return;
            }
            if (!errmsg.isEmpty()) {
                // TODO: Some user feedback when this goes wrong
                logger.printfln("An error occurred while adding peer: %s", errmsg.c_str());
                logger.tracefln(this->trace_buffer_index, "Error adding or Updating peer: %s", errmsg.c_str());
                return;
            }
            Config::Wrap peer = nullptr;
            bool found = false;
            for (size_t i = 0; i < config.get("peers")->count(); i++) {
                auto p = config.get("peers")->get(i);
                if (p->get("ski")->asString() == add_peer.get("ski")->asString()) {
                    peer = p;
                    found = true;
                    logger.tracefln(this->trace_buffer_index,
                                    "Updating ship peer %s with ip %s",
                                    peer->get("ski")->asString().c_str(),
                                    peer->get("ip")->asString().c_str());
                    break;
                }
            }
            if (!found) {
                peer = config.get("peers")->add();
                logger.tracefln(this->trace_buffer_index,
                                "Adding ship peer %s with ip %s",
                                peer->get("ski")->asString().c_str(),
                                peer->get("ip")->asString().c_str());
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
            if (!is_enabled) {
                logger.tracefln(this->trace_buffer_index, "Tried removing peer while EEBUS is disabled");
                return;
            }

            if (!errmsg.isEmpty()) {
                logger.tracefln(this->trace_buffer_index, "Error removing peer: %s", errmsg.c_str());
                return;
            }
            for (size_t i = 0; i < config.get("peers")->count(); i++) {
                auto peer = config.get("peers")->get(i);
                if (peer->get("ski")->asString() == remove_peer.get("ski")->asString()) {
                    logger.tracefln(this->trace_buffer_index,
                                    "Removing ship peer %s with ip %s",
                                    peer->get("ski")->asString().c_str(),
                                    peer->get("ip")->asString().c_str());
                    config.get("peers")->remove(i);
                    break;
                }
            }
            api.writeConfig("eebus/config", &config);
        },
        true);

    api.addCommand(
        "eebus/enable",
        &switch_enable_config,
        {},
        [this](String &errmsg) {
            bool enabled = switch_enable_config.get("enable")->asBool();
            this->toggle_module(enabled);
        },
        true);

    api.addCommand(
        "eebus/scan",
        &scan_command,
        {},
        [this](String &errmsg) {
            if (!is_enabled) {
                return "EEBUS is disabled";
            }
            if (ship.discovery_state == Ship_Discovery_State::SCANNING) {
                return "scan in progress";
            }
            if (ship.discovery_state == Ship_Discovery_State::READY || ship.discovery_state == Ship_Discovery_State::SCAN_DONE) {
                state.get("discovery_state")->updateUint(Ship_Discovery_State::SCANNING);

                task_scheduler.scheduleOnce(
                [this]() {
                    ship.discover_ship_peers();
                    update_peers_config();
                },
                1_s);
                // we do a bit of a wait here so the user has time to see the feedback on the frontend

                return "scan started";
            }
            if (ship.discovery_state == Ship_Discovery_State::ERROR) {
                ship.discovery_state = Ship_Discovery_State::READY;
                return "scan error";
            }
            return "scan done";
        },
        true);
}
void EEBus::toggle_module(const bool enable)
{
    if (enable == is_enabled && initialized) {
        return;
    }
    is_enabled = enable;
    state.get("enabled")->updateBool(is_enabled);
    config.get("enabled")->updateBool(is_enabled);
    api.writeConfig("eebus/config", &config);

    if (enable) {
        usecases = make_unique_psram<EEBusUseCases>();
        data_handler = make_unique_psram<SpineDataTypeHandler>();
        ship.setup();
        logger.printfln("EEBUS Module enabled");
    } else {
        usecases = nullptr;
        data_handler = nullptr;
        ship.disable_ship();
        if (initialized) {
            logger.printfln("EEBUS Module disabled");
        }
    }
}
String EEBus::get_eebus_name()
{
    return device_name;
}

int EEBus::get_state_connection_id_by_ski(const String &ski)
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

    for (size_t i = 0; i < config.get("peers")->count(); i++) {
        // Cleanup invalid peers
        if (config.get("peers")->get(i)->get("ski")->asString().isEmpty()
            || config.get("peers")->get(i)->get("ski")->asString().length() < 1) {
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
