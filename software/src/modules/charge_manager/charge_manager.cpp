/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "charge_manager.h"
#include "module_dependencies.h"

#include <algorithm>
#include <Arduino.h>
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"
#include "modules/cm_networking/cm_networking_defs.h"

#include "api.h"
#include "build.h"
#include "task_scheduler.h"
#include "tools.h"

#define DISTRIBUTION_LOG_LEN 2048

#define CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE 128
#define CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE 129
#define CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE 130
#define CHARGE_MANAGER_CLIENT_ERROR_START 192

// This is a hack to allow the validator of available_current
// to access config["maximum_available_current"]
// It is necessary, because configs only take a function pointer as
// validator function, so lambda capture lists have to be empty.
static uint32_t max_avail_current = 0;

#define TIMEOUT_MS 32000

#define WATCHDOG_TIMEOUT_MS 30000

// If this is an energy manager, we have exactly one charger and the margin is still the default,
// double it to react faster if more current is available.
#define REQUESTED_CURRENT_MARGIN_DEFAULT 3000

extern bool firmware_update_allowed;
extern ChargeManager charge_manager;

#if MODULE_ENERGY_MANAGER_AVAILABLE()
#define REQUESTED_CURRENT_MARGIN_ENERGY_MANAGER_1_CHARGER_DEFAULT (2 * REQUESTED_CURRENT_MARGIN_DEFAULT)

static void apply_energy_manager_config(Config &conf)
{
    conf.get("enable_charge_manager")->updateBool(true);
    conf.get("enable_watchdog")->updateBool(false);
    conf.get("default_available_current")->updateUint(0);


    if (conf.get("chargers")->count() == 1 && conf.get("requested_current_margin")->asUint() == REQUESTED_CURRENT_MARGIN_DEFAULT) {
        conf.get("requested_current_margin")->updateUint(REQUESTED_CURRENT_MARGIN_ENERGY_MANAGER_1_CHARGER_DEFAULT);
    }

    if (conf.get("chargers")->count() != 1 && conf.get("requested_current_margin")->asUint() == REQUESTED_CURRENT_MARGIN_ENERGY_MANAGER_1_CHARGER_DEFAULT) {
        conf.get("requested_current_margin")->updateUint(REQUESTED_CURRENT_MARGIN_DEFAULT);
    }
}
#endif

#if MODULE_AUTOMATION_AVAILABLE()
bool ChargeManager::action_triggered(Config *config, void *data) {
    switch(config->getTag<AutomationTriggerID>()) {
        case AutomationTriggerID::ChargeManagerWd:
            return true;

        default:
            return false;
    }
}
#endif

