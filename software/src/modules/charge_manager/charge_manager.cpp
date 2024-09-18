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

#include <algorithm>
#include <Arduino.h>
#include <esp_http_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "current_allocator.h"
#include "build.h"
#include "tools.h"

// This is a hack to allow the validator of available_current
// to access config["maximum_available_current"]
// It is necessary, because configs only take a function pointer as
// validator function, so lambda capture lists have to be empty.
static uint32_t max_avail_current = 0;

#define WATCHDOG_TIMEOUT_MS 30000

// If this is an energy manager, we have exactly one charger and the margin is still the default,
// double it to react faster if more current is available.
#define REQUESTED_CURRENT_MARGIN_DEFAULT 3000

extern ChargeManager charge_manager;

#if MODULE_EM_COMMON_AVAILABLE()
static void apply_energy_manager_config(Config &conf)
{
    bool has_chargers = conf.get("chargers")->count() > 0;
    conf.get("enable_charge_manager")->updateBool(has_chargers);

    conf.get("enable_watchdog")->updateBool(false);
    conf.get("default_available_current")->updateUint(0);
}
#endif

#if MODULE_AUTOMATION_AVAILABLE()
bool ChargeManager::has_triggered(const Config *conf, void *data)
{
    switch (conf->getTag<AutomationTriggerID>()) {
        case AutomationTriggerID::ChargeManagerWd:
            return true;

        default:
            return false;
    }
}
#endif

enum class CMPhaseRotation: uint8_t {
    Unknown,
    L123,
    L132,
    L231,
    L213,
    L321,
    L312
};

static PhaseRotation convert_phase_rotation(CMPhaseRotation pr) {
    switch (pr) {
        case CMPhaseRotation::Unknown: return PhaseRotation::Unknown;
        case CMPhaseRotation::L123:    return PhaseRotation::L123;
        case CMPhaseRotation::L132:    return PhaseRotation::L132;
        case CMPhaseRotation::L231:    return PhaseRotation::L231;
        case CMPhaseRotation::L213:    return PhaseRotation::L213;
        case CMPhaseRotation::L321:    return PhaseRotation::L321;
        case CMPhaseRotation::L312:    return PhaseRotation::L312;
        default: return PhaseRotation::Unknown;
    }
}

