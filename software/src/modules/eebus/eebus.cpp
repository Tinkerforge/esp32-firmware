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
#include "eebus_usecases.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "node_state.enum.h"
#include "ship.h"
#include "ship_connection_state.enum.h"
#include "ship_discovery_state.enum.h"
#include <LittleFS.h>
#include <TFJson.h>

#include "tools/malloc.h"

extern char local_uid_str[32];

#ifdef EEBUS_MODE_EVSE
static void update_usecases_from_phases(const Config *_phases_cfg)
{
#if defined(EEBUS_ENABLE_EVCC_USECASE) || defined(EEBUS_ENABLE_EVSECC_USECASE)
    if (eebus.usecases == nullptr)
        return;

    bool supports_1p = evse_common.backend->phase_switching_capable() || evse_common.backend->get_phases() == 1;
    bool supports_3p = evse_common.backend->phase_switching_capable() || evse_common.backend->get_phases() == 3;

    auto incoming = evse_common.get_slots().get(CHARGING_SLOT_INCOMING_CABLE)->get("max_current")->asUint();
    auto outgoing = evse_common.get_slots().get(CHARGING_SLOT_OUTGOING_CABLE)->get("max_current")->asUint();

    int milliamps_min = 6000;
    int min_power = milliamps_min / 1000 * (supports_1p ? 1 : 3) * 230;
    int max_power = (std::min(incoming, outgoing) / 1000) * (supports_3p ? 3 : 1) * 230;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    //logger.printfln("Calling evcc.update_electrical_connection(min_power=%d, max_power=%d)", min_power, max_power);
    eebus.usecases->evcc->update_electrical_connection(min_power, max_power);
#endif
#ifdef EEBUS_ENABLE_EVSECC_USECASE
    //logger.printfln("Calling evcem.update_constraints(incoming=%d, outgoing=%d, min_power=%d, max_power=%d)", incoming, outgoing, min_power, max_power);
    eebus.usecases->evcem->update_constraints(milliamps_min, std::min(incoming, outgoing), 100, min_power, max_power, 1000, 0, 1000000, 10);
#endif
#endif
}

static void update_usecases_from_charger_state(const Config *charger_state_cfg)
{
#if defined(EEBUS_ENABLE_EVCC_USECASE) || defined(EEBUS_ENABLE_EVSECC_USECASE)
    if (eebus.usecases == nullptr)
        return;

    auto charger_state = charger_state_cfg->asUint8();

    // It's fine that we report an EV connected if the charger is in CHARGER_STATE_ERROR.
    // We will set the EVSECC operating state to faulted below and in this case the spec states:
    /* If the EVSE has an error [EVSECC-020], the EV may no
       longer be able to follow the charging plan correctly and updates from the EV may no longer contain
       valid data.*/
#ifdef EEBUS_ENABLE_EVCC_USECASE
    //logger.printfln("Calling evcc.ev_connected_state(connected=%d)", charger_state != CHARGER_STATE_NOT_PLUGGED_IN);
    eebus.usecases->evcc->ev_connected_state(charger_state != CHARGER_STATE_NOT_PLUGGED_IN);

    //logger.printfln("Calling evcc.update_device_config(communication_standard=\"%s\")", "iec61851");
    eebus.usecases->evcc->update_device_config("iec61851");
#endif
#ifdef EEBUS_ENABLE_EVSECC_USECASE
    //logger.printfln("Calling evcc.update_operating_state(failure=%d)", charger_state == CHARGER_STATE_ERROR);
    eebus.usecases->evsecc->update_operating_state(charger_state == CHARGER_STATE_ERROR); // TODO: error message
#endif
#endif
}
static int sanitized(float x, float scale)
{
    return isnan(x) ? INT32_MIN : (int)(x * scale);
}