void ChargeManager::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable_charge_manager", Config::Bool(false)},
        {"enable_watchdog", Config::Bool(false)},
        {"default_available_current", Config::Uint32(0)},
        {"maximum_available_current", Config::Uint32(0xFFFFFFFF)},
        {"minimum_current_auto", Config::Bool(true)},
        {"minimum_current", Config::Uint(6000, 6000, 32000)},
        {"minimum_current_1p", Config::Uint(6000, 6000, 32000)},
        {"minimum_current_vehicle_type", Config::Uint32(0)},
        {"verbose", Config::Bool(false)},
        // Line currents are used to calculate the "requested current"
        // if a charger is in state C for at least
        // "requested_current_threshold" ms.
        // Otherwise the "requested current" is the "supported current"
        // to allow the car to ramp up fast.
        {"requested_current_threshold", Config::Uint16(60)},
        // Added to requested current before distributing current.
        // This margin has to be large enough to make sure
        // every vehicle can always request more current.
        // See https://github.com/Tinkerforge/warp-charger/blob/master/tools/current_ramp/allowed_vs_effective_3.gp
        {"requested_current_margin", Config::Uint16(REQUESTED_CURRENT_MARGIN_DEFAULT)},
        {"chargers", Config::Array({},
            new Config{Config::Object({
                {"host", Config::Str("", 0, 64)},
                {"name", Config::Str("", 0, 32)}
            })},
            0, MAX_CONTROLLED_CHARGERS, Config::type_id<Config::ConfObject>()
        )}
    }), [](Config &conf, ConfigSource source) -> String {
#if MODULE_ENERGY_MANAGER_AVAILABLE()
        apply_energy_manager_config(conf);
#else
        uint32_t default_available_current = conf.get("default_available_current")->asUint();
        uint32_t maximum_available_current = conf.get("maximum_available_current")->asUint();

        if (maximum_available_current == 0xFFFFFFFF) {
            conf.get("maximum_available_current")->updateUint(default_available_current);
        }

        if (default_available_current > maximum_available_current)
            return "default_available_current can not be greater than maximum_available_current";
#endif

        if (conf.get("minimum_current_auto")->asBool()) {
            auto minimum_current_vehicle_type = conf.get("minimum_current_vehicle_type")->asUint();
            uint32_t min_1p;
            uint32_t min_3p;

            if (minimum_current_vehicle_type > 0) {
                min_1p = 6000;
                min_3p = 9200;
            } else {
                min_1p = 6000;
                min_3p = 6000;
            }

            conf.get("minimum_current_1p")->updateUint(min_1p);
            conf.get("minimum_current")->updateUint(min_3p);
        }

        auto chargers = conf.get("chargers");

        for (size_t i = 0; i < chargers->count(); i++)
            for (size_t a = i + 1; a < chargers->count(); a++)
                if (chargers->get(i)->get("host")->asString() == chargers->get(a)->get("host")->asString())
                    return "there must not be two chargers with the same hostname or ip-address";

        return "";
    }};

    state = Config::Object({
        {"state", Config::Uint8(0)}, // 0 - not configured, 1 - active, 2 - shutdown
        {"chargers", Config::Array(
            {},
            new Config{Config::Object({
                {"state", Config::Uint8(0)}, // 0 - no vehicle, 1 - user blocked, 2 - manager blocked, 3 - car blocked, 4 - charging, 5 - error, 6 - charged
                {"error", Config::Uint8(0)}, // 0 - okay, 1 - unreachable, 2 - FW mismatch, 3 - not managed
                {"allocated_current", Config::Uint16(0)}, // last current limit send to the charger
                {"supported_current", Config::Uint16(0)}, // maximum current supported by the charger
                {"last_update", Config::Uint32(0)},
                {"name", Config::Str("", 0, 32)},
                {"uid", Config::Uint32(0)}
            })},
            0, MAX_CONTROLLED_CHARGERS, Config::type_id<Config::ConfObject>()
        )}
    });

    available_current = ConfigRoot{Config::Object({
        {"current", Config::Uint32(0)},
    }), [](const Config &conf, ConfigSource source) -> String {
        if (conf.get("current")->asUint() > max_avail_current)
            return "Current too large: maximum available current is configured to " + String(max_avail_current);
        return "";
    }};
    available_current_update = available_current;

    available_phases = ConfigRoot{Config::Object({
        {"phases", Config::Uint(3, 1, 3)},
    }), [](const Config &conf, ConfigSource source) -> String {
        if (conf.get("phases")->asUint() == 2)
            return "Two phases not supported.";
        return "";
    }};
    available_phases_update = available_phases;

    control_pilot_disconnect = Config::Object({
        {"disconnect", Config::Bool(false)},
    });

#if MODULE_AUTOMATION_AVAILABLE() && !MODULE_ENERGY_MANAGER_AVAILABLE()
    automation.register_trigger(
        AutomationTriggerID::ChargeManagerWd,
        *Config::Null()
    );

    automation.register_action(
        AutomationActionID::SetManagerCurrent,
        Config::Object({
            {"current", Config::Uint(0)}
        }),
        [this](const Config *config) {
            this->available_current.get("current")->updateUint(config->get("current")->asUint());
            this->last_available_current_update = millis();
        },
        [](const Config *config) -> String {
            if (config->get("current")->asUint() > max_avail_current)
                return "Current too large: maximum available current is configured to " + String(max_avail_current);
            return "";
        });
#endif
}

static uint8_t get_charge_state(uint8_t charger_state, uint16_t supported_current, uint32_t charging_time, uint16_t target_allocated_current)
{
    if (charger_state == 0) // not connected
        return 0;
    if (charger_state == 3) // charging
        return 4;
    if (charger_state == 4) // error
        return 5;
    if (charger_state == 1 && supported_current == 0) // connected but blocked, supported current == 0 means another slot blocks
        return 1;
    if (charger_state == 1 && supported_current != 0) { // blocked by charge management (as supported current != 0)
        if (charging_time == 0)
            return 2; // Not charged this session
        else
            return 6; // Charged at least once
    }
    if (charger_state == 2)
        return 3; // Waiting for the car to start charging

    logger.printfln("Unknown state! cs %u sc %u ct %u tac %u", charger_state, supported_current, charging_time, target_allocated_current);
    return 5;
}

