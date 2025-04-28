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

    state = Config::Object({
        {"ski", Config::Str("", 0, 64)},
        {"connections", Config::Uint16(0)},
    });

    ship.pre_setup();
}

void EEBus::setup()
{
    api.restorePersistentConfig("eebus/config", &config);
    //api.restorePersistentConfig("eebus/peers", &peers);
    cleanup_peers();
    update_peers_config();

    initialized = true;
}

void EEBus::register_urls()
{
    api.addPersistentConfig("eebus/config", &config);
    api.addState("eebus/state", &state);

    //api.addPersistentConfig("eebus/peers", &peers);

    /*api.addCommand(
        "eebus/peers/add",
        &add_ship_peer,
        //TODO: Here we should set the parameters required, How can i set optional parameters?
        {},
        [this](String & /*errmsg) {
            logger.printfln("Adding ship peer");
        },
        true);
*/
    api.addCommand(
        "eebus/discover_devices",
        Config::Null(),
        {},
        [this](String & /*errmsg*/) {
            ship.scan_skis();
            update_peers_config();
        },
        true);
    server.on("/eebus/discovered_devices", HTTP_GET, [this](WebServerRequest request) {
        if (ship.discovery_state != Ship_Discovery_State::SCANNING) {
            return request.send(200, "text/plain; charset=utf-8", "scan in progress");
        }
        if (ship.discovery_state != Ship_Discovery_State::READY) {
            return request.send(200, "text/plain; charset=utf-8", "no scan done yet");
        }

        return request.send(200, "text/plain; charset=utf-8", "scan done");
    });

    ship.setup();
}

void EEBus::cleanup_peers()
{
    for (size_t i = 0; i < config.get("peers")->count(); i++) {
        auto peer = config.get("peers")->get(i);
        if (peer->get("trusted")->asBool() == false) {
            config.get("peers")->remove(i);
            i--;
        }
    }
}

void EEBus::update_peers_config()
{
    size_t currently_configured_count = config.get("peers")->count();
    logger.printfln("Updating peers to config");

    for (ShipNode node : ship.mdns_results) {
        bool found = false;
        // Update existing peer
        for (size_t i = 0; i < currently_configured_count; i++) {
            auto peer = config.get("peers")->get(i);
            if (peer->get("ski")->asString() == node.txt_ski) {
                peer->get("port")->updateUint(node.port);
                peer->get("dns_name")->updateString(node.dns_name);
                peer->get("id")->updateString(node.txt_id);
                peer->get("wss_path")->updateString(node.txt_wss_path);
                peer->get("ski")->updateString(node.txt_ski);
                peer->get("autoregister")->updateBool(node.txt_autoregister);
                peer->get("model_brand")->updateString(node.txt_brand);
                peer->get("model_model")->updateString(node.txt_model);
                peer->get("mode_type")->updateString(node.txt_type);
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