void ChargeManager::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable_charge_manager", Config::Bool(false)},
        {"enable_watchdog", Config::Bool(false)},
        {"default_available_current", Config::Uint32(0)},
        {"maximum_available_current", Config::Uint(0, 0, 32000 * MAX_CONTROLLED_CHARGERS)},
        {"minimum_current_auto", Config::Bool(true)},
        {"minimum_current", Config::Uint(6000, 6000, 32000)},
        {"minimum_current_1p", Config::Uint(6000, 6000, 32000)},
        {"minimum_current_vehicle_type", Config::Uint32(0)},
        {"verbose", Config::Bool(false)},
        // Line currents are used to calculate the "requested current"
        // if a charger is in state C for at least
        // "requested_current_threshold" s.
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
                {"name", Config::Str("", 0, 32)},
                {"rot", Config::Uint((uint8_t)CMPhaseRotation::Unknown, (uint8_t)CMPhaseRotation::Unknown, (uint8_t)CMPhaseRotation::L312)}
            })},
            0, MAX_CONTROLLED_CHARGERS, Config::type_id<Config::ConfObject>()
        )}
    }), [](Config &conf, ConfigSource source) -> String {
#if MODULE_EM_COMMON_AVAILABLE()
        apply_energy_manager_config(conf);
#else
        uint32_t default_available_current = conf.get("default_available_current")->asUint();
        uint32_t maximum_available_current = conf.get("maximum_available_current")->asUint();

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

    low_level_config = Config::Object({
        {"global_hysteresis", Config::Uint(3 * 60, 0, 60 * 60)},
        {"alloc_energy_rot_thres", Config::Uint(5, 0, 100)},
        {"min_active_time", Config::Uint(15 * 60, 0, 24 * 60 * 60)},
        {"wakeup_time", Config::Uint(3 * 60, 0, 60 * 60)},
        {"plug_in_time", Config::Uint(3 * 60, 0, 60 * 60)},
        {"enable_current_factor_pct", Config::Uint(150, 100, 300)},
        {"allocation_interval", Config::Uint(10, 1, 60 * 60)},
    });

    // This has to fit in the 6k WebSocket send buffer with 32 chargers with long names. (Currently 4076 bytes)
    // -> Be stingy with the key names. If we need more space we could switch to an array for all numbers.
    // This API only exists to communicate with the web interface and is not documented.
    state = Config::Object({
        {"state", Config::Uint8(0)}, // 0 - not configured, 1 - active, 2 - shutdown
        {"l_raw", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            new Config{Config::Int32(0)},
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"l_min", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            new Config{Config::Int32(0)},
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"l_spread", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            new Config{Config::Int32(0)},
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"l_max_pv", Config::Int32(0)},
        {"alloc", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            new Config{Config::Int32(0)},
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"chargers", Config::Array(
            {},
            new Config{Config::Object({
                {"s", Config::Uint(0, 0, 6)},      // "state" - 0 - no vehicle, 1 - user blocked, 2 - manager blocked, 3 - car blocked, 4 - charging, 5 - error, 6 - charged
                {"e", Config::Uint8(0)},      // "error" - 0 - okay, 1 - unreachable, 2 - FW mismatch, 3 - not managed, >= 192 client errors
                {"ac", Config::Uint16(0)},    // "allocated_current" - last current limit send to the charger
                {"ap",  Config::Uint(0, 0, 3)},    // "allocated_phases" - last phase limit send to the charger
                {"sc", Config::Uint16(0)},    // "supported_current" - maximum current supported by the charger
                {"sp",  Config::Uint(0, 0, 3)},    // "supported_phases" - maximum phases supported by the charger. Is 1 or 3 for a not phase-switchable charger. Bit 2 is set if it can be switched.
                {"lu", Config::Uint32(0)},    // "last_update" - The last time we've received a CM packet from this charger
                {"n", Config::Str("", 0, 32)},// "name" - Configured display name. Has to be duplicated in case the config was written but we didn't reboot.
                {"u", Config::Uint32(0)}     // "uid" - The ESPs UID
            })},
            0, MAX_CONTROLLED_CHARGERS, Config::type_id<Config::ConfObject>()
        )}
    });

    low_level_state = Config::Object({
        {"last_hyst_reset", Config::Uint32(0)},
        {"wnd_min", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            new Config{Config::Int32(0)},
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"wnd_max", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            new Config{Config::Int32(0)},
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"chargers", Config::Array(
            {},
            new Config{Config::Object({
                {"b", Config::Uint(0, 0, 0x3F)},   /* "bits"
                                                       0 cp_disc_supported
                                                       1 cp_disc_state
                                                       2 phase_switch_supported
                                                       3 phases (2)
                                                       5 last_alloc_fulfilled_reqd*/
                {"rc", Config::Uint16(0)},    // "requested_current" - either the supported current or (after requested_current_threshold is elapsed in state C) the max phase current + requested_current_margin
                {"ae", Config::Uint(0, 9, 99999)}, // "allocated_energy" in Wh, values > 99999 Wh are truncated to 99999.
                {"ar", Config::Uint(0, 9, 99999)}, // "allocated_energy_this_rotation" in Wh, values > 99999 Wh are truncated to 99999.
                {"ls", Config::Uint32(0)},         // "last_switch" in millis
                {"lp", Config::Uint32(0)},         // "last_plug_in" in millis
                {"lw", Config::Uint32(0)},         // "last_wakeup" in millis
                {"ip", Config::Uint32(0)},         // "ignore_phase_currents" in millis
            })},
            0, MAX_CONTROLLED_CHARGERS, Config::type_id<Config::ConfObject>()
        )}
    });

    available_current = ConfigRoot{Config::Object({
        {"current", Config::Uint32(0)},
    }), [this](const Config &conf, ConfigSource source) -> String {
        if (source == ConfigSource::API && !this->static_cm) {
            logger.printfln("Cannot set available_current via the API if the Power Manager is enabled.");
            return "Cannot set available_current if the Power Manager is enabled.";
        }

        if (conf.get("current")->asUint() > max_avail_current)
            return "Current too large: maximum available current is configured to " + String(max_avail_current);
        return "";
    }};
    available_current_update = available_current;

    control_pilot_disconnect = Config::Object({
        {"disconnect", Config::Bool(false)},
    });

#if MODULE_AUTOMATION_AVAILABLE() && !MODULE_EM_COMMON_AVAILABLE()
    automation.register_trigger(
        AutomationTriggerID::ChargeManagerWd,
        *Config::Null(),
        nullptr, false
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
        },
        false);
