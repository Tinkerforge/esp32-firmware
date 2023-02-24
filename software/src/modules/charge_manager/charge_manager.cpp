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

#include <Arduino.h>
#include "time.h"

#include "api.h"
#include "task_scheduler.h"
#include "tools.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_client.h"

#include "modules.h"
#include "build.h"

#include <algorithm>

// Keep in sync with cm_networing.h
#define MAX_CLIENTS 10

#define CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE 128
#define CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE 129
#define CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE 130
#define CHARGE_MANAGER_CLIENT_ERROR_START 192

// This is a hack to allow the validator of charge_manager_available_current
// to access charge_manager_config["maximum_available_current"]
// It is necessary, because configs only take a function pointer as
// validator function, so lambda capture lists have to be empty.
static uint32_t max_avail_current = 0;

#define TIMEOUT_MS 32000

#define DISTRIBUTION_LOG_LEN 2048
static char distribution_log[DISTRIBUTION_LOG_LEN] = {0};

#define WATCHDOG_TIMEOUT_MS 30000

#if MODULE_ENERGY_MANAGER_AVAILABLE()
static void apply_energy_manager_config(Config &conf)
{
    conf.get("enable_charge_manager")->updateBool(true);
    conf.get("enable_watchdog")->updateBool(false);
    conf.get("default_available_current")->updateUint(0);
}
#endif

void ChargeManager::pre_setup()
{
    charge_manager_config = ConfigRoot{Config::Object({
        {"enable_charge_manager", Config::Bool(false)},
        {"enable_watchdog", Config::Bool(false)},
        {"default_available_current", Config::Uint32(0)},
        {"maximum_available_current", Config::Uint32(0xFFFFFFFF)},
        {"minimum_current", Config::Uint(6000, 6000, 32000)},
        {"verbose", Config::Bool(false)},
        {"chargers", Config::Array({},
            new Config{Config::Object({
                {"host", Config::Str("", 0, 64)},
                {"name", Config::Str("", 0, 32)}
            })},
            0, MAX_CLIENTS, Config::type_id<Config::ConfObject>()
        )}
    }), [](Config &conf) -> String {
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

        return "";
    }};

    charge_manager_state = Config::Object({
        {"state", Config::Uint8(0)}, // 0 - not configured, 1 - active, 2 - shutdown
        {"uptime", Config::Uint32(0)},
        {"allocated_current", Config::Uint32(0)},
        {"chargers", Config::Array(
            {},
            new Config{Config::Object({
                {"name", Config::Str("", 0, 32)},
                {"last_update", Config::Uint32(0)},
                {"uptime", Config::Uint32(0)},
                {"supported_current", Config::Uint16(0)}, // maximum current supported by the charger
                {"allowed_current", Config::Uint16(0)}, // last current limit reported by the charger
                {"wants_to_charge", Config::Bool(false)},
                {"wants_to_charge_low_priority", Config::Bool(false)},
                {"is_charging", Config::Bool(false)},

                {"cp_disconnect_supported", Config::Bool(false)}, // last CP disconnect support reported by the charger: false - CP disconnect not supported, true - CP disconnect supported
                {"cp_disconnect_state", Config::Bool(false)}, // last CP disconnect state reported by the charger: false - automatic, true - disconnected
                {"cp_disconnect", Config::Bool(false)}, // last CP disconnect request sent to charger: false - automatic/don't care, true - disconnect

                {"last_sent_config", Config::Uint32(0)},
                {"allocated_current", Config::Uint16(0)}, // last current limit send to the charger

                {"state", Config::Uint8(0)}, // 0 - no vehicle, 1 - user blocked, 2 - manager blocked, 3 - car blocked, 4 - charging, 5 - error, 6 - charged
                {"error", Config::Uint8(0)} // 0 - okay, 1 - unreachable, 2 - FW mismatch, 3 - not managed
            })},
            0, MAX_CLIENTS, Config::type_id<Config::ConfObject>()
        )}
    });

    charge_manager_available_current = ConfigRoot{Config::Object({
        {"current", Config::Uint32(0)},
    }), [](Config &conf) -> String {
        if (conf.get("current")->asUint() > max_avail_current)
            return String("Current too large: maximum available current is configured to ") + String(max_avail_current);
        return "";
    }};

    charge_manager_control_pilot_disconnect = ConfigRoot{Config::Object({
        {"disconnect", Config::Bool(false)},
    })};
}