void ChargeManager::start_manager_task()
{
    auto charger_count = config.get("chargers")->count();

    cm_networking.register_manager(this->hosts.get(), config.get("chargers")->count(), [this](uint8_t client_id, cm_state_v1 *v1, cm_state_v2 *v2) mutable {
            // TODO: bounds check
            ChargerState &target = this->charger_state[client_id];

            // Don't update if the uptimes are the same.
            // This means, that the EVSE hangs or the communication
            // is not working. As last_update will now hang too,
            // the management will stop all charging after some time.
            if (target.uptime == v1->evse_uptime) {
                logger.printfln("Received stale charger state from %s (%s). Reported EVSE uptime (%u) is the same as in the last state. Is the EVSE still reachable?",
                    this->get_charger_name(client_id), this->hosts[client_id],
                    v1->evse_uptime);
                if (deadline_elapsed(target.last_update + 10000)) {
                    target.state = 5;
                    target.error = CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE;
                }

                return;
            }

            target.uid = v1->esp32_uid;
            target.uptime = v1->evse_uptime;

#if MODULE_ENERGY_MANAGER_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
            // Immediately block firmware updates if this charger reports a connected vehicle.
            if (v1->charger_state != 0)
                firmware_update_allowed = false;
#endif

            // A charger wants to charge if:
            // the charging time is 0 (it has not charged this vehicle yet), no other slot blocks
            //     AND we are still in charger state 1 (i.e. blocked by a slot, so the charge management slot)
            //         or 2 (i.e. already have current allocated)
            // OR the charger is already charging
            bool wants_to_charge = (v1->charging_time == 0 && v1->supported_current != 0 && (v1->charger_state == 1 || v1->charger_state == 2)) || v1->charger_state == 3;
            target.wants_to_charge = wants_to_charge;

            // A charger wants to charge and has low priority if it has already charged this vehicle
            // AND only the charge manager slot (charger_state == 1, supported_current != 0) or no slot (charger_state == 2) blocks.
            bool low_prio = v1->charging_time != 0 && v1->supported_current != 0 && (v1->charger_state == 1 || v1->charger_state == 2);
            target.wants_to_charge_low_priority = low_prio;

            target.is_charging = v1->charger_state == 3;
            target.allowed_current = v1->allowed_charging_current;
            target.supported_current = v1->supported_current;
            target.cp_disconnect_supported = CM_FEATURE_FLAGS_CP_DISCONNECT_IS_SET(v1->feature_flags);
            target.cp_disconnect_state = CM_STATE_FLAGS_CP_DISCONNECTED_IS_SET(v1->state_flags);
            target.last_update = millis();
            target.charger_state = v1->charger_state;

            uint16_t requested_current = v1->supported_current;

            if (v2 != nullptr && v1->charger_state == 3 && v2->time_since_state_change >= this->requested_current_threshold * 1000) {
                int32_t max_phase_current = -1;

                for (int i = 0; i < 3; i++) {
                    if (isnan(v1->line_currents[i])) {
                        // Don't trust the line currents if one is missing.
                        max_phase_current = 32000;
                        break;
                    }

                    max_phase_current = max(max_phase_current, (int32_t)(v1->line_currents[i] * 1000.0f));
                }
                // The CM protocol sends 0 instead of nan.
                if (max_phase_current == 0)
                    max_phase_current = 32000;

                max_phase_current += this->requested_current_margin;

                max_phase_current = max(6000, min(32000, max_phase_current));
                requested_current = min(requested_current, (uint16_t)max_phase_current);
            }
            target.requested_current = requested_current;

            target.meter_supported = CM_FEATURE_FLAGS_METER_IS_SET(v1->feature_flags);
            if (!isnan(v1->power_total)) {
                target.power_total_sum = target.power_total_sum + v1->power_total;
                target.power_total_count = target.power_total_count + 1;
            }
            if (!isnan(v1->energy_abs)) {
                target.energy_abs = v1->energy_abs;
            }

            if (v1->error_state != 0) {
                target.error = CHARGE_MANAGER_CLIENT_ERROR_START + static_cast<uint32_t>(v1->error_state);
            }

            auto current_error = target.error;
            if (current_error < 128 || current_error == CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE) {
                target.error = 0;
            }

            current_error = target.error;
            if (current_error == 0 || current_error >= CHARGE_MANAGER_CLIENT_ERROR_START)
                target.state = get_charge_state(v1->charger_state,
                                                                  v1->supported_current,
                                                                  v1->charging_time,
                                                                  target.allocated_current);

            auto *charger_cfg = (Config *) this->state.get("chargers")->get(client_id);
            charger_cfg->get("state")->updateUint(target.state);
            charger_cfg->get("error")->updateUint(target.error);
            charger_cfg->get("allocated_current")->updateUint(target.allocated_current);
            charger_cfg->get("supported_current")->updateUint(target.supported_current);
            charger_cfg->get("last_update")->updateUint(target.last_update);
            charger_cfg->get("uid")->updateUint(target.uid);
    }, [this](uint8_t client_id, uint8_t error){
        //TODO bounds check
        ChargerState &target = this->charger_state[client_id];
        target.state = 5;
        target.error = error;
    });

    uint32_t cm_send_delay = 1000 / charger_count;

    task_scheduler.scheduleWithFixedDelay([this, charger_count]() mutable {
        static int i = 0;

        if (i >= charger_count)
            i = 0;

        auto &charger = this->charger_state[i];
        if(cm_networking.send_manager_update(i, charger.allocated_current, charger.cp_disconnect))
            ++i;

    }, 0, cm_send_delay);
}

