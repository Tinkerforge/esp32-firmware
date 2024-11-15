/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "bindings/bricklet_warp_energy_manager_v2.h"
#include "config.h"
#include "device_module.h"
//#include "modules/debug_protocol/debug_protocol_backend.h"
#include "modules/em_common/em_common.h"
#include "modules/em_common/structs.h"

#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

typedef struct {
    EMAllDataCommon common;

    bool input[4];
    bool output_sg_ready[2];
    bool output_relay[2];
} EnergyManagerV2AllData;

class EMV2 final : public DeviceModule<TF_WARPEnergyManagerV2,
                                       tf_warp_energy_manager_v2_create,
                                       tf_warp_energy_manager_v2_get_bootloader_mode,
                                       tf_warp_energy_manager_v2_reset,
                                       tf_warp_energy_manager_v2_destroy>,
                   public IEMBackend
               //, public IDebugProtocolBackend
#if MODULE_AUTOMATION_AVAILABLE()
                 , public IAutomationBackend
#endif
{
public:
    EMV2();

    // for IModule
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

protected:

    // for IEMBackend
    bool is_initialized() const override;

    bool device_module_is_in_bootloader(int rc) override;

    [[gnu::const]] uint32_t get_em_version() const override;
    [[gnu::const]] const EMAllDataCommon *get_all_data_common() const override;

    void get_input_output_states(bool *inputs, size_t *inputs_len, bool *outputs, size_t *outputs_len) const override;

    int wem_register_sd_wallbox_data_points_low_level_callback(WEM_SDWallboxDataPointsLowLevelHandler handler, void *user_data) override;
    int wem_register_sd_wallbox_daily_data_points_low_level_callback(WEM_SDWallboxDailyDataPointsLowLevelHandler handler, void *user_data) override;
    int wem_register_sd_energy_manager_data_points_low_level_callback(WEM_SDEnergyManagerDataPointsLowLevelHandler handler, void *user_data) override;
    int wem_register_sd_energy_manager_daily_data_points_low_level_callback(WEM_SDEnergyManagerDailyDataPointsLowLevelHandler handler, void *user_data) override;
    int wem_get_sd_information(uint32_t *ret_sd_status, uint32_t *ret_lfs_status, uint16_t *ret_sector_size, uint32_t *ret_sector_count, uint32_t *ret_card_type, uint8_t *ret_product_rev, char ret_product_name[5], uint8_t *ret_manufacturer_id) override;
    int wem_set_sd_wallbox_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t flags, uint16_t power, uint8_t *ret_status) override;
    int wem_get_sd_wallbox_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status) override;
    int wem_set_sd_wallbox_daily_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint32_t energy, uint8_t *ret_status) override;
    int wem_get_sd_wallbox_daily_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status) override;
    int wem_set_sd_energy_manager_data_point(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t flags, int32_t power_grid, const int32_t power_general[6], uint32_t price, uint8_t *ret_status) override;
    int wem_get_sd_energy_manager_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status) override;
    int wem_set_sd_energy_manager_daily_data_point(uint8_t year, uint8_t month, uint8_t day, uint32_t energy_grid_in, uint32_t energy_grid_out, const uint32_t energy_general_in[6], const uint32_t energy_general_out[6], uint32_t price, uint8_t *ret_status) override;
    int wem_get_sd_energy_manager_daily_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status) override;
    int wem_format_sd(uint32_t password, uint8_t *ret_format_status) override;
    int wem_set_date_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t days_of_week, uint8_t month, uint16_t year) override;
    int wem_get_date_time(uint8_t *ret_seconds, uint8_t *ret_minutes, uint8_t *ret_hours, uint8_t *ret_days, uint8_t *ret_days_of_week, uint8_t *ret_month, uint16_t *ret_year) override;
    int wem_set_data_storage(uint8_t page, const uint8_t data[63]) override;
    int wem_get_data_storage(uint8_t page, uint8_t *status, uint8_t ret_data[63]) override;
    int wem_reset_energy_meter_relative_energy() override;
    int wem_get_energy_meter_detailed_values(float *ret_values, uint16_t *ret_values_length) override;

public:

    void setup_energy_manager();
    //[[gnu::const]] size_t get_debug_header_length() const override;
    //void get_debug_header(StringBuilder *sb) override;
    //[[gnu::const]] size_t get_debug_line_length() const override;
    //void get_debug_line(StringBuilder *sb) override;

    bool get_input(uint32_t index);
    void set_sg_ready_output(uint32_t index, bool value);
    bool get_sg_ready_output(uint32_t index);
    void set_relay_output(uint32_t index, bool value);
    bool get_relay_output(uint32_t index);

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

private:
    template<typename T>
    void update_all_data_triggers(T id, void *data);
    void update_all_data();
    void update_all_data_struct();

    EnergyManagerV2AllData all_data;

    ConfigRoot outputs_update;
};

#include "module_available_end.h"
