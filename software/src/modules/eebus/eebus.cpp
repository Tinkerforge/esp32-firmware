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
#include <TFJson.h>

void EEBus::pre_setup()
{
    config = ConfigRoot{Config::Object({{"cert_id", Config::Int(-1, -1, MAX_CERT_ID)}, {"key_id", Config::Int(-1, -1, MAX_CERT_ID)}}),
                        [this](Config &update, ConfigSource source) -> String {
                            return "";
                        }};

    state = Config::Object({
        {"ski", Config::Str("", 0, 64)},
    });

    ship_peer_prototye = Config::Object({
        // Information important for establishing connection
        {"ip", Config::Str("", 0, 20)},
        {"port", Config::Uint(0, 0, 65535)},
        {"trusted", Config::Bool(false)},
        {"connected", Config::Bool(false)},
        {"dns_name", Config::Str("", 0, 64)},
        // Information not important to the user
        {"txt_vers", Config::Str("", 0, 64)},
        {"txt_id", Config::Str("", 0, 64)},
        {"txt_wss_path", Config::Str("", 0, 64)},
        // Information that helps the user identify the device
        {"txt_ski", Config::Str("", 0, 64)},
        {"txt_autoregister", Config::Bool(false)},
        {"txt_brand", Config::Str("", 0, 64)},
        {"txt_model", Config::Str("", 0, 64)},
        {"txt_type", Config::Str("", 0, 64)},
        {"discovered", Config::Bool(false)},
        // Custom information
        {"display_name", Config::Str("", 0, 64)},
        {"display_notes", Config::Str("", 0, 256)},
    });

    peers = Config::Object({
        {"ship_peers",
         Config::Array({Config::Object({
                           // Information important for establishing connection
                           {"ip", Config::Str("", 0, 20)},
                           {"port", Config::Uint(0, 0, 65535)},
                           {"trusted", Config::Bool(false)},
                           {"connected", Config::Bool(false)},
                           {"dns_name", Config::Str("", 0, 64)},
                           // Information not important to the user
                           {"txt_vers", Config::Str("", 0, 64)},
                           {"txt_id", Config::Str("", 0, 64)},
                           {"txt_wss_path", Config::Str("", 0, 64)},
                           // Information that helps the user identify the device
                           {"txt_ski", Config::Str("", 0, 64)},
                           {"txt_autoregister", Config::Bool(false)},
                           {"txt_brand", Config::Str("", 0, 64)},
                           {"txt_model", Config::Str("", 0, 64)},
                           {"txt_type", Config::Str("", 0, 64)},
                           {"discovered", Config::Bool(false)},
                           // Custom information
                           {"display_name", Config::Str("", 0, 64)},
                           {"display_notes", Config::Str("", 0, 256)},
                       })},
                       &ship_peer_prototye,
                       1,
                       MAX_SHIP_PEER_REMEMBERED,
                       Config::type_id<Config::ConfObject>())},
    });

    add_ship_peer = ConfigRoot{Config::Object({{"ip", Config::Str("", 0, 20)},
                                               {"port", Config::Uint(0, 0, 65535)},
                                               {"trusted", Config::Bool(false)},
                                               {"connected", Config::Bool(false)},
                                               {"dns_name", Config::Str("", 0, 64)},
                                               {"txt_vers", Config::Str("", 0, 64)},
                                               {"txt_id", Config::Str("", 0, 64)},
                                               {"txt_wss_path", Config::Str("", 0, 64)},
                                               {"txt_ski", Config::Str("", 0, 64)},
                                               {"txt_autoregister", Config::Bool(false)},
                                               {"txt_brand", Config::Str("", 0, 64)},
                                               {"txt_model", Config::Str("", 0, 64)},
                                               {"txt_type", Config::Str("", 0, 64)},
                                               {"display_name", Config::Str("", 0, 64)},
                                               {"display_notes", Config::Str("", 0, 256)}}),
                               [this](Config &add_ship_peer, ConfigSource source) -> String {
                                   auto ip = add_ship_peer.get("ip")->asString();
                                   auto port = add_ship_peer.get("port")->asUint();
                                   auto txt_ski = add_ship_peer.get("txt_ski")->asString();
                                   auto trusted = add_ship_peer.get("trusted")->asBool();

                                   Config *peer = nullptr;
                                   // Check if ship peer is known
                                   for (int i = 0; i < peers.get("ship_peers")->count(); i++) {
                                       if (peers.get("ship_peers")->get(i)->get("txt_ski")->asString() == txt_ski) {
                                           peer = (Config *)peers.get("ship_peers")->get(i);
                                           break;
                                       }
                                   }
                                   // TODO: Add the rest of the add/modify logic
                                   return "";
                               }};

    ship.pre_setup();
}

void EEBus::setup()
{
    api.restorePersistentConfig("eebus/config", &config);
    api.restorePersistentConfig("eebus/peers", &peers);

    initialized = true;
}

void EEBus::register_urls()
{
    api.addPersistentConfig("eebus/config", &config);
    api.addState("eebus/state", &state);

    api.addPersistentConfig("eebus/peers", &peers);

    api.addCommand("eebus/peers/add",
                   &add_ship_peer,
                   //TODO: Here we should set the parameters required, How can i set optional parameters?
                   {},
                   [this](String & /*errmsg*/) {
                       logger.printfln("Adding ship peer");
                   }, true);

    api.addCommand(
        "eebus/discover_devices",
        Config::Null(),
        {},
        [this](String & /*errmsg*/) {
            ship.scan_skis();
        },
        true);
    server.on("/eebus/discovered_devices", HTTP_GET, [this](WebServerRequest request) {
        if (ship.discovery_state != Ship_Discovery_State::SCANNING) {
            return request.send(200, "text/plain; charset=utf-8", "scan in progress");
        }
        if (ship.discovery_state != Ship_Discovery_State::READY) {
            return request.send(200, "text/plain; charset=utf-8", "no scan done yet");
        }

        StringBuilder sb;
        sb.setCapacity(2048); // TODO: Calculate size of this
        ship.print_skis(&sb);

        return request.send(200, "application/json", sb.getPtr());
    });

    ship.setup();
}
