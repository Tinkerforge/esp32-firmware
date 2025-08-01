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


#include "module.h"
#include "config.h"
#include "phase_switcher_back-end.h"
#include "battery_mode.enum.h"
#include "modules/debug_protocol/debug_protocol_backend.h"
#include "modules/charge_manager/current_limits.h"
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

#include "modules/charge_manager/config_charge_mode.enum.h"

#define PM_TASK_DELAY_MS                    500

#define CLOUD_FILTER_OFF                    0
#define CLOUD_FILTER_LIGHT                  1
#define CLOUD_FILTER_MEDIUM                 2
#define CLOUD_FILTER_STRONG                 3

#define PHASE_SWITCHING_MIN                 0
#define PHASE_SWITCHING_AUTOMATIC           0
#define PHASE_SWITCHING_EXTERNAL_CONTROL    3
#define PHASE_SWITCHING_MAX                 4

#define EXTERNAL_CONTROL_STATE_AVAILABLE    0
#define EXTERNAL_CONTROL_STATE_DISABLED     1
#define EXTERNAL_CONTROL_STATE_UNAVAILABLE  2
#define EXTERNAL_CONTROL_STATE_SWITCHING    3

#define PM_CONFIG_ERROR_FLAGS_DLM_NO_METER_BIT_POS      4
#define PM_CONFIG_ERROR_FLAGS_DLM_NO_METER_MASK         (1 << PM_CONFIG_ERROR_FLAGS_DLM_NO_METER_BIT_POS)
#define PM_CONFIG_ERROR_FLAGS_EXCESS_NO_METER_BIT_POS   3
#define PM_CONFIG_ERROR_FLAGS_EXCESS_NO_METER_MASK      (1 << PM_CONFIG_ERROR_FLAGS_EXCESS_NO_METER_BIT_POS)
#define PM_CONFIG_ERROR_FLAGS_NO_CHARGERS_BIT_POS       2
#define PM_CONFIG_ERROR_FLAGS_NO_CHARGERS_MASK          (1 << PM_CONFIG_ERROR_FLAGS_NO_CHARGERS_BIT_POS)
#define PM_CONFIG_ERROR_FLAGS_NO_MAX_CURRENT_BIT_POS    1
#define PM_CONFIG_ERROR_FLAGS_NO_MAX_CURRENT_MASK       (1 << PM_CONFIG_ERROR_FLAGS_NO_MAX_CURRENT_BIT_POS)
#define PM_CONFIG_ERROR_FLAGS_PHASE_SWITCHING_BIT_POS   0
#define PM_CONFIG_ERROR_FLAGS_PHASE_SWITCHING_MASK      (1 << PM_CONFIG_ERROR_FLAGS_PHASE_SWITCHING_BIT_POS)

class PowerManager final : public IModule,
                           public IDebugProtocolBackend
#if MODULE_AUTOMATION_AVAILABLE()
                         , public IAutomationBackend