#endif
static void update_evse_limit()
{
    if (eebus.usecases == nullptr) {
        return;
    }

#if MODULE_EVSE_COMMON_AVAILABLE()
    const uint32_t phases = evse_common.backend->get_phases();
    if (phases == 0) {
        return;
    }

    int limit_mA = 32000;

#ifdef EEBUS_ENABLE_LPC_USECASE
    limit_mA = eebus.usecases->lpc->get_current_limit_w() * 1000 / 230 / phases;
#ifdef EEBUS_ENABLE_OPEV_USECASE
    if (!eebus.usecases->limitation_of_power_consumption.limit_is_active() && eebus.usecases->overload_protection_by_ev_charging_current_curtailment.limit_is_active()) {
        auto limit_phases = eebus.usecases->overload_protection_by_ev_charging_current_curtailment.get_limit_milliamps();
        if (limit_phases[0] != limit_phases[1] || limit_phases[0] != limit_phases[2]) {
            eebus.trace_fmtln("OPEV attempted to apply an asymmetric limit which is not supported. Ignoring OPEV limit.");
            return;
        }
        limit_mA = limit_phases[0] * 1000 / 230 / phases;
    }
#endif
#endif
    limit_mA = std::min(limit_mA, 32000);
    limit_mA = std::max(limit_mA, 6000);

    evse_common.set_eebus_current(static_cast<uint16_t>(limit_mA));
#endif
}

#ifdef EEBUS_DEV_TEST_ENABLE
size_t dev_test_iteration = 0;

// If enable this will feed test data into the usecases to simulate certain conditions.
// Additionally it demonstrate how to access the usecases for implementing them into the rest of the systems
void run_eebus_usecase_tests()
{
    switch (dev_test_iteration) {
        case 0:
#ifdef EEBUS_ENABLE_EVSECC_USECASE
            logger.printfln("EEBUS Usecase test: Updating EvseccUsecase with a test error");
            eebus.usecases->evse_commissioning_and_configuration.update_operating_state(true, "This is a test error message. It should be displayed. The error state will be resolved in 30 seconds.");
#endif
            break;
        case 1:
#ifdef EEBUS_ENABLE_EVCS_USECASE
            logger.printfln("EEBUS Usecase test: Updating ChargingSummary");
            eebus.usecases->charging_summary.update_billing_data(1, 299921, 3242662, 245233, 1242, 75, 90, 25, 10);
            eebus.usecases->charging_summary.update_billing_data(2, 5622123, 5655611, 23677, 1242, 50, 100, 50, 0);
#endif
            break;
        case 2:
#ifdef EEBUS_ENABLE_EVCC_USECASE
            logger.printfln("EEBUS Usecase test: Updating EvccUsecase");
            eebus.usecases->ev_commissioning_and_configuration.ev_connected_state(true);
            eebus.usecases->ev_commissioning_and_configuration.update_device_config("iso15118-2ed1", true);
            eebus.usecases->ev_commissioning_and_configuration.update_identification("12:34:56:78:9a:bc");
            eebus.usecases->ev_commissioning_and_configuration.update_manufacturer("VW", "0", "00001", "1.0", "0.1", "Volkswagen", "1", "Skoda", "VW", "");
            eebus.usecases->ev_commissioning_and_configuration.update_electrical_connection(100, EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION, 800);
            eebus.usecases->ev_commissioning_and_configuration.update_operating_state(false);
#endif
            break;
        case 3:
#ifdef EEBUS_ENABLE_EVSECC_USECASE
            logger.printfln("EEBUS Usecase test: Updating EvseccUsecase to normal operation");
            eebus.usecases->evse_commissioning_and_configuration.update_operating_state(false, "This is a test error message. It should not be shown");
#endif
            break;
        case 4:
#ifdef EEBUS_ENABLE_LPC_USECASE
            /*logger.printfln("EEBUS Usecase test: Updating Limitation of Power Consumption Usecase to set a demo limit");
            eebus.usecases->limitation_of_power_consumption.update_lpc(true, 3000, 1_h);
            eebus.usecases->limitation_of_power_consumption.update_failsafe(1000, 3_h);
            eebus.usecases->limitation_of_power_consumption.update_constraints(22000);*/
#endif
            break;
        case 5:
#ifdef EEBUS_ENABLE_MPC_USECASE
            logger.printfln("EEBUS Usecase test: Updating MPC with test measurements");
            // Update constraints for all measurement types
            eebus.usecases->mpc->update_constraints(0, 22000, 100, 0, 16000, 10, 0, 1000000, 10, 0, 400, 1, 10, 100, 1);
            // Update power measurements (total + 3 phases) - includes negative value for phase 2 to test feed-in
            eebus.usecases->mpc->update_power(6500, 2200, -150, 4450);
            // Update voltage measurements (3 phase-to-neutral + 3 phase-to-phase)
            eebus.usecases->mpc->update_voltage(230, 231, 229, 400, 401, 398);
            // Update current measurements (3 phases in mA)
            eebus.usecases->mpc->update_current(9570, 6520, 19350);
            // Update frequency (50Hz = 50000 mHz)
            eebus.usecases->mpc->update_frequency(51000);
            // Initial energy values (will be updated in subsequent iterations)
            eebus.usecases->mpc->update_energy(150000, 25000);
#endif
            break;
        case 6:
        default:
#ifdef EEBUS_ENABLE_EVCEM_USECASE
            logger.printfln("EEBUS Usecase test enabled. Update Power consumed");
            eebus.usecases->ev_charging_electricity_measurement.update_measurements(1234, 5678, 9000, 1000, 2000, 3000, dev_test_iteration * 10);
#endif
#ifdef EEBUS_ENABLE_MPC_USECASE
            // Update MPC energy values in subsequent iterations (increasing over time)
            eebus.usecases->mpc->update_energy(150000 + (dev_test_iteration * 100), 25000 + (dev_test_iteration * 50));
#endif
            break;
    }
    dev_test_iteration++;
}
#endif

