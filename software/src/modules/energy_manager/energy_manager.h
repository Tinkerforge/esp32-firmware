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
#include <list>

#include "config.h"

#include "bindings/bricklet_warp_energy_manager.h"
#include "modules/power_manager/phase_switcher_back-end.h"

#include "device_module.h"
#include "em_rgb_led.h"
#include "structs.h"
#include "warp_energy_manager_bricklet_firmware_bin.embedded.h"

#define EM_TASK_DELAY_MS                    250

#define ERROR_FLAGS_BAD_CONFIG_BIT_POS      31
#define ERROR_FLAGS_BAD_CONFIG_MASK         (1u<< ERROR_FLAGS_BAD_CONFIG_BIT_POS)
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

class EnergyManager final : public PhaseSwitcherBackend, public DeviceModule<TF_WARPEnergyManager,
                                          warp_energy_manager_bricklet_firmware_bin_data,
                                          warp_energy_manager_bricklet_firmware_bin_length,
                                          tf_warp_energy_manager_create,
                                          tf_warp_energy_manager_get_bootloader_mode,
                                          tf_warp_energy_manager_reset,
                                          tf_warp_energy_manager_destroy>
{
public:
    EnergyManager() : DeviceModule("energy_manager", "WARP Energy Manager", "Energy Manager", [this](){this->setup_energy_manager();}) {}

    // for IModule
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;
    void loop() override;

    // for PhaseSwitcherBackend
    bool phase_switching_capable() override;
    bool can_switch_phases_now(bool wants_3phase) override;
    bool requires_cp_disconnect() override {return true;}
    bool get_is_3phase() override;
    PhaseSwitcherBackend::SwitchingState get_phase_switching_state() override;
    bool switch_phases_3phase(bool wants_3phase) override;

    void set_error(uint32_t error_mask);

    void setup_energy_manager();
    String get_energy_manager_debug_header();
    String get_energy_manager_debug_line();

    bool get_sdcard_info(struct sdcard_info *data);
    bool format_sdcard();
    uint16_t get_energy_meter_detailed_values(float *ret_values);
    bool reset_energy_meter_relative_energy();
    void set_output(bool output);
    void set_rgb_led(uint8_t pattern, uint16_t hue);

    void set_time(const tm &tm);
    struct timeval get_time();

    void update_grid_balance_led(EmRgbLed::GridBalance balance);

    bool disallow_fw_update_with_vehicle_connected();

private:
    void update_status_led();
    void clr_error(uint32_t error_mask);
    bool is_error(uint32_t error_bit_pos);
    void set_config_error(uint32_t config_error_mask);
    void check_bricklet_reachable(int rc, const char *context);

    template<typename T>
    void update_all_data_triggers(T id, void *data);
    void update_all_data();
    void update_all_data_struct();

    void start_network_check_task();

    bool action_triggered(const Config *config, void *data);
    void check_debug();
    String prepare_fmtstr();

    ConfigRoot state;
    ConfigRoot low_level_state;
    ConfigRoot config;

    EnergyManagerAllData all_data;

    EmRgbLed rgb_led;

    uint32_t error_flags        = 0;
    uint32_t config_error_flags = 0;

    uint32_t last_debug_keep_alive               = 0;
    bool     debug                               = false;
    bool     contactor_check_tripped             = false;
    micros_t phase_switch_deadtime_us            = 0_usec;
    bool     bricklet_reachable                  = true;
    uint32_t consecutive_bricklet_errors         = 0;

    // Config cache
    bool     contactor_installed      = false;

    void update_history_meter_power(uint32_t slot, float power /* W */);
    void collect_data_points();
    void set_pending_data_points();
    bool load_persistent_data();
    void load_persistent_data_v1(uint8_t *buf);
    void load_persistent_data_v2(uint8_t *buf);
    void save_persistent_data();
    void history_wallbox_5min_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    void history_wallbox_daily_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    void history_energy_manager_5min_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    void history_energy_manager_daily_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    bool set_wallbox_5min_data_point(const struct tm *utc, const struct tm *local, uint32_t uid, uint8_t flags, uint16_t power /* W */);
    bool set_wallbox_daily_data_point(const struct tm *local, uint32_t uid, uint32_t energy /* daWh */);
    bool set_energy_manager_5min_data_point(const struct tm *utc, const struct tm *local, uint8_t flags, const int32_t power[7] /* W */);
    bool set_energy_manager_daily_data_point(const struct tm *local, const uint32_t energy_import[7] /* daWh */, const uint32_t energy_export[7] /* daWh */);

    std::list<std::function<bool(void)>> pending_data_points;
    bool persistent_data_loaded = false;
    bool show_blank_value_id_update_warnings = false;
    uint32_t last_history_5min_slot = 0;
    ConfigRoot history_wallbox_5min;
    ConfigRoot history_wallbox_daily;
    ConfigRoot history_energy_manager_5min;
    ConfigRoot history_energy_manager_daily;
    bool history_meter_setup_done[METERS_SLOTS];
    float history_meter_power_value[METERS_SLOTS]; // W
    uint32_t history_meter_power_timestamp[METERS_SLOTS];
    double history_meter_power_sum[METERS_SLOTS] = {0}; // watt seconds
    double history_meter_power_duration[METERS_SLOTS] = {0}; // seconds
    double history_meter_energy_import[METERS_SLOTS] = {0}; // daWh
    double history_meter_energy_export[METERS_SLOTS] = {0}; // daWh
};