uint8_t get_charge_state(uint8_t charger_state, uint16_t supported_current, uint32_t charging_time, uint16_t target_allocated_current)
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
    std::vector<Config> &chargers = charge_manager_config_in_use.get("chargers")->asArray();

    std::vector<String> hosts;
    std::vector<String> names;
    for (int i = 0; i < chargers.size(); ++i) {
        hosts.push_back(chargers[i].get("host")->asString());
        names.push_back(chargers[i].get("name")->asString());
    }

    cm_networking.register_manager(std::move(hosts), names, [this, chargers](
            uint8_t client_id,
            uint8_t iec61851_state,
            uint8_t charger_state,
            uint8_t error_state,
            uint32_t uptime,
            uint32_t charging_time,
            uint16_t allowed_charging_current,
            uint16_t supported_current,
            bool cp_disconnect_supported,
            bool cp_disconnected_state
        ){
            Config &target = charge_manager_state.get("chargers")->asArray()[client_id];
            // Don't update if the uptimes are the same.
            // This means, that the EVSE hangs or the communication
            // is not working. As last_update will now hang too,
            // the management will stop all charging after some time.
            if (target.get("uptime")->asUint() == uptime) {
                logger.printfln("Received stale charger state from %s (%s). Reported EVSE uptime (%u) is the same as in the last state. Is the EVSE still reachable?",
                    chargers[client_id].get("name")->asEphemeralCStr(), chargers[client_id].get("host")->asEphemeralCStr(),
                    uptime);
                if (deadline_elapsed(target.get("last_update")->asUint() + 10000)) {
                    target.get("state")->updateUint(5);
                    target.get("error")->updateUint(CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE);
                }

                return;
            }

            target.get("uptime")->updateUint(uptime);

            // A charger wants to charge if:
            // - the charging time is 0 (it has not charged this vehicle yet), no other slot blocks and we are still in charger state 1 (i.e. blocked by a slot, so the charge management slot)
            // - OR the charger waits for the vehicle to start charging
            // - OR the charger is already charging
            bool wants_to_charge = (charging_time == 0 && supported_current != 0 && charger_state == 1) || charger_state == 2 || charger_state == 3;
            target.get("wants_to_charge")->updateBool(wants_to_charge);

            // A charger wants to charge and has low priority if it has already charged this vehicle and only the charge manager slot blocks.
            bool low_prio = charging_time != 0 && supported_current != 0 && charger_state == 1;
            target.get("wants_to_charge_low_priority")->updateBool(low_prio);

            target.get("is_charging")->updateBool(charger_state == 3);
            target.get("allowed_current")->updateUint(allowed_charging_current);
            target.get("supported_current")->updateUint(supported_current);
            target.get("cp_disconnect_supported")->updateBool(cp_disconnect_supported);
            target.get("cp_disconnect_state")->updateBool(cp_disconnected_state);
            target.get("last_update")->updateUint(millis());

            if (error_state != 0) {
                target.get("error")->updateUint(CHARGE_MANAGER_CLIENT_ERROR_START + error_state);
            }

            auto current_error = target.get("error")->asUint();
            if (current_error < 128 || current_error == CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE) {
                target.get("error")->updateUint(0);
            }

            current_error = target.get("error")->asUint();
            if (current_error == 0 || current_error >= CHARGE_MANAGER_CLIENT_ERROR_START)
                target.get("state")->updateUint(get_charge_state(charger_state,
                                                                 supported_current,
                                                                 charging_time,
                                                                 target.get("allocated_current")->asUint()));
            charge_manager_state.get("uptime")->updateUint(millis());
    }, [this](uint8_t client_id, uint8_t error){
        Config &target = charge_manager_state.get("chargers")->asArray()[client_id];
        target.get("state")->updateUint(5);
        target.get("error")->updateUint(error);
    });

    uint32_t cm_send_delay = 1000 / chargers.size();

    task_scheduler.scheduleWithFixedDelay([this, chargers](){
        static int i = 0;

        if (i >= chargers.size())
            i = 0;

        Config &state = charge_manager_state.get("chargers")->asArray()[i];
        if(cm_networking.send_manager_update(i, state.get("allocated_current")->asUint(), state.get("cp_disconnect")->asBool()))
            ++i;

    }, 0, cm_send_delay);
}

int idx_array[MAX_CLIENTS] = {0};

