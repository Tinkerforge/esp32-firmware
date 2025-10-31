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

#pragma once

#include "module.h"
#include "config.h"
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

#include "current_limits.h"
#include "modules/cm_networking/config_charge_mode.enum.h"

struct CurrentAllocatorConfig;
struct CurrentAllocatorState;
struct ChargerState;
struct ChargerAllocationState;
struct ChargerDecision;

namespace ChargeMode {
    enum Type {
        _min = 1,
        PV = 1,
        Min = 2,
        Eco = 4,
        Fast = 8,
        _max = 8
    };
    const char * const Strings[Type::_max + 1] {
        "Off",
        "PV",
        "Min",
        "Min+PV",
        "Eco",
        "Eco+PV",
        "Eco+Min",
        "Eco+Min+PV",
        "Fast"
    };
}

class ChargeManager final : public IModule
#if MODULE_AUTOMATION_AVAILABLE()
                          , public IAutomationBackend
#endif
{
public:
    ChargeManager(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

    size_t get_charger_count();

    const String &get_charger_host(uint8_t idx);
    const char *get_charger_name(uint8_t idx);
    bool is_only_proxy() { return this->config.get("chargers")->count() == 1 && !this->config.get("enable_charge_manager")->asBool(); }

    uint32_t get_maximum_available_current();

    CurrentLimits *get_limits() {
        // TODO: Maybe add separate function for this?
        static_cm = false;
        return &limits;
    }
    const Cost *get_allocated_currents() {return &allocated_currents;}

    void trigger_allocator_run() {next_allocation = 0_us;}
    void skip_global_hysteresis();
    void enable_fast_single_charger_mode();

    const ChargerState *get_charger_state(uint8_t idx);
    ChargerState *get_mutable_charger_state(uint8_t idx);

    size_t trace_buffer_index;

    uint8_t config_cm_to_cm(ConfigChargeMode power_manager_charge_mode);
    ConfigChargeMode cm_to_config_cm(uint8_t mode);

private:
    bool seen_all_chargers();
    void start_manager_task();
    void check_watchdog();

    void update_charger_state_config(uint8_t idx);
    void update_charger_state_from_mode(ChargerState *state, int charger_idx);

    void update_supported_charge_modes(bool pv, bool eco);

    /*
        Charge modes are confusing.

        Currently the situation is as follows:
        - power_manager/charge_mode is the global charge mode, that is applied to all chargers when updated.
          This API is only used by the charge manager. It is registered unter power_manager for legacy reasons.

        - charge_manager/charge_modes (an array) is the charge mode of each charger.
          Writing this API overrides the charge mode for the specific charger(s).

          Overridden charge modes are changed back to the default (see below)
          when the charge manager sees a X -> 0 transition of the charger state (i.e. a vehicle is unplugged)

          A managed charger can request a charge mode change.
          This is the same as if this charger's value in charge_manager/charge_modes was updated:a charge move override for a single charging session.

        - power_manager/config contains the "default_mode":
          This is the default mode that will be used to de-override a charger's mode.

          The default_mode can be ConfigChargeMode::Default.
          This is used as a marker that power_manager/charge_mode should be persistent.

          If power_manager/charge_mode is persistent, a charger's mode will be set to the **current** value of power_manager/charge_mode when unplugging the vehicle.
          If it is not, the charger's mode will be set to power_manager/config["default_mode"]

        - The currently selected and all supported modes are sent to managed chargers (see cm_networking)
          The charger responds with a requested charge mode override or ConfigChargeMode::Default if it does not want to request an override.

          If the charge manager restarts, it sends ConfigChargeMode::Default to all managed chargers.
          The managed charger than responds as if it wants to override the mode.
          This allows the manager to recover overridden charge modes of managed chargers from before the reboot.

          If a managed charger restarts, the (possibly overridden) charge mode is sent by the charge manager, so this recovers automagically.

          If the charge manager and a managed charger restart at the same time
          (power outage or if the charge manager is a charger itself)
          an overridden charge mode is lost and the default mode will be used once again.
          This also happens if a charger does not respond to charge manager messages for more than 30 seconds after a charge manager reboot.
    */
    ConfigChargeMode get_default_mode();

    size_t charger_count = 0;

    micros_t last_available_current_update = 0_us;

    ChargerState *charger_state = nullptr;

    ConfigRoot config;
    ConfigRoot low_level_config;

    ConfigRoot state;
    ConfigRoot low_level_state;

    ConfigRoot available_current;
    ConfigRoot available_current_update;

    ConfigRoot control_pilot_disconnect;

    Config config_chargers_prototype;
    Config state_chargers_prototype;
    Config low_level_state_chargers_prototype;

    ConfigRoot charge_mode;
    ConfigRoot pm_charge_mode;
    ConfigRoot pm_charge_mode_update;
    ConfigRoot supported_charge_modes;

#ifdef DEBUG_FS_ENABLE
    ConfigRoot debug_limits_update;
#endif

    CurrentLimits limits, limits_post_allocation;
    Cost allocated_currents;

    micros_t next_allocation = 0_us;
    bool static_cm = true;

    bool all_chargers_seen = false;

    bool watchdog_triggered = false;

    std::unique_ptr<const char *[]> hosts;
    uint16_t requested_current_threshold;
    uint16_t requested_current_margin;

    uint32_t guaranteed_pv_current;
    ConfigChargeMode pm_default_charge_mode;

    ChargerAllocationState *charger_allocation_state = nullptr;
    CurrentAllocatorConfig *ca_config = nullptr;
    CurrentAllocatorState *ca_state = nullptr;

    ChargerDecision *charger_decisions = nullptr;

    std::array<uint8_t, 2> supported_charge_mode_bitmask;
};

#include "module_available_end.h"