int idx_array[MAX_CONTROLLED_CHARGERS] = {0};

void ChargeManager::setup()
{
    uint32_t control_cycle_time_ms;
    if (!api.restorePersistentConfig("charge_manager/config", &config)) {
        config.get("maximum_available_current")->updateUint(0);
#if MODULE_ENERGY_MANAGER_AVAILABLE()
        apply_energy_manager_config(config);
        control_cycle_time_ms = 5 * 1000;
#else
        control_cycle_time_ms = 10 * 1000;
#endif
    }

    default_available_current = config.get("default_available_current")->asUint();
    minimum_current = config.get("minimum_current")->asUint();
    minimum_current_1p = config.get("minimum_current_1p")->asUint();
    requested_current_threshold = config.get("requested_current_threshold")->asUint();
    requested_current_margin = config.get("requested_current_margin")->asUint();

    max_avail_current = config.get("maximum_available_current")->asUint();

    if(!config.get("enable_charge_manager")->asBool() || config.get("chargers")->count() == 0) {
        initialized = true;
        return;
    }
    state.get("state")->updateUint(1);

    available_current.get("current")->updateUint(config.get("default_available_current")->asUint());
    for (int i = 0; i < config.get("chargers")->count(); ++i) {
        state.get("chargers")->add();
        state.get("chargers")->get(i)->get("name")->updateString(config.get("chargers")->get(i)->get("name")->asString());
        idx_array[i] = i;
    }

    for (int i = config.get("chargers")->count(); i < MAX_CONTROLLED_CHARGERS; ++i)
        idx_array[i] = -1;

    size_t hosts_buf_size = 0;
    for (int i = 0; i < config.get("chargers")->count(); ++i) {
        hosts_buf_size += config.get("chargers")->get(i)->get("host")->asString().length() + 1; //null terminator
    }

    char *hosts_buf = (char*)heap_caps_calloc_prefer(hosts_buf_size, sizeof(char), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    this->hosts = heap_alloc_array<const char *>(config.get("chargers")->count());
    size_t hosts_written = 0;

    for (int i = 0; i < config.get("chargers")->count(); ++i) {
        hosts[i] = hosts_buf + hosts_written;
        memcpy(hosts_buf + hosts_written, config.get("chargers")->get(i)->get("host")->asEphemeralCStr(), config.get("chargers")->get(i)->get("host")->asString().length());
        hosts_written += config.get("chargers")->get(i)->get("host")->asString().length();
        hosts_buf[hosts_written] = '\0';
        ++hosts_written;
    }

    this->charger_count = config.get("chargers")->count();
    this->charger_state = (ChargerState*) heap_caps_calloc_prefer(this->charger_count, sizeof(ChargerState), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

    start_manager_task();

    task_scheduler.scheduleWithFixedDelay([this](){this->distribute_current();}, control_cycle_time_ms, control_cycle_time_ms);

    if (config.get("enable_watchdog")->asBool()) {
        task_scheduler.scheduleWithFixedDelay([this](){this->check_watchdog();}, 1000, 1000);
    }

    if (config.get("verbose")->asBool())
        this->distribution_log = heap_alloc_array<char>(DISTRIBUTION_LOG_LEN);

    initialized = true;
}

void ChargeManager::check_watchdog()
{
    if (this->watchdog_triggered || !deadline_elapsed(last_available_current_update + WATCHDOG_TIMEOUT_MS))
        return;

    this->watchdog_triggered = true;

    uint32_t default_available_current = this->default_available_current;

    logger.printfln("Charge manager watchdog triggered! Received no available current update for %d ms. Setting available current to %u mA", WATCHDOG_TIMEOUT_MS, default_available_current);

#if MODULE_AUTOMATION_AVAILABLE()
    automation.trigger_action(AutomationTriggerID::ChargeManagerWd, nullptr, [this](Config *conf, void *data) -> bool {
        return charge_manager.action_triggered(conf, data);
    });
#endif
    this->available_current.get("current")->updateUint(default_available_current);

    last_available_current_update = millis();
}

bool ChargeManager::have_chargers() {
    return charger_count > 0;
}

// Check is not 100% reliable after an uptime of 49 days because last_update might legitimately 0.
// Work around that by caching the value once all chargers were seen once.
bool ChargeManager::seen_all_chargers() {
    if (all_chargers_seen)
        return true;

    // Don't claim to have seen "all" chargers when none are configured.
    if (charger_count == 0)
        return false;

    for(size_t i = 0; i < charger_count; ++i)
        if (this->charger_state[i].last_update == 0)
            return false;

    all_chargers_seen = true;
    return true;
}

bool ChargeManager::is_charging_stopped(uint32_t last_update_cutoff)
{
    for(size_t i = 0; i < charger_count; ++i) {
        const auto &charger = this->charger_state[i];
        if (!a_after_b(charger.last_update, last_update_cutoff)) {
            return false;
        }

        if (charger.allowed_current > 0) {
            return false;
        }
    }

    return true;
}

void ChargeManager::set_all_control_pilot_disconnect(bool disconnect)
{
    control_pilot_disconnect.get("disconnect")->updateBool(disconnect);
}

bool ChargeManager::are_all_control_pilot_disconnected(uint32_t last_update_cutoff)
{
    for(size_t i = 0; i < charger_count; ++i) {
        const auto &charger = this->charger_state[i];
        if (!a_after_b(charger.last_update, last_update_cutoff)) {
            return false;
        }

        if (!charger.cp_disconnect_state) {
            return false;
        }
    }

    return true;
}

bool ChargeManager::is_control_pilot_disconnect_supported(uint32_t last_update_cutoff)
{
    for(size_t i = 0; i < charger_count; ++i) {
        const auto &charger = this->charger_state[i];
        if (!a_after_b(charger.last_update, last_update_cutoff)) {
            return false;
        }

        if (!charger.cp_disconnect_supported) {
            return false;
        }
    }

    return true;
}


const char* ChargeManager::get_charger_name(uint8_t idx) {
    return this->state.get("chargers")->get(idx)->get("name")->asEphemeralCStr();
}

#define LOCAL_LOG(fmt, ...) if(local_log) local_log += snprintf(local_log, DISTRIBUTION_LOG_LEN - (local_log - distribution_log.get()), "    " fmt "%c", __VA_ARGS__, '\0');

void ChargeManager::distribute_current()
{
    bool seen_all_chargers_local = seen_all_chargers();
    if (seen_all_chargers_local && !printed_all_chargers_seen) {
        logger.printfln("Charge manager: Seen all chargers.");
        printed_all_chargers_seen = true;
    }

    uint32_t available = seen_all_chargers_local ? available_current.get("current")->asUint() : 0;

    bool use_3phase_minimum_current = available_phases.get("phases")->asUint() >= 3;
    uint32_t minimum_current = use_3phase_minimum_current ? this->minimum_current :
                                                            this->minimum_current_1p;

    bool print_local_log = false;
    char *local_log = distribution_log.get();
    if (local_log)
        local_log += snprintf(local_log, DISTRIBUTION_LOG_LEN - (local_log - distribution_log.get()), "Redistributing current%c", '\0');

    bool any_charger_blocking_firmware_update = false;

    uint32_t current_array[MAX_CONTROLLED_CHARGERS] = {0};

    // Update control pilot disconnect
    {
        bool disconnect_requested = control_pilot_disconnect.get("disconnect")->asBool();
        for(size_t i = 0; i < charger_count; ++i) {
            this->charger_state[i].cp_disconnect = disconnect_requested;
        }
    }

    // Handle unreachable EVSEs
    {
        static bool last_print_local_log_was_error = false;

        // If any EVSE is unreachable or in another error state, we set the available current to 0.
        // The distribution algorithm can then run normally and will block all chargers.
        bool unreachable_evse_found = false;
        for (int i = 0; i < charger_count; ++i) {
            auto &charger = this->charger_state[i];

            auto charger_error = charger.error;
            if (charger_error != CM_NETWORKING_ERROR_NO_ERROR &&
                charger_error != CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE &&
                charger_error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE &&
                charger_error < CHARGE_MANAGER_CLIENT_ERROR_START) {
                unreachable_evse_found = true;
                LOCAL_LOG("stage 0: %s (%s) reports error %u.", this->get_charger_name(i), this->hosts[i], charger.error);

                print_local_log = !last_print_local_log_was_error;
                last_print_local_log_was_error = true;
            }

            // Charger does not respond anymore
            if (deadline_elapsed(charger.last_update + TIMEOUT_MS)) {
                unreachable_evse_found = true;
                LOCAL_LOG("stage 0: Can't reach EVSE of %s (%s): last_update too old.",this->get_charger_name(i), this->hosts[i]);

                bool state_was_not_five = charger.state != 5;
                charger.state = 5;
                if (state_was_not_five || charger_error < CHARGE_MANAGER_CLIENT_ERROR_START) {
                    charger.error = CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE;
                    cm_networking.clear_cached_hostname(i);

                    print_local_log = !last_print_local_log_was_error;
                    last_print_local_log_was_error = true;
                }
            } else if (charger.error == CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE) {
                charger.error = CM_NETWORKING_ERROR_NO_ERROR;
            }

            // Charger did not update the charging current in time
            if(charger.allocated_current < charger.allowed_current && deadline_elapsed(charger.last_sent_config + TIMEOUT_MS)) {
                unreachable_evse_found = true;
                LOCAL_LOG("stage 0: EVSE of %s (%s) did not react in time.", this->get_charger_name(i), this->hosts[i]);

                bool state_was_not_five = charger.state != 5;
                charger.state = 5;
                if (state_was_not_five || charger_error < CHARGE_MANAGER_CLIENT_ERROR_START) {
                    charger.error = CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE;
                    print_local_log = !last_print_local_log_was_error;
                    last_print_local_log_was_error = true;
                }
            } else if (charger.error == CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE) {
                charger.error = CM_NETWORKING_ERROR_NO_ERROR;
            }

            // Block firmware update if charger has a vehicle connected.
            if (charger.state != 0)
                any_charger_blocking_firmware_update = true;
        }

        if (unreachable_evse_found) {
            // Shut down everything.
            available = 0;
            LOCAL_LOG("%s", "stage 0: Unreachable, unreactive or misconfigured EVSE(s) found. Setting available current to 0 mA.");
            state.get("state")->updateUint(2);

            // Any unreachable EVSE will block a firmware update.
            any_charger_blocking_firmware_update = true;
        } else {
            state.get("state")->updateUint(1);
            if (last_print_local_log_was_error) {
                last_print_local_log_was_error = false;
                print_local_log = true;
            }
        }
    }

    // Sort chargers.
    {
        // Sort the chargers by their minimum supported current,
        // then sort chargers that are already charging before those that
        // want to charge but are not charging yet.
        // Sorting by the minimum current allows us to distribute the current "perfectly"
        // with a single pass over the chargers.
        int chargers_requesting_current = 0;
        int chargers_requesting_current_low_priority = 0;
        for (int i = 0; i < charger_count; ++i) {
            auto &charger = this->charger_state[i];
            if (charger.is_charging || charger.wants_to_charge) {
                ++chargers_requesting_current;
            } else if (charger.wants_to_charge_low_priority) {
                ++chargers_requesting_current_low_priority;
            }
        }

        LOCAL_LOG("%d charger%s request%s current. %u mA available.",
                  chargers_requesting_current,
                  chargers_requesting_current == 1 ? "" : "s",
                  chargers_requesting_current == 1 ? "s" : "",
                  available);

        std::stable_sort(idx_array, idx_array + charger_count, [this](int left, int right) {
            return this->charger_state[left].requested_current < this->charger_state[right].requested_current;
        });

        std::stable_sort(idx_array, idx_array + charger_count, [this](int left, int right) {
            bool left_charging = this->charger_state[left].is_charging;
            bool right_charging = this->charger_state[right].is_charging;
            return left_charging && !right_charging;
        });
    }

    // Allocate current to chargers.
    {
        // First allocate the minimum supported current to each charger.
        // Then distribute the rest of the available current to those
        // that received the minimum.
        int chargers_allocated_current_to = 0;

        uint16_t current_to_set = minimum_current;
        for (int i = 0; i < charger_count; ++i) {
            auto &charger = this->charger_state[idx_array[i]];

            if (!charger.is_charging && !charger.wants_to_charge) {
                continue;
            }

            uint16_t supported_current = charger.supported_current;
            if (supported_current < current_to_set) {
                LOCAL_LOG("stage 0: Can't unblock %s (%s): It only supports %u mA, but %u mA is the configured minimum current. Handling as low priority charger.",
                          this->get_charger_name(idx_array[i]),
                          this->hosts[idx_array[i]],
                          supported_current,
                          current_to_set);
                continue;
            }

            if (available < current_to_set) {
                LOCAL_LOG("stage 0: %u mA left, but %u mA required to unblock another charger. Blocking all following chargers.",available, current_to_set);
                current_to_set = 0;
            }

            if (current_to_set > 0) {
                ++chargers_allocated_current_to;
            }

            current_array[idx_array[i]] = current_to_set;
            available -= current_to_set;

            LOCAL_LOG("stage 0: Calculated target for %s (%s) of %u mA. %u mA left.",
                      this->get_charger_name(idx_array[i]),
                      this->hosts[idx_array[i]],
                      current_to_set,
                      available);
        }

        if (available > 0) {
            LOCAL_LOG("stage 0: %u mA still available. Recalculating targets.", available);

            int chargers_reallocated = 0;
            for (int i = 0; i < charger_count; ++i) {
                if (current_array[idx_array[i]] == 0)
                    continue;

                auto &charger = this->charger_state[idx_array[i]];
                uint16_t current_per_charger = MIN(32000, available / (chargers_allocated_current_to - chargers_reallocated));

                uint16_t requested_current = charger.requested_current;
                // Protect against overflow.
                if (requested_current < current_array[idx_array[i]])
                    continue;

                uint16_t current_to_add = MIN(requested_current - current_array[idx_array[i]], current_per_charger);

                ++chargers_reallocated;

                current_array[idx_array[i]] += current_to_add;
                available -= current_to_add;

                LOCAL_LOG("stage 0: Recalculated target for %s (%s) of %u mA. %u mA left.",
                          this->get_charger_name(idx_array[i]),
                          this->hosts[idx_array[i]],
                          current_array[idx_array[i]],
                          available);
            }
        }
    }

    // Wake up chargers that already charged once.
    {
        if (available > 0) {
            LOCAL_LOG("stage 0: %u mA still available. Attempting to wake up chargers that already charged their vehicle once.", available);

            uint16_t current_to_set = minimum_current;
            for (int i = 0; i < charger_count; ++i) {
                auto &charger = this->charger_state[idx_array[i]];

                uint16_t supported_current = charger.supported_current;

                bool high_prio = charger.is_charging || charger.wants_to_charge;
                bool low_prio = charger.wants_to_charge_low_priority;

                if (!low_prio && !(high_prio && supported_current < current_to_set)) {
                    continue;
                }

                if (supported_current < current_to_set) {
                    LOCAL_LOG("stage 0: %s (%s) only supports %u mA, but %u mA is the configured minimum current. Allocating %u mA.",
                              this->get_charger_name(idx_array[i]),
                              this->hosts[idx_array[i]],
                              supported_current,
                              current_to_set,
                              current_to_set);
                }

                if (available < current_to_set) {
                    LOCAL_LOG("stage 0: %u mA left, but %u mA required to unblock another charger. Blocking all following chargers.",available, current_to_set);
                    current_to_set = 0;
                }

                /*if (current_to_set > 0) {
                    ++chargers_allocated_current_to;
                }*/

                current_array[idx_array[i]] = current_to_set;
                available -= current_to_set;

                LOCAL_LOG("stage 0: Calculated target for %s (%s) of %u mA. %u mA left.",
                          this->get_charger_name(idx_array[i]),
                          this->hosts[idx_array[i]],
                          current_to_set,
                          available);
            }
        }
    }

    // Apply current limits.
    {
        // First, throttle chargers that have a higher current limit than the calculated one.
        // If no charger has to be throttled, then also unthrottle other chargers. Skip this
        // stage if even one charger needs to be throttled to be sure that the available current
        // is never exceeded.
        bool skip_stage_2 = false;
        for (int i = 0; i < charger_count; ++i) {
            auto &charger = this->charger_state[i];

            uint16_t current_to_set = current_array[i];

            bool will_throttle = current_to_set < charger.allocated_current || current_to_set < charger.allowed_current;

            if (!will_throttle) {
                continue;
            }

            LOCAL_LOG("stage 1: Throttled %s (%s) to %d mA.",
                      this->get_charger_name(i),
                      this->hosts[i],
                      current_to_set);

            bool current_changed = charger.allocated_current != current_to_set;
            charger.allocated_current = current_to_set;
            if (current_changed) {
                print_local_log = true;
                if (charger.error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                    charger.last_sent_config = millis();
            }

            // Skip stage 2 to wait for the charger to adapt to the now smaller limit.
            // Some cars are slow to adapt to a new limit. The standard requires them to
            // react in 5 seconds.
            // More correct would be to detect whether the throttled current limit
            // was accepted by the box more than 5 seconds ago (so that we can be sure the timing fits)
            // However this is complicated and waiting a complete cycle (i.e. 10 seconds)
            // works good enough.
            if (!skip_stage_2) {
                LOCAL_LOG("%s", "stage 1: Throttled a charger. Skipping stage 2");
                skip_stage_2 = true;
            }
        }

        if (!skip_stage_2) {
            for (int i = 0; i < charger_count; ++i) {
                auto &charger = this->charger_state[i];

                uint16_t current_to_set = current_array[i];

                // > instead of >= to only catch chargers that were not already modified in stage 1.
                bool will_not_throttle = current_to_set > charger.allocated_current || current_to_set > charger.allowed_current;

                if (!will_not_throttle) {
                    continue;
                }

                LOCAL_LOG("stage 2: Unthrottled %s (%s) to %d mA.",
                          this->get_charger_name(i),
                          this->hosts[i],
                          current_to_set);

                bool current_changed = charger.allocated_current != current_to_set;
                charger.allocated_current = current_to_set;
                if (current_changed) {
                    print_local_log = true;
                    if (charger.error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                        charger.last_sent_config = millis();
                }
            }
        } else {
            LOCAL_LOG("%s", "Skipping stage 2");
        }
    }

    if (print_local_log) {
        local_log = distribution_log.get();
        if (local_log) {
            size_t len = strlen(local_log);
            while (len > 0) {
                logger.write(local_log, len);
                local_log += len + 1;
                if ((local_log - distribution_log.get()) >= DISTRIBUTION_LOG_LEN)
                    break;
                len = strlen(local_log);
            }
        }
    }

    if (allocated_current_callback) {
        uint32_t allocated_current = 0;
        for (int i = 0; i < charger_count; ++i) {
            auto &charger = this->charger_state[i];
            allocated_current += charger.allocated_current;
        }

        // Inform callback about how much current we distributed to chargers.
        allocated_current_callback(allocated_current);
    }

#if MODULE_ENERGY_MANAGER_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
    firmware_update_allowed = !any_charger_blocking_firmware_update;
#else
    (void)any_charger_blocking_firmware_update;
#endif

    for (int i = 0; i < charger_count; ++i) {
        auto &charger = this->charger_state[i];
        auto *charger_cfg = (Config *)this->state.get("chargers")->get(i);
        charger_cfg->get("state")->updateUint(charger.state);
        charger_cfg->get("error")->updateUint(charger.error);
        charger_cfg->get("allocated_current")->updateUint(charger.allocated_current);
        charger_cfg->get("supported_current")->updateUint(charger.supported_current);
        charger_cfg->get("last_update")->updateUint(charger.last_update);
        charger_cfg->get("uid")->updateUint(charger.uid);
    }
}

void ChargeManager::set_allocated_current_callback(std::function<void(uint32_t)> callback) {
    allocated_current_callback = callback;
}

void ChargeManager::register_urls()
{
    api.addPersistentConfig("charge_manager/config", &config);
    api.addState("charge_manager/state", &state);

    api.addState("charge_manager/available_current", &available_current);
    api.addCommand("charge_manager/available_current_update", &available_current_update, {}, [this](){
        uint32_t current = this->available_current_update.get("current")->asUint();
        this->available_current.get("current")->updateUint(current);
        this->last_available_current_update = millis();
        this->watchdog_triggered = false;
    }, false);

    api.addState("charge_manager/available_phases", &available_phases);
    api.addCommand("charge_manager/available_phases_update", &available_phases_update, {}, [this](){
        uint32_t phases = this->available_phases_update.get("phases")->asUint();
        this->available_phases.get("phases")->updateUint(phases);
        logger.printfln("charge_manager: Available phases: %u", phases);
    }, false);

    //api.addState("charge_manager/control_pilot_disconnect", &control_pilot_disconnect);
    //api.addCommand("charge_manager/control_pilot_disconnect_update", &control_pilot_disconnect, {}, [](){}, false);
}