void EEBus::pre_setup()
{
    // Use PSRAM 128kB for trace buffer for now. We can reduce it if necessary.
    this->trace_buffer_index = logger.alloc_trace_buffer("eebus", 1 << 17);

    // TOOD: Rework API so this lot is a bit cleaner
    // Config peers prototype - only persistent peers stored in flash
    // Note: state is NOT stored in config, only in state API
    config_peers_prototype = Config::Object({
        {"ip", Config::Str("0.0.0.0", 7, 150)}, // Store a maximum of 3 ipv6 addresses
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
        {"model_type", Config::Str("", 0, 32)},
    });

    // State peers prototype - all peers (both persistent and discovered) in state API
    state_peers_prototype = Config::Object({
        {"ip", Config::Str("0.0.0.0", 7, 150)},
        {"port", Config::Uint16(0)},
        {"trusted", Config::Bool(false)},
        {"dns_name", Config::Str("", 0, 63)},
        {"id", Config::Str("", 0, 63)},
        {"wss_path", Config::Str("", 0, 32)},
        {"ski", Config::Str("", 0, 40)},
        {"autoregister", Config::Bool(false)},
        {"model_brand", Config::Str("", 0, 32)},
        {"model_model", Config::Str("", 0, 32)},
        {"model_type", Config::Str("", 0, 32)},
        {"state", Config::Enum(NodeState::Disconnected)},
        {"persistent", Config::Bool(false)}, // Indicates if peer is stored in persistent config
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
    add_peer = ConfigRoot{Config::Object({{"ip", Config::Str("0.0.0.0", 7, 150)}, {"port", Config::Uint16(0)}, {"trusted", Config::Bool(false)}, {"dns_name", Config::Str("", 0, 63)}, {"wss_path", Config::Str("", 0, 32)}, {"ski", Config::Str("", 0, 40)}, {"persistent", Config::Bool(true)}}), [this](Config &add_peer, ConfigSource source) -> String {
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
        {"peers", Config::Array({state_peers_prototype}, &state_peers_prototype, 0, 32, Config::type_id<Config::ConfObject>())}, // All peers (persistent + discovered)
    });

    // A list of all charges, ideally with their cost and which percentage of it was self produced energy
    charges_prototype = Config::Object({{"id", Config::Uint16(0)}, {"charged_kwh", Config::Float(0)}, {"start_time", Config::Uint32(0)}, {"duration", Config::Uint16(0)}, {"cost", Config::Float(0)}, {"percent_self_produced_energy", Config::Uint16(0)}, {"percent_self_produced_cost", Config::Uint16(0)}});
    usecase_list = Config::Enum(Usecases::NMC);
    // Currently eebus state and eebus config are one config. Maybe split them?
    eebus_usecase_state = Config::Object({
        {"commands_received", Config::Uint16(0)},
        {"commands_sent", Config::Uint16(0)},
        //      {"usecases_supported", Config::Str("", 0, 128)}, // Comma separated list of supported usecases
        {"usecases_supported", Config::Array({usecase_list}, &usecase_list, 0, 12, Config::type_id<Config::ConfObject>())},
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
             {"outstanding_duration_s", Config::Uint32(0)},    // If a limit due to limited state or failsafe is active, how long is left
         })},
        {"power_production_limitation",
         Config::Object({
             // Usecase Limitation of power production
             {"usecase_state", Config::Enum(LPPState::Init)},
             {"limit_active", Config::Bool(false)},
             {"current_limit", Config::Uint16(0)},
             {"failsafe_limit_power_w", Config::Uint16(0)},    // The limit which may have been set by the energy guard
             {"failsafe_limit_duration_s", Config::Uint32(0)}, // If a failsafe state is entered, how long until this limit is applied before it goes back to default
             {"constraints_power_maximum", Config::Uint16(0)}, // The maximum power production the device is capable of
             {"outstanding_duration_s", Config::Uint32(0)},    // If a limit due to limited state or failsafe is active, how long is left
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
        {"evse_commissioning_and_configuration",
         Config::Object({
             {"evse_failure", Config::Bool(false)},
             {"evse_failure_description", Config::Str("", 0, 64)},
         })},
        {"ev_charging_electricity_measurement",
         Config::Object({
             {"amps_phase_1", Config::Uint16(0)},
             {"amps_phase_2", Config::Uint16(0)},
             {"amps_phase_3", Config::Uint16(0)},
             {"power_phase_1", Config::Uint16(0)},
             {"power_phase_2", Config::Uint16(0)},
             {"power_phase_3", Config::Uint16(0)},
             {"charged_wh", Config::Uint32(0)},
             {"charged_valuesource_measured", Config::Bool(false)},
         })},
        {"monitoring_of_power_consumption",
         Config::Object({
             // Usecase Monitoring of power consumption
             {"total_power_w", Config::Uint32(0)},
             {"power_phase_1_w", Config::Uint32(0)},
             {"power_phase_2_w", Config::Uint32(0)},
             {"power_phase_3_w", Config::Uint32(0)},
             {"energy_consumed_wh", Config::Uint32(0)},
             {"energy_produced_wh", Config::Uint32(0)},
             {"current_phase_1_ma", Config::Uint32(0)},
             {"current_phase_2_ma", Config::Uint32(0)},
             {"current_phase_3_ma", Config::Uint32(0)},
             {"voltage_phase_1_v", Config::Uint32(0)},
             {"voltage_phase_2_v", Config::Uint32(0)},
             {"voltage_phase_3_v", Config::Uint32(0)},
             {"voltage_phase_1_2_v", Config::Uint32(0)},
             {"voltage_phase_2_3_v", Config::Uint32(0)},
             {"voltage_phase_3_1_v", Config::Uint32(0)},
             {"frequency_mhz", Config::Uint32(50000)}, // Default 50Hz in millihertz
         })},
        {"coordinated_ev_charging",
         Config::Object({
             {"has_charging_plan", Config::Bool(false)},
             {"charging_plan_start_time", Config::Uint32(0)},
             {"target_power_w", Config::Int32(-1)},
             {"has_incentives", Config::Bool(false)},
             {"energy_broker_connected", Config::Bool(false)},
             {"energy_broker_heartbeat_ok", Config::Bool(false)},
         })},
    });

    ship.pre_setup();
}

void EEBus::setup()
{
    api.restorePersistentConfig("eebus/config", &config);

    ship.setup();
    update_peers_config();

    initialized = true;
    eebus_name = device_name.name.get("name")->asEphemeralCStr();
    eebus.trace_fmtln("EEBUS initialized");

#ifdef EEBUS_DEV_TEST_ENABLE
    task_scheduler.scheduleUncancelable(
        []() {
            if (eebus.is_enabled()) {
                run_eebus_usecase_tests();
            }
        },
        20_s,
        10_s);
    logger.printfln("EEBUS Usecase testing is enabled. This updates the eebus system with test data to simulate certain conditions.");
#endif
    task_scheduler.scheduleUncancelable(
        []() {
            update_evse_limit();
        },
        10_s,
        1_s);
}

void EEBus::register_urls()
{
    api.addPersistentConfig("eebus/config", &config);
    api.addState("eebus/state", &state, {}, {});

    api.addState("eebus/usecases", &eebus_usecase_state, {}, {}, false);

    api.addCommand(
        "eebus/add",
        &add_peer,
        {},
        [this](Language /*language*/, String &errmsg) {
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

            auto existing_peer = ship.peer_handler.get_peer_by_ski(ski);
            if (existing_peer == nullptr) {
                ship.peer_handler.new_peer_from_ski(ski);
                existing_peer = ship.peer_handler.get_peer_by_ski(ski);
            }

            if (existing_peer != nullptr) {
                existing_peer->persistent = add_peer.get("persistent")->asBool();

                ship.peer_handler.update_ip_by_ski(ski, add_peer.get("ip")->asString());
                ship.peer_handler.update_port_by_ski(ski, add_peer.get("port")->asUint());
                ship.peer_handler.update_trusted_by_ski(ski, add_peer.get("trusted")->asBool());
                ship.peer_handler.update_dns_name_by_ski(ski, add_peer.get("dns_name")->asString());
                ship.peer_handler.update_wss_path_by_ski(ski, add_peer.get("wss_path")->asString());

                sync_persistent_peer_to_config(existing_peer);

                update_peers_state();

                ship.connect_trusted_peers();
            }
        },
        true);

    api.addCommand(
        "eebus/remove",
        &remove_peer,
        {"ski"},
        [this](Language /*language*/, String &errmsg) {
            if (!config.get("enable")->asBool()) {
                eebus.trace_fmtln("Tried removing peer while EEBUS is disabled");
                return;
            }

            if (!errmsg.isEmpty()) {
                eebus.trace_fmtln("Error removing peer: %s", errmsg.c_str());
                return;
            }

            String ski = remove_peer.get("ski")->asString();
            auto peer = ship.peer_handler.get_peer_by_ski(ski);

            if (peer == nullptr) {
                eebus.trace_fmtln("Cannot remove peer: Peer with SKI %s not found", ski.c_str());
                return;
            }

            // Check if peer was persistent before removing
            bool was_persistent = peer->persistent;

            ship.peer_handler.remove_peer_by_ski(ski);

            // Only update config if the peer was persistent
            if (was_persistent) {
                update_peers_config(); // Remove from persistent storage
            }
            update_peers_state(); // Update state to reflect removal
        },
        true);

    api.addCommand(
        "eebus/scan",
        &scan_command,
        {},
        [this](Language /*language*/, String &errmsg) {
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
                    0_s);

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

static constexpr MeterValueID mvids[] = {
    MeterValueID::CurrentL1ImExDiff,
    MeterValueID::CurrentL2ImExDiff,
    MeterValueID::CurrentL3ImExDiff,
    MeterValueID::PowerActiveL1ImExDiff,
    MeterValueID::PowerActiveL2ImExDiff,
    MeterValueID::PowerActiveL3ImExDiff,
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::VoltageL1N,
    MeterValueID::VoltageL2N,
    MeterValueID::VoltageL3N,
    MeterValueID::VoltageL1L2,
    MeterValueID::VoltageL2L3,
    MeterValueID::VoltageL3L1,
    MeterValueID::FrequencyLAvg,
};

template <typename T> struct MeterValues {
    T currents[3];
    T powers[3];
    T total_power;
    T energy_import;
    T energy_export;
    T voltages[3];
    T phase_voltages[3];
    T frequency;
};
static_assert(ARRAY_SIZE(mvids) == (sizeof(MeterValues<int>) / sizeof(int)));

void EEBus::register_events()
{
    // These are all the Events related to the device being an EVSE
#ifdef EEBUS_MODE_EVSE
    event.registerEvent("evse/phases_connected", {}, [](const Config *phases_cfg) {
        update_usecases_from_phases(phases_cfg);
        return EventResult::OK;
    });

    event.registerEvent("evse/state", {"charger_state"}, [](const Config *iec_state_cfg) {
        update_usecases_from_charger_state(iec_state_cfg);
        return EventResult::OK;
    });

    auto meter_slot = evse_common.get_charger_meter();

    event.registerEvent(meters.get_path(meter_slot, Meters::PathType::ValueIDs), {}, [meter_slot](const Config *value_ids) {
        // TODO: Is this necessary? Stolen from meters_legacy_api.cpp
        if (value_ids->count() == 0) {
            logger.printfln("Ignoring blank value IDs update from charger meter.");
            return EventResult::OK;
        }

        constexpr size_t VALUE_COUNT = ARRAY_SIZE(mvids);

        uint32_t *index_cache = perm_new_array<uint32_t>(VALUE_COUNT, DRAM);

        meters.fill_index_cache(meter_slot, VALUE_COUNT, mvids, index_cache);

        event.registerEvent(meters.get_path(meter_slot, Meters::PathType::Values), {}, [meter_slot, index_cache](const Config *_) {
            if (eebus.usecases == nullptr)
                return EventResult::OK;

            MeterValues<int> v;
            int charged_wh;
            {
                MeterValues<float> floats;
                meters.get_values_with_cache(meter_slot, reinterpret_cast<float *>(&floats), index_cache, VALUE_COUNT);

                for (int i = 0; i < 3; ++i) {
                    v.currents[i] = sanitized(floats.currents[i], 1000.0f);
                    v.powers[i] = sanitized(floats.powers[i], 1);
                    v.voltages[i] = sanitized(floats.voltages[i], 1);
                    v.phase_voltages[i] = sanitized(floats.phase_voltages[i], 1);
                }
                v.energy_import = sanitized(floats.energy_import, 1000.0f);
                v.energy_export = sanitized(floats.energy_export, 1000.0f);
                v.frequency = sanitized(floats.frequency, 1000.0f);
                v.total_power = sanitized(floats.total_power, 1);

                bool charging = charge_tracker.current_charge.get("user_id")->asInt16() != -1;
                float meter_start = charge_tracker.current_charge.get("meter_start")->asFloat();

                // NaN propagates, so this checks both energy_import and meter_start.
                charged_wh = !charging ? INT32_MIN : sanitized(floats.energy_import - meter_start, 1000.0f);
            }

#ifdef EEBUS_ENABLE_EVCEM_USECASE
            eebus.usecases->evcem->update_measurements(v.currents[0], v.currents[1], v.currents[2], v.powers[0], v.powers[1], v.powers[2], charged_wh);
#endif
#ifdef EEBUS_ENABLE_MPC_USECASE
            eebus.usecases->mpc->update_power(v.total_power, v.powers[0], v.powers[1], v.powers[2]);
            eebus.usecases->mpc->update_energy(v.energy_import, v.energy_export);
            eebus.usecases->mpc->update_current(v.currents[0], v.currents[1], v.currents[2]);
            eebus.usecases->mpc->update_voltage(v.voltages[0], v.voltages[1], v.voltages[2], v.phase_voltages[0], v.phase_voltages[1], v.phase_voltages[2]);
            eebus.usecases->mpc->update_frequency(v.frequency);
#endif
            charged_wh++; // This is only here to shut up the compiler as it thinks charged_wh is unused if evcem is disabled
            return EventResult::OK;
        });

        // Value IDs should only change once
        return EventResult::Deregister;
    });
#endif
}

void EEBus::toggle_module()
{
    if (config.get("enable")->asBool()) {
        module_enabled = true;
        usecases = make_unique_psram<EEBusUseCases>();
#ifdef EEBUS_IN_EVSE_MODE
        update_usecases_from_charger_state((const Config *)api.getState("evse/state")->get("charger_state"));
        update_usecases_from_phases((const Config *)api.getState("evse/phases_connected")->get("phases"));
#endif
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
    return eebus_name;
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

void EEBus::update_peers_state()
{
    // Update state API with all peers (both persistent and discovered)
    state.get("peers")->removeAll();
    auto peers = ship.peer_handler.get_peers();
    for (const std::shared_ptr<ShipNode> &node : peers) {
        auto peer = state.get("peers")->add();
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
        peer->get("model_type")->updateString(node->txt_type);
        peer->get("state")->updateEnum(node->state);
        peer->get("persistent")->updateBool(node->persistent);
    }
}

void EEBus::update_peers_config()
{
    // Only save persistent peers to config (to reduce EEPROM wear)
    config.get("peers")->removeAll();
    auto peers = ship.peer_handler.get_peers();
    for (const std::shared_ptr<ShipNode> &node : peers) {
        if (!node->persistent) {
            continue; // Skip non-persistent peers
        }
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
        peer->get("model_type")->updateString(node->txt_type);
    }
}

void EEBus::sync_persistent_peer_to_config(const std::shared_ptr<ShipNode> &node)
{
    // Sync a single persistent peer to config (for incremental updates)
    if (!node->persistent) {
        return; // Only sync persistent peers
    }

    // Find if peer already exists in config
    bool found = false;
    size_t peer_count = config.get("peers")->count();
    for (size_t i = 0; i < peer_count; i++) {
        auto config_peer = config.get("peers")->get(i);
        if (config_peer->get("ski")->asString() == node->txt_ski) {
            // Update existing peer
            found = true;
            bool needs_update = false;

            // Check if any field needs updating
            if (config_peer->get("ip")->asString() != node->ip_address_as_string())
                needs_update = true;
            if (config_peer->get("port")->asUint() != node->port)
                needs_update = true;
            if (config_peer->get("trusted")->asBool() != node->trusted)
                needs_update = true;
            if (config_peer->get("dns_name")->asString() != node->dns_name)
                needs_update = true;
            if (config_peer->get("id")->asString() != node->txt_id)
                needs_update = true;
            if (config_peer->get("wss_path")->asString() != node->txt_wss_path)
                needs_update = true;
            if (config_peer->get("autoregister")->asBool() != node->txt_autoregister)
                needs_update = true;
            if (config_peer->get("model_brand")->asString() != node->txt_brand)
                needs_update = true;
            if (config_peer->get("model_model")->asString() != node->txt_model)
                needs_update = true;
            if (config_peer->get("model_type")->asString() != node->txt_type)
                needs_update = true;

            if (needs_update) {
                config_peer->get("ip")->updateString(node->ip_address_as_string());
                config_peer->get("port")->updateUint(node->port);
                config_peer->get("trusted")->updateBool(node->trusted);
                config_peer->get("dns_name")->updateString(node->dns_name);
                config_peer->get("id")->updateString(node->txt_id);
                config_peer->get("wss_path")->updateString(node->txt_wss_path);
                config_peer->get("autoregister")->updateBool(node->txt_autoregister);
                config_peer->get("model_brand")->updateString(node->txt_brand);
                config_peer->get("model_model")->updateString(node->txt_model);
                config_peer->get("model_type")->updateString(node->txt_type);
                api.writeConfig("eebus/config", &config);
            }
            break;
        }
    }

    if (!found) {
        // Add new peer to config
        if (config.get("peers")->count() < MAX_PEER_REMEMBERED) {
            auto new_peer = config.get("peers")->add();
            new_peer->get("ip")->updateString(node->ip_address_as_string());
            new_peer->get("port")->updateUint(node->port);
            new_peer->get("trusted")->updateBool(node->trusted);
            new_peer->get("dns_name")->updateString(node->dns_name);
            new_peer->get("id")->updateString(node->txt_id);
            new_peer->get("wss_path")->updateString(node->txt_wss_path);
            new_peer->get("ski")->updateString(node->txt_ski);
            new_peer->get("autoregister")->updateBool(node->txt_autoregister);
            new_peer->get("model_brand")->updateString(node->txt_brand);
            new_peer->get("model_model")->updateString(node->txt_model);
            new_peer->get("model_type")->updateString(node->txt_type);
            api.writeConfig("eebus/config", &config);
        } else {
            logger.printfln("EEBUS: Cannot add persistent peer %s to config - max peers (%d) reached", node->txt_ski.c_str(), MAX_PEER_REMEMBERED);
        }
    }
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
