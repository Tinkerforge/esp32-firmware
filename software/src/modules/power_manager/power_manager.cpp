/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "power_manager.h"
#include "module_dependencies.h"

#include "api.h"
#include "event_log.h"

#include "gcc_warnings.h"

void PowerManager::pre_setup()
{
    // States
    state = Config::Object({
        {"config_error_flags", Config::Uint32(0)},
        {"external_control", Config::Uint32(EXTERNAL_CONTROL_STATE_DISABLED)},
    });

    low_level_state = Config::Object({
        {"power_at_meter", Config::Float(0)},
        {"power_at_meter_filtered", Config::Float(0)}, //TODO make this int?
        {"power_available", Config::Int32(0)},
        {"power_available_filtered", Config::Int32(0)},
        {"overall_min_power", Config::Int32(0)},
        {"threshold_3to1", Config::Int32(0)},
        {"threshold_1to3", Config::Int32(0)},
        {"charge_manager_available_current", Config::Uint32(0)},
        {"charge_manager_allocated_current", Config::Uint32(0)},
        {"max_current_limited", Config::Uint32(0)},
        {"uptime_past_hysteresis", Config::Bool(false)},
        {"is_3phase", Config::Bool(false)},
        {"wants_3phase", Config::Bool(false)},
        {"wants_3phase_last", Config::Bool(false)},
        {"is_on_last", Config::Bool(false)},
        {"wants_on_last", Config::Bool(false)},
        {"phase_state_change_blocked", Config::Bool(false)},
        {"phase_state_change_delay", Config::Uint32(0)},
        {"on_state_change_blocked", Config::Bool(false)},
        {"on_state_change_delay", Config::Uint32(0)},
        {"charging_blocked", Config::Uint32(0)},
        {"switching_state", Config::Uint32(0)},
    });

    config = ConfigRoot(Config::Object({
        {"phase_switching_mode", Config::Uint(PHASE_SWITCHING_AUTOMATIC, PHASE_SWITCHING_MIN, PHASE_SWITCHING_MAX)},
        {"excess_charging_enable", Config::Bool(false)},
        {"default_mode", Config::Uint(0, 0, 3)},
        {"meter_slot_grid_power", Config::Uint(0, 0, METERS_SLOTS - 1)},
        {"target_power_from_grid", Config::Int32(0)}, // in watt
        {"guaranteed_power", Config::Uint(1380, 0, 22080)}, // in watt
        {"cloud_filter_mode", Config::Uint(CLOUD_FILTER_MEDIUM, CLOUD_FILTER_OFF, CLOUD_FILTER_STRONG)},
    }), [](const Config &cfg, ConfigSource source) -> String {
        //const Config *em_cfg = energy_manager.get_config();

        if (cfg.get("phase_switching_mode")->asUint() == 3) { // external control
            //if (em_cfg->get("contactor_installed")->asBool() != true)
            //    return "Can't enable external control with no contactor installed.";
            if (cfg.get("excess_charging_enable")->asBool() != false)
                return "Can't enable excess charging when external control is enabled for the Energy Manager's phase switching.";
            if (cfg.get("default_mode")->asUint() != MODE_FAST)
                return "Can't select any charging mode besides 'Fast' when extrenal control is enabled for the Energy Manager's phase switching.";
        }

        return "";
    });

    debug_config = Config::Object({
        {"hysteresis_time", Config::Uint(HYSTERESIS_MIN_TIME_MINUTES, 0, 60)}, // in minutes
    });

    // Runtime config
    charge_mode = Config::Object({
        {"mode", Config::Uint(0, 0, 3)},
    });
    charge_mode_update = charge_mode;

    external_control = Config::Object({
        {"phases_wanted", Config::Uint32(0)},
    });
    external_control_update = external_control;

}

void PowerManager::setup()
{
    initialized = true;

    api.restorePersistentConfig("power_manager/config", &config);

#if MODULE_DEBUG_AVAILABLE()
    api.restorePersistentConfig("power_manager/debug_config", &debug_config);
#endif
}

void PowerManager::register_urls()
{
    api.addState("power_manager/state", &state);

    api.addPersistentConfig("power_manager/config", &config);
#if MODULE_DEBUG_AVAILABLE()
    api.addPersistentConfig("power_manager/debug_config", &debug_config);
#endif

    api.addState("power_manager/low_level_state", &low_level_state);

    api.addState("power_manager/charge_mode", &charge_mode);
    api.addCommand("power_manager/charge_mode_update", &charge_mode_update, {}, [this](){
        energy_manager.update_charge_mode(charge_mode_update);
    }, false);

    api.addState("power_manager/external_control", &external_control);
    api.addCommand("power_manager/external_control_update", &external_control_update, {}, [this](){
        uint32_t external_control_state = state.get("external_control")->asUint();
        switch (external_control_state) {
            case EXTERNAL_CONTROL_STATE_DISABLED:
                logger.printfln("power_manager: Ignoring external control phase change request: External control is not enabled.");
                return;
            case EXTERNAL_CONTROL_STATE_UNAVAILABLE:
                logger.printfln("power_manager: Ignoring external control phase change request: Phase switching is currently unavailable.");
                return;
            case EXTERNAL_CONTROL_STATE_SWITCHING:
                logger.printfln("power_manager: Ignoring external control phase change request: Phase switching in progress.");
                return;
            default:
                break; // All good, proceed.
        }

        auto phases_wanted = external_control.get("phases_wanted");
        uint32_t old_phases = phases_wanted->asUint();
        uint32_t new_phases = external_control_update.get("phases_wanted")->asUint();

        if (new_phases == old_phases) {
            logger.printfln("power_manager: Ignoring external control phase change request: Value is already %u.", new_phases);
            return;
        }

        if (new_phases == 2 || new_phases > 3) {
            logger.printfln("power_manager: Ignoring external control phase change request: Value %u is invalid.", new_phases);
            return;
        }

        logger.printfln("power_manager: External control phase change request: switching from %u to %u", old_phases, new_phases);
        phases_wanted->updateUint(new_phases);
    }, true);
}

Config * PowerManager::get_state()
{
    return &state;
}

Config * PowerManager::get_config_low_level_state()
{
    return &low_level_state;
}

const Config * PowerManager::get_config()
{
    return &config;
}

const Config * PowerManager::get_debug_config()
{
    return &debug_config;
}

Config * PowerManager::get_config_charge_mode()
{
    return &charge_mode;
}

const Config * PowerManager::get_external_control()
{
    return &external_control;
}

void PowerManager::set_config_error(uint32_t config_error_mask)
{
    config_error_flags |= config_error_mask;
    state.get("config_error_flags")->updateUint(config_error_flags);

    energy_manager.set_error(ERROR_FLAGS_BAD_CONFIG_MASK);
}
