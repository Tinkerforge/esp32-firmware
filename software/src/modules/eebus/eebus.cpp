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

#include "build.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "node_state.enum.h"
#include "ship.h"
#include "ship_connection_state.enum.h"
#include "ship_discovery_state.enum.h"
#include <LittleFS.h>
#include <TFJson.h>

void EEBus::pre_setup()
{
    // Use PSRAM 128kB for trace buffer for now. We can reduce it if necessary.
    this->trace_buffer_index = logger.alloc_trace_buffer("eebus", 32768 * 4);

    // TOOD: Rework API so this lot is a bit cleaner
    config_peers_prototype = Config::Object({
        {"ip", Config::Str("", 7, 1000)}, // Long string as all IPs the device has will be saved. TODO: Do this another way
        {"port", Config::Uint16(0)},
        {"trusted", Config::Bool(false)},
        {"dns_name", Config::Str("", 0, 63)},
        // [SHIP 7.3] A local SHIP node SHALL assign a unique multicast DNS host name of up to 63 bytes
        {"id", Config::Str("", 0, 63)},
        // [SHIP 7.3.2] The value of the id key contains a globally unique ID of the SHIP node and has a maximum length of 63 bytes
        {"wss_path", Config::Str("", 0, 32)}, // [SHIP 7.3.2] The maximum length of the path value will be 32 bytes
        {"ski", Config::Str("", 0, 40)},      // [RFC 5280, §4.2.1.2] Subject Key Identifier uses 40 hex characters
        {"autoregister", Config::Bool(false)},
        {"model_brand", Config::Str("", 0, 32)},
        // [SHIP 7.3.2] The maximum length of the brand, type and model values will be 32 byte
        {"model_model", Config::Str("", 0, 32)},
        {"mode_type", Config::Str("", 0, 32)},
        {"state", Config::Enum(NodeState::Disconnected)},
    });

    config = ConfigRoot{Config::Object({
                            {"enable", Config::Bool(false)},
                            {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
                            {"key_id", Config::Int(-1, -1, MAX_CERT_ID)},
                            {"peers", Config::Array({config_peers_prototype}, &config_peers_prototype, 0, MAX_PEER_REMEMBERED, Config::type_id<Config::ConfObject>())},
                        }),
                        [this](Config &update, ConfigSource source) -> String {
                            if (boot_stage == BootStage::LOOP) {
                                if (update.get("enable")->asBool() != config.get("enable")->asBool()) {
                                    task_scheduler.scheduleOnce([this]() {
                                        this->toggle_module();
                                    });
                                }
                            }

                            return "";
                        }};
    add_peer = ConfigRoot{Config::Object({{"ip", Config::Str("0.0.0.0", 7, 15)}, {"port", Config::Uint16(0)}, {"trusted", Config::Bool(false)}, {"dns_name", Config::Str("", 0, 63)}, {"wss_path", Config::Str("", 0, 32)}, {"ski", Config::Str("", 0, 40)}}), [this](Config &add_peer, ConfigSource source) -> String {
                              if (add_peer.get("ski")->asString().isEmpty()) {
                                  return "Can't add peer. Ski is missing.";
                              }
                              if (config.get("peers")->count() == MAX_PEER_REMEMBERED) {
                                  return "Can't add peer. Already have the maximum number of peers.";
                              }
                              return "";
                          }};

    add_peer.set_permit_null_updates(false);

    remove_peer = ConfigRoot{Config::Object({{"ski", Config::Str("", 0, 40)}}), [this](Config &remove_peer, ConfigSource source) -> String {
                                 if (remove_peer.get("ski")->asString().isEmpty()) {
                                     return "Can't remove peer. Ski is missing.";
                                 }
                                 return "";
                             }};
    scan_command = ConfigRoot(Config::Object({}));

    state = Config::Object({
        {"ski", Config::Str("", 0, 40)},
        {"discovery_state", Config::Enum(ShipDiscoveryState::Ready)},
    });

    // A list of all charges, ideally with their cost and which percentage of it was self produced energy
    charges_prototype = Config::Object({{"id", Config::Uint16(0)}, {"charged_kwh", Config::Float(0)}, {"start_time", Config::Uint32(0)}, {"duration", Config::Uint16(0)}, {"cost", Config::Float(0)}, {"percent_self_produced_energy", Config::Uint16(0)}, {"percent_self_produced_cost", Config::Uint16(0)}});

    // Currently eebus state and eebus config are one config. Maybe split them?
    eebus_usecase_state = Config::Object({
        {"commands_received", Config::Uint16(0)},
        {"commands_sent", Config::Uint16(0)},
        {"charging_summary",
         Config::Array(
             {// Read/Write
              // Usecase EV Charging summary
              charges_prototype},
             &charges_prototype,
             0,
             8,
             Config::type_id<Config::ConfObject>())},
        {"power_consumption_limitation",
         Config::Object({
             // Usecase Limitation of power consumption
             {"usecase_state", Config::Enum(LPCState::Init)},
             {"limit_active", Config::Bool(false)},
             {"current_limit", Config::Uint16(0)},
             {"failsafe_limit_power_w", Config::Uint16(0)},    // The limit which may have been set by the energy guard
             {"failsafe_limit_duration_s", Config::Uint32(0)}, // If a failsafe state is entered, how long until this limit is applied before it goes back to default
             {"constraints_power_maximum", Config::Uint16(0)}, // The maximum power consumption the device is capable of
         })},
        {"ev_commissioning_and_configuration",
         Config::Object({{"ev_connected", Config::Bool(false)},
                         {"communication_standard", Config::Str("", 0, 16)}, // "iso15118-2ed1","iso15118-2ed1" or "iec61851"
                         {"asymmetric_charging_supported", Config::Bool(false)},
                         {"mac_address", Config::Str("", 0, 64)},
                         {"minimum_power", Config::Uint16(0)},
                         {"maximum_power", Config::Uint16(0)},
                         {"standby_power", Config::Uint16(0)},
                         {"standby_mode", Config::Bool(false)}})},
        {"evse_commissioning_and_configuration", Config::Object({{"evse_failure", Config::Bool(false)}, {"evse_failure_description", Config::Str("", 0, 64)}})},
    });

    ship.pre_setup();
}

void EEBus::setup()
{
    api.restorePersistentConfig("eebus/config", &config);

    ship.setup();
    update_peers_config();

    initialized = true;
    eebus.trace_fmtln("EEBUS initialized");
}

void EEBus::register_urls()
{
    api.addPersistentConfig("eebus/config", &config);
    api.addState("eebus/state", &state, {}, {}, true);

    api.addState("eebus/usecases", &eebus_usecase_state, {}, {}, false);

    api.addCommand(
        "eebus/add",
        &add_peer,
        {},
        [this](String &errmsg) {
            if (!initialized) {
                eebus.trace_fmtln("Tried adding or editing peer while EEBUS is disabled");
                return;
            }
            if (!errmsg.isEmpty()) {
                // TODO: Some user feedback when this goes wrong
                logger.printfln("An error occurred while adding peer: %s", errmsg.c_str());
                eebus.trace_fmtln("Error adding or Updating peer: %s", errmsg.c_str());
                return;
            }
            String ski = add_peer.get("ski")->asString();
            ship.peer_handler.update_ip_by_ski(ski, add_peer.get("ip")->asString());
            ship.peer_handler.update_port_by_ski(ski, add_peer.get("port")->asUint());
            ship.peer_handler.update_trusted_by_ski(ski, add_peer.get("trusted")->asBool());
            ship.peer_handler.update_dns_name_by_ski(ski, add_peer.get("dns_name")->asString());
            ship.peer_handler.update_wss_path_by_ski(ski, add_peer.get("wss_path")->asString());
            update_peers_config();
            ship.connect_trusted_peers();
        },
        true);

    api.addCommand(
        "eebus/remove",
        &remove_peer,
        {"ski"},
        [this](String &errmsg) {
            if (!config.get("enable")->asBool()) {
                eebus.trace_fmtln("Tried removing peer while EEBUS is disabled");
                return;
            }

            if (!errmsg.isEmpty()) {
                eebus.trace_fmtln("Error removing peer: %s", errmsg.c_str());
                return;
            }
            ship.peer_handler.remove_peer_by_ski(remove_peer.get("ski")->asString());
            update_peers_config();
        },
        true);

    api.addCommand(
        "eebus/scan",
        &scan_command,
        {},
        [this](String &errmsg) {
            if (!config.get("enable")->asBool()) {
                return "EEBUS is disabled";
            }
            if (ship.discovery_state == ShipDiscoveryState::Scanning) {
                return "scan in progress";
            }
            if (ship.discovery_state == ShipDiscoveryState::Ready || ship.discovery_state == ShipDiscoveryState::ScanDone) {
                state.get("discovery_state")->updateEnum(ShipDiscoveryState::Scanning);

                task_scheduler.scheduleOnce(
                    [this]() {
                        ship.discover_ship_peers();
                        update_peers_config();
                        ship.connect_trusted_peers();
                    },
                    1_s);

                return "scan started";
            }
            if (ship.discovery_state == ShipDiscoveryState::Error) {
                ship.discovery_state = ShipDiscoveryState::Ready;
                return "scan error";
            }
            return "scan done";
        },
        true);

    // toggle_module will register URI handlers and cannot be started during the setup stage.
    toggle_module();
}

void EEBus::toggle_module()
{
    // All peers are unknown when its either toggled or at startup
    for (size_t i = 0; i < config.get("peers")->count(); i++) {
        config.get("peers")->get(i)->get("state")->updateEnum(NodeState::Disconnected);
    }
    api.writeConfig("eebus/config", &config);

    if (config.get("enable")->asBool()) {
        module_enabled = true;
        usecases = make_unique_psram<EEBusUseCases>();
        data_handler = make_unique_psram<SpineDataTypeHandler>();
        ship.enable_ship();
        logger.printfln("EEBUS Module enabled");

    } else {
        module_enabled = false;
        usecases = nullptr;
        data_handler = nullptr;
        ship.disable_ship();
        if (initialized) {
            logger.printfln("EEBUS Module disabled");
        }
    }
}

// ManufacturerName-Model-UniqueID
String EEBus::get_eebus_name()
{
    const char *manufacturer = OPTIONS_MANUFACTURER_USER_AGENT();
    const char *model = api.getState("info/name")->get("type")->asEphemeralCStr();
    const char *uid = api.getState("info/name")->get("uid")->asEphemeralCStr();

    char buffer[64];
    StringWriter sw(buffer, ARRAY_SIZE(buffer));

    sw.puts(manufacturer);
    sw.putc('-');
    sw.puts(model);
    sw.putc('-');
    sw.puts(uid);

    return {buffer, sw.getLength()};
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
    config.get("peers")->removeAll();
    auto peers = ship.peer_handler.get_peers();
    for (const std::shared_ptr<ShipNode> &node : peers) {
        auto peer = config.get("peers")->add();
        peer->get("ip")->updateString(node->ip_address_as_string());
        peer->get("port")->updateUint(node->port);
        peer->get("trusted")->updateBool(node->trusted);
        peer->get("dns_name")->updateString(node->dns_name);
        peer->get("id")->updateString(node->txt_id);
        peer->get("wss_path")->updateString(node->txt_wss_path);
        peer->get("ski")->updateString(node->txt_ski);
        peer->get("autoregister")->updateBool(node->txt_autoregister);
        peer->get("model_brand")->updateString(node->txt_brand);
        peer->get("model_model")->updateString(node->txt_model);
        peer->get("mode_type")->updateString(node->txt_type);
        peer->get("state")->updateEnum(node->state);
    }
    api.writeConfig("eebus/config", &config);
}

void EEBus::trace_strln(const char *str, const size_t length)
{
#if defined(BOARD_HAS_PSRAM)
    uint32_t secs = now_us().to<millis_t>().as<uint32_t>();
    char buffer[32] = {0};
    sprintf(buffer, "%04lu ", secs % 10000);
    logger.trace_plain(trace_buffer_index, buffer, strlen(buffer));

    logger.trace_plain(trace_buffer_index, str, length);
    logger.trace_plain(trace_buffer_index, "\n", 1);
#endif
}

void EEBus::trace_jsonln(JsonVariantConst data)
{
#if defined(BOARD_HAS_PSRAM)
    String string = data.as<String>();
    const char *str = string.c_str();
    const size_t length = strlen(str);
    trace_strln(str, length);
#endif
}

void EEBus::trace_fmtln(const char *fmt, ...)
{
#if defined(BOARD_HAS_PSRAM)
    uint32_t secs = now_us().to<millis_t>().as<uint32_t>();
    char buffer[32] = {0};
    sprintf(buffer, "%04lu ", secs % 10000);
    logger.trace_plain(trace_buffer_index, buffer, strlen(buffer));

    va_list args;
    va_start(args, fmt);
    logger.vtracefln_plain(trace_buffer_index, fmt, args);
    va_end(args);
#endif
}

void EEBus::set_own_ski(const String &ski)
{
    state.get("ski")->updateString(ski);
}
