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

#pragma once

#include "phase_switcher_back-end.h"

#include "config.h"
#include "module.h"

#define PM_TASK_DELAY_MS                    250
#define CURRENT_POWER_SMOOTHING_SAMPLES     4

#define MODE_FAST                           0
#define MODE_OFF                            1
#define MODE_PV                             2
#define MODE_MIN_PV                         3
#define MODE_DO_NOTHING                     255

#define CLOUD_FILTER_OFF                    0
#define CLOUD_FILTER_LIGHT                  1
#define CLOUD_FILTER_MEDIUM                 2
#define CLOUD_FILTER_STRONG                 3

#define PHASE_SWITCHING_MIN                 0
#define PHASE_SWITCHING_AUTOMATIC           0
#define PHASE_SWITCHING_ALWAYS_1PHASE       1
#define PHASE_SWITCHING_ALWAYS_3PHASE       2
#define PHASE_SWITCHING_EXTERNAL_CONTROL    3
#define PHASE_SWITCHING_PV1P_FAST3P         4
#define PHASE_SWITCHING_MAX                 4

#define EXTERNAL_CONTROL_STATE_AVAILABLE    0
#define EXTERNAL_CONTROL_STATE_DISABLED     1
#define EXTERNAL_CONTROL_STATE_UNAVAILABLE  2
#define EXTERNAL_CONTROL_STATE_SWITCHING    3

#define HYSTERESIS_MIN_TIME_MINUTES         5

#define PM_CONFIG_ERROR_FLAGS_EXCESS_NO_METER_BIT_POS   3
#define PM_CONFIG_ERROR_FLAGS_EXCESS_NO_METER_MASK      (1 << PM_CONFIG_ERROR_FLAGS_EXCESS_NO_METER_BIT_POS)
#define PM_CONFIG_ERROR_FLAGS_NO_CHARGERS_BIT_POS       2
#define PM_CONFIG_ERROR_FLAGS_NO_CHARGERS_MASK          (1 << PM_CONFIG_ERROR_FLAGS_NO_CHARGERS_BIT_POS)
#define PM_CONFIG_ERROR_FLAGS_NO_MAX_CURRENT_BIT_POS    1
#define PM_CONFIG_ERROR_FLAGS_NO_MAX_CURRENT_MASK       (1 << PM_CONFIG_ERROR_FLAGS_NO_MAX_CURRENT_BIT_POS)
#define PM_CONFIG_ERROR_FLAGS_PHASE_SWITCHING_BIT_POS   0
#define PM_CONFIG_ERROR_FLAGS_PHASE_SWITCHING_MASK      (1 << PM_CONFIG_ERROR_FLAGS_PHASE_SWITCHING_BIT_POS)

enum class SwitchingState
{
    Monitoring = 0,
    StartSwitching,
    Stopping,
    DisconnectingCP,
    TogglingContactor,
    WaitUntilSwitched,
};

class PowerManager final : public IModule
{
    friend class EnergyManager;

public:
    PowerManager() {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void register_phase_switcher_backend(PhaseSwitcherBackend *backend);

    bool get_enabled() const;
    bool get_is_3phase() const;

private:
    class PhaseSwitcherBackendDummy final : public PhaseSwitcherBackend
    {
        bool phase_switching_capable()               override {return false;}
        bool can_switch_phases_now(bool wants_3phase) override {return false;}
        bool requires_cp_disconnect()                override {return true;}
        bool get_is_3phase()                         override {return false;}
        SwitchingState get_phase_switching_state()   override {return SwitchingState::Error;}
        bool switch_phases_3phase(bool wants_3phase) override {return false;}
    };

    enum class TristateBool : uint8_t {
        False = 0,
        True = 1,
        Undefined = 2,
    };

    void set_available_current(uint32_t current);
    void set_available_phases(uint32_t phases);
    void update_data();
    void update_energy();
    void limit_max_current(uint32_t limit_ma);
    void reset_limit_max_current();
    void set_config_error(uint32_t config_error_mask);

    bool action_triggered(const Config *config, void *data);

    ConfigRoot state;
    ConfigRoot low_level_state;
    ConfigRoot config;
    ConfigRoot debug_config;
    ConfigRoot charge_mode;
    ConfigRoot charge_mode_update;
    ConfigRoot external_control;
    ConfigRoot external_control_update;

    uint32_t config_error_flags = 0;

    PhaseSwitcherBackendDummy phase_switcher_dummy = PhaseSwitcherBackendDummy();
    PhaseSwitcherBackend *phase_switcher_backend = &phase_switcher_dummy;

    bool     printed_not_seen_all_chargers       = false;
    bool     printed_seen_all_chargers           = false;
    bool     printed_skipping_energy_update      = false;
    bool     uptime_past_hysteresis              = false;

    SwitchingState switching_state               = SwitchingState::Monitoring;
    SwitchingState switching_state_prev          = switching_state;
    uint32_t switching_start                     = 0;
    uint32_t mode                                = 0;
    uint32_t have_phases                         = 0;
    bool     is_3phase                           = false;
    bool     wants_3phase                        = false;
    bool     wants_3phase_last                   = false;
    bool     is_on_last                          = false;
    bool     wants_on_last                       = false;
    bool     just_switched_phases                = false;
    bool     just_switched_mode                  = false;
    uint32_t phase_state_change_blocked_until    = 0;
    uint32_t on_state_change_blocked_until       = 0;
    uint32_t charge_manager_available_current_ma = 0;
    uint32_t charge_manager_allocated_current_ma = 0;
    uint32_t max_current_limited_ma              = 0;

    union {
        uint32_t combined;
        uint8_t  pin[4];
    } charging_blocked               = {0};

    int32_t  power_available_w                   = 0;
    int32_t  power_available_filtered_w          = 0;

    float    power_at_meter_raw_w                = NAN;

    int32_t  power_at_meter_smooth_w             = INT32_MAX;
    int32_t  power_at_meter_smooth_values_w[CURRENT_POWER_SMOOTHING_SAMPLES];
    int32_t  power_at_meter_smooth_total         = 0;
    int32_t  power_at_meter_smooth_position      = 0;

    int32_t  power_at_meter_filtered_w           = INT32_MAX;
    int32_t *power_at_meter_mavg_values_w        = nullptr;
    int32_t  power_at_meter_mavg_total           = 0;
    int32_t  power_at_meter_mavg_values_count    = 0;
    int32_t  power_at_meter_mavg_position        = 0;

    // Config cache
    uint32_t default_mode             = 0;
    bool     excess_charging_enable   = false;
    uint32_t meter_slot_power         = UINT32_MAX;
    int32_t  target_power_from_grid_w = 0;
    uint32_t guaranteed_power_w       = 0;
    uint32_t phase_switching_mode     = 0;
    uint32_t switching_hysteresis_ms  = 0;
    bool     hysteresis_wear_ok       = false;
    uint32_t max_current_unlimited_ma = 0;
    uint32_t min_current_1p_ma        = 0;
    uint32_t min_current_3p_ma        = 0;

    // Pre-calculated limits
    int32_t  overall_min_power_w = 0;
    int32_t  threshold_3to1_w    = 0;
    int32_t  threshold_1to3_w    = 0;
    uint32_t max_phases          = 0;

    // Automation
    TristateBool automation_drawing_power_last   = TristateBool::Undefined;
    TristateBool automation_power_available_last = TristateBool::Undefined;
};
