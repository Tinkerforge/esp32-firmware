/* esp32-firmware
 * Copyright (C) 2022 Olaf Lüke <olaf@tinkerforge.com>
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

#include <math.h>

#include "config.h"

#include "bindings/bricklet_warp_energy_manager.h"

#include "device_module.h"
#include "em_rgb_led.h"
#include "input_pin.h"
#include "output_relay.h"
#include "warp_energy_manager_bricklet_firmware_bin.embedded.h"

#define EM_TASK_DELAY_MS                    250

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
#define PHASE_SWITCHING_MAX                 3

#define RELAY_CONFIG_MANUAL                 0
#define RELAY_CONFIG_RULE_BASED             1

#define RELAY_CONFIG_WHEN_INPUT3            0
#define RELAY_CONFIG_WHEN_INPUT4            1
#define RELAY_CONFIG_WHEN_PHASE_SWITCHING   2
#define RELAY_CONFIG_WHEN_CONTACTOR_CHECK   3
#define RELAY_CONFIG_WHEN_POWER_AVAILABLE   4
#define RELAY_CONFIG_WHEN_GRID_DRAW         5

#define RELAY_RULE_IS_HIGH                  0
#define RELAY_RULE_IS_LOW                   1
#define RELAY_RULE_IS_1PHASE                2
#define RELAY_RULE_IS_3PHASE                3
#define RELAY_RULE_IS_CONTACTOR_FAIL        4
#define RELAY_RULE_IS_CONTACTOR_OK          5
#define RELAY_RULE_IS_POWER_SUFFIC          6
#define RELAY_RULE_IS_POWER_INSUFFIC        7
#define RELAY_RULE_IS_GT0                   8
#define RELAY_RULE_IS_LE0                   9

#define INPUT_CONFIG_DISABLED               0
#define INPUT_CONFIG_CONTACTOR_CHECK        1
#define INPUT_CONFIG_BLOCK_CHARGING         2
#define INPUT_CONFIG_LIMIT_MAX_CURRENT      3
#define INPUT_CONFIG_SWITCH_MODE            4

#define INPUT_CONFIG_WHEN_HIGH              0
#define INPUT_CONFIG_WHEN_LOW               1

#define HYSTERESIS_MIN_TIME_MINUTES         5

#define ERROR_FLAGS_BAD_CONFIG_BIT_POS      31
#define ERROR_FLAGS_BAD_CONFIG_MASK         (1 << ERROR_FLAGS_BAD_CONFIG_BIT_POS)
#define ERROR_FLAGS_SDCARD_BIT_POS          25
#define ERROR_FLAGS_SDCARD_MASK             (1 << ERROR_FLAGS_SDCARD_BIT_POS)
#define ERROR_FLAGS_BRICKLET_BIT_POS        24
#define ERROR_FLAGS_BRICKLET_MASK           (1 << ERROR_FLAGS_BRICKLET_BIT_POS)
#define ERROR_FLAGS_CONTACTOR_BIT_POS       16
#define ERROR_FLAGS_CONTACTOR_MASK          (1 << ERROR_FLAGS_CONTACTOR_BIT_POS)
#define ERROR_FLAGS_NETWORK_BIT_POS         1
#define ERROR_FLAGS_NETWORK_MASK            (1 << ERROR_FLAGS_NETWORK_BIT_POS)

#define ERROR_FLAGS_ALL_INTERNAL_MASK       (ERROR_FLAGS_SDCARD_MASK | ERROR_FLAGS_BRICKLET_MASK)
#define ERROR_FLAGS_ALL_ERRORS_MASK         (0x7FFF0000)
#define ERROR_FLAGS_ALL_WARNINGS_MASK       (0x0000FFFF)

#define CONFIG_ERROR_FLAGS_NO_CM_BIT_POS            4
#define CONFIG_ERROR_FLAGS_NO_CM_MASK               (1 << CONFIG_ERROR_FLAGS_NO_CM_BIT_POS)
#define CONFIG_ERROR_FLAGS_EXCESS_NO_METER_BIT_POS  3
#define CONFIG_ERROR_FLAGS_EXCESS_NO_METER_MASK     (1 << CONFIG_ERROR_FLAGS_EXCESS_NO_METER_BIT_POS)
#define CONFIG_ERROR_FLAGS_NO_CHARGERS_BIT_POS      2
#define CONFIG_ERROR_FLAGS_NO_CHARGERS_MASK         (1 << CONFIG_ERROR_FLAGS_NO_CHARGERS_BIT_POS)
#define CONFIG_ERROR_FLAGS_NO_MAX_CURRENT_BIT_POS   1
#define CONFIG_ERROR_FLAGS_NO_MAX_CURRENT_MASK      (1 << CONFIG_ERROR_FLAGS_NO_MAX_CURRENT_BIT_POS)
#define CONFIG_ERROR_FLAGS_PHASE_SWITCHING_BIT_POS  0
#define CONFIG_ERROR_FLAGS_PHASE_SWITCHING_MASK     (1 << CONFIG_ERROR_FLAGS_PHASE_SWITCHING_BIT_POS)

#define EXTERNAL_CONTROL_STATE_AVAILABLE    0
#define EXTERNAL_CONTROL_STATE_DISABLED     1
#define EXTERNAL_CONTROL_STATE_UNAVAILABLE  2
#define EXTERNAL_CONTROL_STATE_SWITCHING    3

typedef struct {
    uint32_t last_update;
    bool is_valid;

    bool contactor_value;

    uint8_t rgb_value_r;
    uint8_t rgb_value_g;
    uint8_t rgb_value_b;

    float power;
    float energy_import;
    float energy_export;

    uint8_t energy_meter_type;
    uint32_t error_count[6];

    bool input[2];
    bool output;
    uint16_t voltage;
    uint8_t contactor_check_state;
    uint32_t uptime;
} EnergyManagerAllData;

struct sdcard_info {
    uint32_t sd_status;
    uint32_t lfs_status;
    uint32_t card_type;
    uint32_t sector_count;
    uint16_t sector_size;
    uint8_t  manufacturer_id;
    uint8_t  product_rev;
    char     product_name[6];
} __attribute__((packed));

enum class SwitchingState
{
    Monitoring = 0,
    Stopping,
    DisconnectingCP,
    TogglingContactor,
    ConnectingCP
};

class EnergyManager : public DeviceModule<TF_WARPEnergyManager,
                                          warp_energy_manager_bricklet_firmware_bin_data,
                                          warp_energy_manager_bricklet_firmware_bin_length,
                                          tf_warp_energy_manager_create,
                                          tf_warp_energy_manager_get_bootloader_mode,
                                          tf_warp_energy_manager_reset,
                                          tf_warp_energy_manager_destroy>
{
public:
    EnergyManager() : DeviceModule("energy_manager", "WARP Energy Manager", "Energy Manager", std::bind(&EnergyManager::setup_energy_manager, this)){}
    void pre_setup();
    void setup();
    void register_urls();
    void register_events();
    void loop();

    // Called in energy_manager_meter setup
    void update_all_data();

    void limit_max_current(uint32_t limit_ma);
    void switch_mode(uint32_t new_mode);

    void setup_energy_manager();
    String get_energy_manager_debug_header();
    String get_energy_manager_debug_line();

    void apply_defaults();

    float calculate_cloud_filter_coefficient(uint32_t mode);

    bool get_sdcard_info(struct sdcard_info *data);
    bool format_sdcard();
    uint16_t get_energy_meter_detailed_values(float *ret_values);
    void set_output(bool output);
    void set_rgb_led(uint8_t pattern, uint16_t hue);

    void set_time(const tm &tm);
    struct timeval get_time();

    bool debug = false;

    ConfigRoot state;
    ConfigRoot low_level_state;
    ConfigRoot meter_state;
    ConfigRoot config;
    ConfigRoot config_in_use;
    ConfigRoot debug_config;
    ConfigRoot debug_config_in_use;
    ConfigRoot charge_mode;
    ConfigRoot charge_mode_update;
    ConfigRoot external_control;
    ConfigRoot external_control_update;

    EnergyManagerAllData all_data;

    union {
        uint32_t combined;
        uint8_t  pin[4];
    } charging_blocked               = {0};

    uint32_t error_flags             = 0;
    uint32_t config_error_flags      = 0;
    bool     contactor_check_tripped = false;
    bool     is_3phase               = false;
    bool     wants_on_last           = false;
    float    power_at_meter_w        = NAN;

private:
    void update_status_led();
    void clr_error(uint32_t error_mask);
    void set_error(uint32_t error_mask);
    bool is_error(uint32_t error_bit_pos);
    void set_config_error(uint32_t config_error_mask);
    void check_bricklet_reachable(int rc, const char *context);
    void update_all_data_struct();
    void update_io();
    void update_energy();

    void start_network_check_task();
    void start_auto_reset_task();
    void schedule_auto_reset_task();
    void set_available_current(uint32_t current);

    void check_debug();
    String prepare_fmtstr();

    EmRgbLed rgb_led;
    OutputRelay *output;
    InputPin *input3;
    InputPin *input4;

    uint32_t last_debug_check                    = 0;
    bool     printed_not_seen_all_chargers       = false;
    bool     printed_seen_all_chargers           = false;
    bool     uptime_past_hysteresis              = false;
    uint32_t consecutive_bricklet_errors         = 0;
    bool     bricklet_reachable                  = true;
    SwitchingState switching_state               = SwitchingState::Monitoring;
    uint32_t switching_start                     = 0;
    uint32_t mode                                = 0;
    uint8_t  have_phases                         = 0;
    bool     wants_3phase                        = false;
    bool     wants_3phase_last                   = false;
    bool     is_on_last                          = false;
    bool     just_switched_phases                = false;
    bool     just_switched_mode                  = false;
    uint32_t phase_state_change_blocked_until    = 0;
    uint32_t on_state_change_blocked_until       = 0;
    uint32_t charge_manager_allocated_current_ma = 0;
    uint32_t max_current_limited_ma              = 0;
    int32_t  power_available_w                   = 0;
    int32_t  power_available_filtered_w          = 0;
    float    power_at_meter_filtered_w           = NAN;

    // Config cache
    uint32_t default_mode             = 0;
    uint32_t auto_reset_hour          = 0;
    uint32_t auto_reset_minute        = 0;
    bool     excess_charging_enable   = false;
    int32_t  target_power_from_grid_w = 0;
    uint32_t guaranteed_power_w       = 0;
    bool     contactor_installed      = false;
    uint8_t  phase_switching_mode     = 0;
    uint32_t switching_hysteresis_ms  = 0;
    bool     hysteresis_wear_ok       = false;
    uint32_t max_current_unlimited_ma = 0;
    uint32_t min_current_1p_ma        = 0;
    uint32_t min_current_3p_ma        = 0;

    // Pre-calculated limits
    int32_t  overall_min_power_w = 0;
    int32_t  threshold_3to1_w    = 0;
    int32_t  threshold_1to3_w    = 0;

    // Pre-calculated data
    float    cloud_filter_coefficient = 0;

    void update_history_meter_power(float power);
    void collect_data_points();
    bool load_persistent_data();
    void save_persistent_data();
    void history_wallbox_5min_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    void history_wallbox_daily_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    void history_energy_manager_5min_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    void history_energy_manager_daily_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    void set_wallbox_5min_data_point(struct tm *utc, struct tm *local, uint32_t uid, uint8_t flags, uint16_t power /* W */);
    void set_wallbox_daily_data_point(struct tm *local, uint32_t uid, uint32_t energy /* dWh */);
    void set_energy_manager_5min_data_point(struct tm *utc, struct tm *local, uint8_t flags, int32_t power_grid /* W */, int32_t power_general[6] /* W */);
    void set_energy_manager_daily_data_point(struct tm *local, uint32_t energy_grid_in /* dWh */, uint32_t energy_grid_out /* dWh */,
                                             uint32_t energy_general_in[6] /* dWh */, uint32_t energy_general_out[6] /* dWh */);

    bool persistent_data_loaded = false;
    uint32_t last_history_5min_slot = 0;
    uint32_t last_history_daily_slot = 0;
    ConfigRoot history_wallbox_5min;
    ConfigRoot history_wallbox_daily;
    ConfigRoot history_energy_manager_5min;
    ConfigRoot history_energy_manager_daily;
    bool history_meter_available = false;
    float history_meter_power_value = NAN; // W
    uint32_t history_meter_power_timestamp;
    double history_meter_power_sum = 0; // watt seconds
    double history_meter_power_duration = 0; // seconds
    double history_meter_energy_import = 0; // dWh
    double history_meter_energy_export = 0; // dWh
    int32_t history_power_grid = INT32_MAX; // W
};