#endif
{
public:
    PowerManager() {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void register_phase_switcher_backend(PhaseSwitcherBackend *backend);

    bool get_enabled() const;
    ConfigChargeMode get_default_charge_mode() const;
    uint32_t get_guaranteed_power_w() const;
    uint32_t get_phase_switching_mode() const;
    uint32_t get_phases() const;

    [[gnu::const]] size_t get_debug_header_length() const override;
    void get_debug_header(StringBuilder *sb) override;
    [[gnu::const]] size_t get_debug_line_length() const override;
    void get_debug_line(StringBuilder *sb) override;

    void print_trace_header() const;

    enum class FilterType : uint8_t {
        MinOnly = 0,
        MaxOnly = 1,
        MinMax  = 2,
    };

    struct minmax_filter {
        int32_t  min             = INT32_MAX;
        int32_t  max             = INT32_MAX;
        int32_t *history_values  = nullptr;
        int32_t  history_length  = 0;
        int32_t  history_pos     = 0;
        int32_t  history_min_pos = 0;
        int32_t  history_max_pos = 0;
        FilterType type          = FilterType::MinOnly;
    };

    struct mavg_filter {
        int32_t  filtered_val       = INT32_MAX;
        int32_t *mavg_values        = nullptr;
        int32_t  mavg_total         = 0;
        int32_t  mavg_values_count  = 0;
        int32_t  mavg_position      = 0;
    };

private:
    class PhaseSwitcherBackendDummy final : public PhaseSwitcherBackend
    {
        uint32_t get_phase_switcher_priority()             override {return 0;}
        bool phase_switching_capable()                     override {return false;}
        bool can_switch_phases_now(uint32_t phases_wanted) override {return false;}
        uint32_t get_phases()                              override {return 0;}
        SwitchingState get_phase_switching_state()         override {return SwitchingState::Ready;} // Don't report an error when phase_switching_capable() is false.
        bool switch_phases(uint32_t phases_wanted)         override {return false;}
        bool is_external_control_allowed()                 override {return false;}
    };

    enum class TristateBool : uint8_t {
        False = 0,
        True = 1,
        Undefined = 2,
    };

    void zero_limits();
    void update_data();
    void update_energy();
    void update_phase_switcher();
    void set_max_current_limit(int32_t limit_ma);
    void reset_max_current_limit();
    void set_config_error(uint32_t config_error_mask);
    const char *prepare_fmtstr();

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

    ConfigRoot state;
    ConfigRoot low_level_state;
    ConfigRoot config;
    ConfigRoot dynamic_load_config;
    ConfigRoot external_control;
    ConfigRoot external_control_update;

    uint32_t config_error_flags = 0;

    PhaseSwitcherBackendDummy phase_switcher_dummy = PhaseSwitcherBackendDummy();
    PhaseSwitcherBackend *phase_switcher_backend = &phase_switcher_dummy;

    bool     printed_skipping_energy_update      = false;
    bool     printed_skipping_currents_update    = false;

    int32_t max_current_limited_ma               = 0;

    union {
        uint32_t combined;
        uint8_t  pin[4];
    } charging_blocked               = {0};

    int32_t  power_available_w = 0;

    size_t current_long_min_iterations = 0;

    float    power_at_meter_raw_w = NAN;
    float    power_at_battery_raw_w = NAN;
    float    battery_soc_raw = NAN;
    int32_t  current_pv_floating_min_ma = INT32_MAX;
    minmax_filter current_pv_minmax_ma;
    minmax_filter current_pv_long_min_ma;

    // Raw currents measured by meter
    int32_t currents_at_meter_raw_ma[3] = {INT32_MAX, INT32_MAX, INT32_MAX};
    // Preprocessor for meter values
    int32_t       currents_phase_preproc_mavg_limit;
    int32_t       currents_phase_preproc_interpolate_limit;
    int32_t       currents_phase_preproc_interpolate_interval_quantized;
    // 1kA = 20 bits; interval_*_quantized max 11 bits; max interval = 18 bits -> quantization factor must save 7 bits
    const int32_t currents_phase_preproc_interpolate_quantization_factor = 128;
    minmax_filter currents_phase_preproc_max_ma[3];
    mavg_filter   currents_phase_preproc_mavg_ma[3];
    // Short filter (4min)
    minmax_filter currents_phase_min_ma[3];
    // Long filter (1h)
    int32_t currents_phase_floating_min_ma[3] = {INT32_MAX, INT32_MAX, INT32_MAX};
    minmax_filter currents_phase_long_min_ma[3];

    // CM data
    CurrentLimits *cm_limits;
    const Cost *cm_allocated_currents;
    micros_t cm_allocator_trigger_hysteresis = 0_us;

    // Config cache
    bool     excess_charging_enabled  = false;
    uint32_t meter_slot_power         = UINT32_MAX;
    uint32_t meter_slot_battery_power = UINT32_MAX;
    BatteryMode battery_mode          = BatteryMode::PreferChargers;
    uint8_t  battery_target_soc       = 0;
    bool     battery_inverted         = false;
    uint16_t battery_deadzone_w       = 0;
    int32_t  target_power_from_grid_w = 0;
    uint32_t phase_switching_mode     = 0;
    bool     dynamic_load_enabled     = false;
    uint32_t meter_slot_currents      = UINT32_MAX;
    int32_t  supply_cable_max_current_ma = 0;

    // Pre-calculated limits
    int32_t  target_phase_current_ma = 0;
    int32_t  phase_current_max_increase_ma = 0;
    bool     have_battery = false;

    // Automation
    TristateBool automation_drawing_power_last   = TristateBool::Undefined;

    // Tracing
    const char *trace_header;
    size_t trace_header_length;
};

#include "module_available_end.h"