void ChargeManager::setup()
{
    uint32_t control_cycle_time_ms;
    if (!api.restorePersistentConfig("charge_manager/config", &charge_manager_config)) {
        charge_manager_config.get("maximum_available_current")->updateUint(0);
#if MODULE_ENERGY_MANAGER_AVAILABLE()
        apply_energy_manager_config(charge_manager_config);
        control_cycle_time_ms = 5 * 1000;
#else
        control_cycle_time_ms = 10 * 1000;
#endif
    }

    charge_manager_config_in_use = charge_manager_config;

    max_avail_current = charge_manager_config_in_use.get("maximum_available_current")->asUint();

    if(!charge_manager_config_in_use.get("enable_charge_manager")->asBool() || charge_manager_config_in_use.get("chargers")->asArray().size() == 0) {
        initialized = true;
        return;
    }
    charge_manager_state.get("state")->updateUint(1);

    charge_manager_available_current.get("current")->updateUint(charge_manager_config_in_use.get("default_available_current")->asUint());
    for (int i = 0; i < charge_manager_config_in_use.get("chargers")->asArray().size(); ++i) {
        charge_manager_state.get("chargers")->add();
        charge_manager_state.get("chargers")->get(i)->get("name")->updateString(charge_manager_config_in_use.get("chargers")->get(i)->get("name")->asString());
        idx_array[i] = i;
    }

    for (int i = charge_manager_config_in_use.get("chargers")->asArray().size(); i < MAX_CLIENTS; ++i)
        idx_array[i] = -1;

    start_manager_task();

    task_scheduler.scheduleWithFixedDelay([this](){this->distribute_current();}, control_cycle_time_ms, control_cycle_time_ms);

    if (charge_manager_config_in_use.get("enable_watchdog")->asBool()) {
        task_scheduler.scheduleWithFixedDelay([this](){this->check_watchdog();}, 1000, 1000);
    }

    initialized = true;
}

void ChargeManager::check_watchdog()
{
    if (!deadline_elapsed(last_available_current_update + WATCHDOG_TIMEOUT_MS))
        return;

    uint32_t default_available_current = this->charge_manager_config_in_use.get("default_available_current")->asUint();

    logger.printfln("Charge manager watchdog triggered! Received no available current update for %d ms. Setting available current to %u mA", WATCHDOG_TIMEOUT_MS, default_available_current);

    this->charge_manager_available_current.get("current")->updateUint(default_available_current);

    last_available_current_update = millis();
}

void ChargeManager::set_available_current(uint32_t current)
{
    charge_manager_available_current.get("current")->updateUint(current);
}

bool ChargeManager::have_chargers() {
    return charge_manager_state.get("chargers")->count() > 0;
}

// Check is not 100% reliable after an uptime of 49 days because last_update might legitimately 0.
// Work around that by caching the value once all chargers were seen once.
bool ChargeManager::seen_all_chargers() {
    if (all_chargers_seen)
        return true;

    std::vector<Config> &chargers = charge_manager_state.get("chargers")->asArray();

    // Don't claim to have seen "all" chargers when none are configured.
    if (chargers.size() == 0)
        return false;

    for (auto &charger : chargers) {
        if (charger.get("last_update")->asUint() == 0) {
            return false;
        }
    }

    all_chargers_seen = true;
    return true;
}

bool ChargeManager::is_charging_stopped(uint32_t last_update_cutoff)
{
    std::vector<Config> &chargers = charge_manager_state.get("chargers")->asArray();

    for (auto &charger : chargers) {
        if (!a_after_b(charger.get("last_update")->asUint(), last_update_cutoff)) {
            return false;
        }

        if (charger.get("allowed_current")->asUint() > 0) {
            return false;
        }
    }

    return true;
}

void ChargeManager::set_all_control_pilot_disconnect(bool disconnect)
{
    charge_manager_control_pilot_disconnect.get("disconnect")->updateBool(disconnect);
}

bool ChargeManager::are_all_control_pilot_disconnected(uint32_t last_update_cutoff)
{
    std::vector<Config> &chargers = charge_manager_state.get("chargers")->asArray();

    for (auto &charger : chargers) {
        if (!a_after_b(charger.get("last_update")->asUint(), last_update_cutoff)) {
            return false;
        }

        if (!charger.get("cp_disconnect_state")->asBool()) {
            return false;
        }
    }

    return true;
}

bool ChargeManager::is_control_pilot_disconnect_supported(uint32_t last_update_cutoff)
{
    std::vector<Config> &chargers = charge_manager_state.get("chargers")->asArray();

    for (auto &charger : chargers) {
        if (!a_after_b(charger.get("last_update")->asUint(), last_update_cutoff)) {
            return false;
        }

        if (!charger.get("cp_disconnect_supported")->asBool()) {
            return false;
        }
    }

    return true;
}