#endif
}

void ChargeManager::start_manager_task()
{
    auto charger_count = config.get("chargers")->count();

    auto get_charger_name_fn = [this](uint8_t i){ return this->get_charger_name(i);};

    cm_networking.register_manager(this->hosts.get(), config.get("chargers")->count(), [this, get_charger_name_fn](uint8_t client_id, cm_state_v1 *v1, cm_state_v2 *v2, cm_state_v3 *v3) mutable {
            if (update_from_client_packet(
                    client_id,
                    v1,
                    v2,
                    v3,
                    this->ca_config,
                    this->charger_state,
                    this->charger_allocation_state,
                    this->hosts.get(),
                    get_charger_name_fn
                    ))
                update_charger_state_config(client_id);
    }, [this](uint8_t client_id, uint8_t error){
        //TODO bounds check
        auto &target_alloc = this->charger_allocation_state[client_id];
        target_alloc.state = 5;
        target_alloc.error = error;
        //TODO: should we call update_charger_state_config(client_id); here? This is currently missing but smells weird.
    });

    uint32_t cm_send_delay = 1000 / charger_count;

    task_scheduler.scheduleWithFixedDelay([this, charger_count]() mutable {
        static int i = 0;

        if (i >= charger_count)
            i = 0;

        auto &charger_alloc = this->charger_allocation_state[i];
        if(cm_networking.send_manager_update(i, charger_alloc.allocated_current, charger_alloc.cp_disconnect, charger_alloc.allocated_phases))
            ++i;

    }, 0, cm_send_delay);
}