#define LOCAL_LOG(fmt, ...) if(verbose) local_log += snprintf(local_log, DISTRIBUTION_LOG_LEN - (local_log - distribution_log), "    " fmt "%c", __VA_ARGS__, '\0');

void ChargeManager::distribute_current()
{
    uint32_t available_current_init = charge_manager_available_current.get("current")->asUint();
    uint32_t available_current = available_current_init;

    static bool verbose = charge_manager_config_in_use.get("verbose")->asBool();

    bool print_local_log = false;
    char *local_log = distribution_log;
    if (verbose)
        local_log += snprintf(local_log, DISTRIBUTION_LOG_LEN - (local_log - distribution_log), "Redistributing current%c", '\0');

    auto &chargers = charge_manager_state.get("chargers")->asArray();
    auto &configs = charge_manager_config_in_use.get("chargers")->asArray();

    uint32_t current_array[MAX_CLIENTS] = {0};

    // Update control pilot disconnect
    {
        bool disconnect_requested = charge_manager_control_pilot_disconnect.get("disconnect")->asBool();
        for (auto &charger : chargers) {
            charger.get("cp_disconnect")->updateBool(disconnect_requested);
        }
    }

    // Handle unreachable EVSEs
    {
        static bool last_print_local_log_was_error = false;

        // If any EVSE is unreachable or in another error state, we set the available current to 0.
        // The distribution algorithm can then run normally and will block all chargers.
        bool unreachable_evse_found = false;
        for (int i = 0; i < chargers.size(); ++i) {
            auto &charger = chargers[i];
            auto &charger_cfg = configs[i];

            auto charger_error = charger.get("error")->asUint();
            if (charger_error != CM_NETWORKING_ERROR_NO_ERROR &&
                charger_error != CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE &&
                charger_error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE &&
                charger_error < CHARGE_MANAGER_CLIENT_ERROR_START) {
                unreachable_evse_found = true;
                LOCAL_LOG("stage 0: %s (%s) reports error %u.", charger_cfg.get("name")->asEphemeralCStr(), charger_cfg.get("host")->asEphemeralCStr(), charger.get("error")->asUint());

                print_local_log = !last_print_local_log_was_error;
                last_print_local_log_was_error = true;
            }

            // Charger does not respond anymore
            if (deadline_elapsed(charger.get("last_update")->asUint() + TIMEOUT_MS)) {
                unreachable_evse_found = true;
                LOCAL_LOG("stage 0: Can't reach EVSE of %s (%s): last_update too old.",charger_cfg.get("name")->asEphemeralCStr(), charger_cfg.get("host")->asEphemeralCStr());

                if (chargers[i].get("state")->updateUint(5) || charger_error < CHARGE_MANAGER_CLIENT_ERROR_START) {
                    chargers[i].get("error")->updateUint(CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE);
                    print_local_log = !last_print_local_log_was_error;
                    last_print_local_log_was_error = true;
                }
            } else if (chargers[i].get("error")->asUint() == CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE) {
                chargers[i].get("error")->updateUint(CM_NETWORKING_ERROR_NO_ERROR);
            }

            // Charger did not update the charging current in time
            if(charger.get("allocated_current")->asUint() < charger.get("allowed_current")->asUint() && deadline_elapsed(charger.get("last_sent_config")->asUint() + TIMEOUT_MS)) {
                unreachable_evse_found = true;
                LOCAL_LOG("stage 0: EVSE of %s (%s) did not react in time.", charger_cfg.get("name")->asEphemeralCStr(), charger_cfg.get("host")->asEphemeralCStr());

                if (chargers[i].get("state")->updateUint(5) || charger_error < CHARGE_MANAGER_CLIENT_ERROR_START) {
                    chargers[i].get("error")->updateUint(CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE);
                    print_local_log = !last_print_local_log_was_error;
                    last_print_local_log_was_error = true;
                }
            } else if (chargers[i].get("error")->asUint() == CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE) {
                chargers[i].get("error")->updateUint(CM_NETWORKING_ERROR_NO_ERROR);
            }
        }

        if (unreachable_evse_found) {
            // Shut down everything.
            available_current = 0;
            LOCAL_LOG("%s", "stage 0: Unreachable, unreactive or misconfigured EVSE(s) found. Setting available current to 0 mA.");
            charge_manager_state.get("state")->updateUint(2);
        } else {
            charge_manager_state.get("state")->updateUint(1);
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
        for (auto &charger : chargers) {
            if (!charger.get("is_charging")->asBool() && !charger.get("wants_to_charge")->asBool()) {
                continue;
            }
            ++chargers_requesting_current;
        }

        LOCAL_LOG("%d charger%s request%s current. %u mA available.",
                  chargers_requesting_current,
                  chargers_requesting_current == 1 ? "" : "s",
                  chargers_requesting_current == 1 ? "s" : "",
                  available_current);

        std::stable_sort(idx_array, idx_array + chargers.size(), [&chargers](int left, int right) {
            return chargers[left].get("supported_current")->asUint() < chargers[right].get("supported_current")->asUint();
        });

        std::stable_sort(idx_array, idx_array + chargers.size(), [&chargers](int left, int right) {
            bool left_charging = chargers[left].get("is_charging")->asBool();
            bool right_charging = chargers[right].get("is_charging")->asBool();
            return left_charging && !right_charging;
        });
    }

    // Allocate current to chargers.
    {
        // First allocate the minimum supported current to each charger.
        // Then distribute the rest of the available current to those
        // that received the minimum.
        int chargers_allocated_current_to = 0;

        uint16_t current_to_set = charge_manager_config_in_use.get("minimum_current")->asUint();
        for (int i = 0; i < chargers.size(); ++i) {
            auto &charger = chargers[idx_array[i]];

            if (!charger.get("is_charging")->asBool() && !charger.get("wants_to_charge")->asBool()) {
                continue;
            }

            auto &charger_cfg = configs[idx_array[i]];

            uint16_t supported_current = charger.get("supported_current")->asUint();
            if (supported_current < current_to_set) {
                LOCAL_LOG("stage 0: Can't unblock %s (%s): It only supports %u mA, but %u mA is the configured minimum current.",
                          charger_cfg.get("name")->asEphemeralCStr(),
                          charger_cfg.get("host")->asEphemeralCStr(),
                          supported_current,
                          current_to_set);
                continue;
            }

            if (available_current < current_to_set) {
                LOCAL_LOG("stage 0: %u mA left, but %u mA required to unblock another charger. Blocking all following chargers.",available_current, current_to_set);
                current_to_set = 0;
            }

            if (current_to_set > 0) {
                ++chargers_allocated_current_to;
            }

            current_array[idx_array[i]] = current_to_set;
            available_current -= current_to_set;

            LOCAL_LOG("stage 0: Calculated target for %s (%s) of %u mA. %u mA left.",
                      charger_cfg.get("name")->asEphemeralCStr(),
                      charger_cfg.get("host")->asEphemeralCStr(),
                      current_to_set,
                      available_current);
        }

        if (available_current > 0) {
            LOCAL_LOG("stage 0: %u mA still available. Recalculating targets.", available_current);

            int chargers_reallocated = 0;
            for (int i = 0; i < chargers.size(); ++i) {
                if (current_array[idx_array[i]] == 0)
                    continue;

                auto &charger = chargers[idx_array[i]];
                uint16_t current_per_charger = MIN(32000, available_current / (chargers_allocated_current_to - chargers_reallocated));

                uint16_t supported_current = charger.get("supported_current")->asUint();
                // Protect against overflow.
                if (supported_current < current_array[idx_array[i]])
                    continue;

                uint16_t current_to_add = MIN(supported_current - current_array[idx_array[i]], current_per_charger);

                ++chargers_reallocated;

                current_array[idx_array[i]] += current_to_add;
                available_current -= current_to_add;

                auto &charger_cfg = configs[idx_array[i]];
                LOCAL_LOG("stage 0: Recalculated target for %s (%s) of %u mA. %u mA left.",
                          charger_cfg.get("name")->asEphemeralCStr(),
                          charger_cfg.get("host")->asEphemeralCStr(),
                          current_array[idx_array[i]],
                          available_current);
            }
        }
    }

    // Wake up chargers that already charged once.
    {
        if (available_current > 0) {
            LOCAL_LOG("stage 0: %u mA still available. Attempting to wake up chargers that already charged their vehicle once.", available_current);

            uint16_t current_to_set = charge_manager_config_in_use.get("minimum_current")->asUint();
            for (int i = 0; i < chargers.size(); ++i) {
                auto &charger = chargers[idx_array[i]];

                if (!charger.get("wants_to_charge_low_priority")->asBool()) {
                    continue;
                }

                auto &charger_cfg = configs[idx_array[i]];

                uint16_t supported_current = charger.get("supported_current")->asUint();
                if (supported_current < current_to_set) {
                    LOCAL_LOG("stage 0: Can't unblock %s (%s): It only supports %u mA, but %u mA is the configured minimum current.",
                              charger_cfg.get("name")->asEphemeralCStr(),
                              charger_cfg.get("host")->asEphemeralCStr(),
                              supported_current,
                              current_to_set);
                    continue;
                }

                if (available_current < current_to_set) {
                    LOCAL_LOG("stage 0: %u mA left, but %u mA required to unblock another charger. Blocking all following chargers.",available_current, current_to_set);
                    current_to_set = 0;
                }

                /*if (current_to_set > 0) {
                    ++chargers_allocated_current_to;
                }*/

                current_array[idx_array[i]] = current_to_set;
                available_current -= current_to_set;

                LOCAL_LOG("stage 0: Calculated target for %s (%s) of %u mA. %u mA left.",
                          charger_cfg.get("name")->asEphemeralCStr(),
                          charger_cfg.get("host")->asEphemeralCStr(),
                          current_to_set,
                          available_current);
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
        for (int i = 0; i < chargers.size(); ++i) {
            auto &charger = chargers[i];

            auto &charger_cfg = configs[i];
            uint16_t current_to_set = current_array[i];

            bool will_throttle = current_to_set < charger.get("allocated_current")->asUint() || current_to_set < charger.get("allowed_current")->asUint();

            if (!will_throttle) {
                continue;
            }

            LOCAL_LOG("stage 1: Throttled %s (%s) to %d mA.",
                      charger_cfg.get("name")->asEphemeralCStr(),
                      charger_cfg.get("host")->asEphemeralCStr(),
                      current_to_set);

            if (charger.get("allocated_current")->updateUint(current_to_set)) {
                print_local_log = true;
                if (charger.get("error")->asUint() != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                    charger.get("last_sent_config")->updateUint(millis());
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
            for (int i = 0; i < chargers.size(); ++i) {
                auto &charger = chargers[i];

                auto &charger_cfg = configs[i];
                uint16_t current_to_set = current_array[i];

                // > instead of >= to only catch chargers that were not already modified in stage 1.
                bool will_not_throttle = current_to_set > charger.get("allocated_current")->asUint() || current_to_set > charger.get("allowed_current")->asUint();

                if (!will_not_throttle) {
                    continue;
                }

                LOCAL_LOG("stage 2: Unthrottled %s (%s) to %d mA.",
                          charger_cfg.get("name")->asEphemeralCStr(),
                          charger_cfg.get("host")->asEphemeralCStr(),
                          current_to_set);

                if (charger.get("allocated_current")->updateUint(current_to_set)) {
                    print_local_log = true;
                    if (charger.get("error")->asUint() != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                        charger.get("last_sent_config")->updateUint(millis());
                }
            }
        } else {
            LOCAL_LOG("%s", "Skipping stage 2");
        }
    }

    if (print_local_log) {
        local_log = distribution_log;
        size_t len = strlen(local_log);
        while (len > 0) {
            logger.write(local_log, len);
            local_log += len + 1;
            if ((local_log - distribution_log) >= DISTRIBUTION_LOG_LEN)
                break;
            len = strlen(local_log);
        }
    }

    charge_manager_state.get("allocated_current")->updateUint(available_current_init - available_current);

    if (allocated_current_callback) {
        // Inform callback about how much current we distributed to chargers.
        allocated_current_callback(available_current_init - available_current);
    }
}

void ChargeManager::set_allocated_current_callback(std::function<void(uint32_t)> callback) {
    allocated_current_callback = callback;
}

void ChargeManager::register_urls()
{
    api.addPersistentConfig("charge_manager/config", &charge_manager_config, {}, 1000);
    api.addState("charge_manager/state", &charge_manager_state, {}, 1000);
    api.addState("charge_manager/available_current", &charge_manager_available_current, {}, 1000);
    api.addCommand("charge_manager/available_current_update", &charge_manager_available_current, {}, [this](){
        this->last_available_current_update = millis();
    }, false);
    //api.addState("charge_manager/control_pilot_disconnect", &charge_manager_control_pilot_disconnect, {}, 1000);
    //api.addCommand("charge_manager/control_pilot_disconnect_update", &charge_manager_control_pilot_disconnect, {}, [](){}, false);
}

void ChargeManager::loop()
{
}