void ChargeManager::setup()
{
    if (!api.restorePersistentConfig("charge_manager/config", &config)) {
#if MODULE_EM_COMMON_AVAILABLE()
        apply_energy_manager_config(config);
#endif
    }

    // We could move this below the enable_charge_manager check, but want to always see
    // the configured values in debug reports even if the charge manager is currently
    // disabled.
    api.restorePersistentConfig("charge_manager/low_level_config", &low_level_config);

    if (!config.get("enable_charge_manager")->asBool() || config.get("chargers")->count() == 0) {
        initialized = true;
        return;
    }
    state.get("state")->updateUint(1);

    this->ca_config = new CurrentAllocatorConfig();

    ca_config->global_hysteresis                    = micros_t{low_level_config.get("global_hysteresis")->asUint()}   * 1_s;
    ca_config->minimum_active_time                  = micros_t{low_level_config.get("min_active_time")->asUint()}     * 1_s;
    ca_config->wakeup_time                          = micros_t{low_level_config.get("wakeup_time")->asUint()}         * 1_s;
    ca_config->plug_in_time                         = micros_t{low_level_config.get("plug_in_time")->asUint()}        * 1_s;
    ca_config->allocation_interval                  = micros_t{low_level_config.get("allocation_interval")->asUint()} * 1_s;
    ca_config->allocated_energy_rotation_threshold  = low_level_config.get("alloc_energy_rot_thres")->asUint();
    ca_config->enable_current_factor                = low_level_config.get("enable_current_factor_pct")->asUint() / 100.0f;

    ca_config->minimum_current_3p = config.get("minimum_current")->asUint();
    ca_config->minimum_current_1p = config.get("minimum_current_1p")->asUint();
    ca_config->requested_current_margin = config.get("requested_current_margin")->asUint();
    ca_config->requested_current_threshold = config.get("requested_current_threshold")->asUint();

    this->ca_state = new CurrentAllocatorState();

    auto default_current = config.get("default_available_current")->asUint();
    available_current.get("current")->updateUint(default_current);
    for (int i = 0; i < config.get("chargers")->count(); ++i) {
        state.get("chargers")->add();
        low_level_state.get("chargers")->add();
        state.get("chargers")->get(i)->get("n")->updateString(config.get("chargers")->get(i)->get("name")->asString());
    }

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
    ca_config->charger_count = this->charger_count;
    this->charger_state = (ChargerState*) heap_caps_calloc_prefer(this->charger_count, sizeof(ChargerState), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    this->charger_allocation_state = (ChargerAllocationState*) heap_caps_calloc_prefer(this->charger_count, sizeof(ChargerAllocationState), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

    for (int i = 0; i < config.get("chargers")->count(); ++i) {
        charger_state[i].phase_rotation = convert_phase_rotation(config.get("chargers")->get(i)->get("rot")->asEnum<CMPhaseRotation>());
    }

    // TODO: Change all currents everywhere to int32_t or int16_t.
    int def_cur = (int) default_current;
    this->limits.raw = {3 * def_cur, def_cur, def_cur, def_cur};
    this->limits.min = {3 * def_cur, def_cur, def_cur, def_cur};
    this->limits.spread = {3 * def_cur, def_cur, def_cur, def_cur};
    this->limits.max_pv = 3 * def_cur;

    start_manager_task();

    auto get_charger_name_fn = [this](uint8_t i){ return this->get_charger_name(i);};
    auto clear_dns_cache_entry_fn = [this](uint8_t i){ return cm_networking.clear_dns_cache_entry(i);};

    this->next_allocation = now_us() + ca_config->allocation_interval;

    task_scheduler.scheduleWithFixedDelay([this, get_charger_name_fn, clear_dns_cache_entry_fn](){
            if (!deadline_elapsed(this->next_allocation))
                return;

            this->next_allocation = now_us() + ca_config->allocation_interval;

            uint32_t allocated_current = 0;

            this->limits_post_allocation = this->limits;

            int result = allocate_current(
                this->ca_config,
                this->seen_all_chargers(),
                &this->limits_post_allocation,
                this->control_pilot_disconnect.get("disconnect")->asBool(),
                this->charger_state,
                this->hosts.get(),
                get_charger_name_fn,
                clear_dns_cache_entry_fn,

                this->ca_state,
                this->charger_allocation_state,
                &allocated_current
            );

            for (size_t i = 0; i < 4; i++) {
                allocated_currents[i] = limits.raw[i] - limits_post_allocation.raw[i];
                this->state.get("l_raw")->get(i)->updateInt(limits.raw[i]);
                this->state.get("l_min")->get(i)->updateInt(limits.min[i]);
                this->state.get("l_spread")->get(i)->updateInt(limits.spread[i]);
                this->state.get("alloc")->get(i)->updateInt(allocated_currents[i]);
                this->low_level_state.get("wnd_min")->get(i)->updateInt(this->ca_state->control_window_min[i]);
                this->low_level_state.get("wnd_max")->get(i)->updateInt(this->ca_state->control_window_max[i]);
            }
            this->state.get("l_max_pv")->updateInt(limits.max_pv);
            this->low_level_state.get("last_hyst_reset")->updateUint(this->ca_state->last_hysteresis_reset.millis());

            for (int i = 0; i < this->charger_count; ++i) {
                update_charger_state_config(i);
            }

            this->state.get("state")->updateUint(result);
        }, 0, 1000);

    if (config.get("enable_watchdog")->asBool()) {
        task_scheduler.scheduleWithFixedDelay([this](){this->check_watchdog();}, 1000, 1000);
    }

    if (config.get("verbose")->asBool()) {
        ca_config->distribution_log = heap_alloc_array<char>(DISTRIBUTION_LOG_LEN);
        ca_config->distribution_log_len = DISTRIBUTION_LOG_LEN;
    } else {
        ca_config->distribution_log = nullptr;
        ca_config->distribution_log_len = 0;
    }

    initialized = true;
}

void ChargeManager::check_watchdog()
{
    if (this->watchdog_triggered || !deadline_elapsed(last_available_current_update + WATCHDOG_TIMEOUT_MS))
        return;

    this->watchdog_triggered = true;

    uint32_t default_available_current = this->default_available_current;

    logger.printfln("Watchdog triggered! Received no available current update for %d ms. Setting available current to %u mA", WATCHDOG_TIMEOUT_MS, default_available_current);

#if MODULE_AUTOMATION_AVAILABLE()
    automation.trigger(AutomationTriggerID::ChargeManagerWd, nullptr, this);
#endif
    this->available_current.get("current")->updateUint(default_available_current);

    last_available_current_update = millis();
}

bool ChargeManager::get_charger_count()
{
    return charger_count;
}

// Check is not 100% reliable after an uptime of 49 days because last_update might legitimately 0.
// Work around that by caching the value once all chargers were seen once.
bool ChargeManager::seen_all_chargers()
{
    if (all_chargers_seen)
        return true;

    // Don't claim to have seen "all" chargers when none are configured.
    if (charger_count == 0)
        return false;

    for (size_t i = 0; i < charger_count; ++i)
        if (this->charger_state[i].last_update == 0)
            return false;

    logger.printfln("Seen all chargers.");

    all_chargers_seen = true;
    return true;
}

bool ChargeManager::is_charging_stopped(uint32_t last_update_cutoff)
{
    for (size_t i = 0; i < charger_count; ++i) {
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
    for (size_t i = 0; i < charger_count; ++i) {
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
    for (size_t i = 0; i < charger_count; ++i) {
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

const char *ChargeManager::get_charger_name(uint8_t idx)
{
    return this->state.get("chargers")->get(idx)->get("n")->asEphemeralCStr();
}

void ChargeManager::register_urls()
{
    bool enabled = config.get("enable_charge_manager")->asBool();

    if (enabled && config.get("chargers")->count() > 0 && this->static_cm) {
        ca_config->global_hysteresis = 0_us;
        ca_config->enable_current_factor = 1;
    }

#if MODULE_AUTOMATION_AVAILABLE() && MODULE_POWER_MANAGER_AVAILABLE() && !MODULE_EM_COMMON_AVAILABLE()
    automation.set_enabled(AutomationTriggerID::ChargeManagerWd, enabled && this->static_cm);
    automation.set_enabled(AutomationActionID::SetManagerCurrent, enabled && this->static_cm);
#endif

    api.addPersistentConfig("charge_manager/config", &config);
    api.addPersistentConfig("charge_manager/low_level_config", &low_level_config);
    api.addState("charge_manager/state", &state);
    api.addState("charge_manager/low_level_state", &low_level_state);

    api.addState("charge_manager/available_current", &available_current);
    api.addCommand("charge_manager/available_current_update", &available_current_update, {}, [this](){
        uint32_t current = this->available_current_update.get("current")->asUint();
        this->available_current.get("current")->updateUint(current);
        this->last_available_current_update = millis();
        this->watchdog_triggered = false;

        for(size_t i = 1; i < 4; ++i) {
            this->limits.raw[i] = current;
            this->limits.min[i] = current;
            this->limits.spread[i] = current;
        }
        this->limits.max_pv = 3 * current; //TODO: unlimited?

    }, false);
}

void ChargeManager::update_charger_state_config(uint8_t idx) {
    auto &charger = charger_state[idx];
    auto &charger_alloc = charger_allocation_state[idx];
    auto *charger_cfg = (Config *)this->state.get("chargers")->get(idx);
    auto *charger_ll_cfg = (Config *)this->low_level_state.get("chargers")->get(idx);
    charger_cfg->get("s")->updateUint(charger_alloc.state);
    charger_cfg->get("e")->updateUint(charger_alloc.error);
    charger_cfg->get("ac")->updateUint(charger_alloc.allocated_current);
    charger_cfg->get("ap")->updateUint(charger_alloc.allocated_phases);
    charger_cfg->get("sc")->updateUint(charger.supported_current);
    charger_cfg->get("sp")->updateUint((charger.phase_switch_supported ? 4 : 0) | (charger.phases));
    charger_cfg->get("lu")->updateUint(charger.last_update);
    charger_cfg->get("u")->updateUint(charger.uid);

    uint8_t bits = (charger.last_alloc_fulfilled_reqd << 5) | (charger.phases << 3) | (charger.phase_switch_supported << 2) | (charger.cp_disconnect_state << 1) | charger.cp_disconnect_supported;
    charger_ll_cfg->get("b")->updateUint(bits);
    charger_ll_cfg->get("rc")->updateUint(charger.requested_current);
    charger_ll_cfg->get("ae")->updateUint(charger.allocated_energy * 1000);
    charger_ll_cfg->get("ar")->updateUint(charger.allocated_energy_this_rotation * 1000);
    charger_ll_cfg->get("ls")->updateUint(charger.last_switch.millis());
    charger_ll_cfg->get("lp")->updateUint(charger.last_plug_in.millis());
    charger_ll_cfg->get("lw")->updateUint(charger.last_wakeup.millis());
    charger_ll_cfg->get("ip")->updateUint(charger.ignore_phase_currents.millis());
}
