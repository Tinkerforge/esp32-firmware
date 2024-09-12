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

#include <stdint.h>
#include <time.h>

#include "module_available.h"
#include "structs.h"

#include "config.h"
#include "module.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

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

class IEMBackend
{
    friend class EMCommon;

protected:
    IEMBackend() {}
    virtual ~IEMBackend() {}

    virtual bool is_initialized() const = 0;

    // Pass through to DeviceModule if used
    //virtual bool setup_device() = 0;
    virtual bool device_module_is_in_bootloader(int rc) = 0;

    virtual uint32_t get_em_version() const = 0;
    virtual const EMAllDataCommon *get_all_data_common() const = 0;

    virtual void get_input_output_states(bool *inputs, size_t *inputs_len, bool *outputs, size_t *outputs_len) const = 0;

    typedef void (*WEM_SDWallboxDataPointsLowLevelHandler)(void *do_not_use, uint16_t data_length, uint16_t data_chunk_offset, uint8_t data_chunk_data[60], void *user_data);
    typedef void (*WEM_SDWallboxDailyDataPointsLowLevelHandler)(void *do_not_use, uint16_t data_length, uint16_t data_chunk_offset, uint32_t data_chunk_data[15], void *user_data);
    typedef void (*WEM_SDEnergyManagerDataPointsLowLevelHandler)(void *do_not_use, uint16_t data_length, uint16_t data_chunk_offset, uint8_t data_chunk_data[33], void *user_data);
    typedef void (*WEM_SDEnergyManagerDailyDataPointsLowLevelHandler)(void *do_not_use, uint16_t data_length, uint16_t data_chunk_offset, uint32_t data_chunk_data[15], void *user_data);

    virtual int wem_register_sd_wallbox_data_points_low_level_callback(WEM_SDWallboxDataPointsLowLevelHandler handler, void *user_data) = 0;
    virtual int wem_register_sd_wallbox_daily_data_points_low_level_callback(WEM_SDWallboxDailyDataPointsLowLevelHandler handler, void *user_data) = 0;
    virtual int wem_register_sd_energy_manager_data_points_low_level_callback(WEM_SDEnergyManagerDataPointsLowLevelHandler handler, void *user_data) = 0;
    virtual int wem_register_sd_energy_manager_daily_data_points_low_level_callback(WEM_SDEnergyManagerDailyDataPointsLowLevelHandler handler, void *user_data) = 0;
    virtual int wem_get_sd_information(uint32_t *ret_sd_status, uint32_t *ret_lfs_status, uint16_t *ret_sector_size, uint32_t *ret_sector_count, uint32_t *ret_card_type, uint8_t *ret_product_rev, char ret_product_name[5], uint8_t *ret_manufacturer_id) = 0;
    virtual int wem_set_sd_wallbox_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, uint16_t power, uint8_t *ret_status) = 0;
    virtual int wem_get_sd_wallbox_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status) = 0;
    virtual int wem_set_sd_wallbox_daily_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint32_t energy, uint8_t *ret_status) = 0;
    virtual int wem_get_sd_wallbox_daily_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status) = 0;
    virtual int wem_set_sd_energy_manager_data_point(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, int32_t power_grid, const int32_t power_general[6], uint32_t price, uint8_t *ret_status) = 0;
    virtual int wem_get_sd_energy_manager_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status) = 0;
    virtual int wem_set_sd_energy_manager_daily_data_point(uint8_t year, uint8_t month, uint8_t day, uint32_t energy_grid_in, uint32_t energy_grid_out, const uint32_t energy_general_in[6], const uint32_t energy_general_out[6], uint32_t price, uint8_t *ret_status) = 0;
    virtual int wem_get_sd_energy_manager_daily_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status) = 0;
    virtual int wem_format_sd(uint32_t password, uint8_t *ret_format_status) = 0;
    virtual int wem_set_date_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t days_of_week, uint8_t month, uint16_t year) = 0;
    virtual int wem_get_date_time(uint8_t *ret_seconds, uint8_t *ret_minutes, uint8_t *ret_hours, uint8_t *ret_days, uint8_t *ret_days_of_week, uint8_t *ret_month, uint16_t *ret_year) = 0;
    virtual int wem_set_data_storage(uint8_t page, const uint8_t data[63]) = 0;
    virtual int wem_get_data_storage(uint8_t page, uint8_t ret_data[63]) = 0;
    virtual int wem_reset_energy_meter_relative_energy() = 0;
    virtual int wem_get_energy_meter_detailed_values(float *ret_values, uint16_t *ret_values_length) = 0;
};

class EMCommon final : public IModule
#if MODULE_AUTOMATION_AVAILABLE()
                       , public IAutomationBackend
#endif
{
    // It's a bit ugly that we have to declare all specific EM modules as friends here.
    // But this allows us to make the configs private, to enforce all access happens via the public methods below.
    friend class EMV1;
    friend class EMV2;

public:
    EMCommon();

    //void pre_setup() override;
    void setup() override;
    void register_urls() override;

    inline bool device_module_is_in_bootloader(int rc) {return backend->device_module_is_in_bootloader(rc);}

    inline uint32_t get_em_version() {return backend->get_em_version();}
    inline const EMAllDataCommon *get_all_data_common() {return backend->get_all_data_common();}

    void set_time(const tm &tm);
    timeval get_time();

    bool get_sdcard_info(struct sdcard_info *data);
    bool format_sdcard();

    uint16_t get_energy_meter_detailed_values(float *ret_values);
    bool reset_energy_meter_relative_energy();

    inline void get_input_output_states(bool *inputs, size_t *inputs_len, bool *outputs, size_t *outputs_len) const
    {
        backend->get_input_output_states(inputs, inputs_len, outputs, outputs_len);
    }

    void set_error(uint32_t error_mask);

    inline int wem_register_sd_wallbox_data_points_low_level_callback(IEMBackend::WEM_SDWallboxDataPointsLowLevelHandler handler, void *user_data)
    {
        return backend->wem_register_sd_wallbox_data_points_low_level_callback(handler, user_data);
    }

    inline int wem_register_sd_wallbox_daily_data_points_low_level_callback(IEMBackend::WEM_SDWallboxDailyDataPointsLowLevelHandler handler, void *user_data)
    {
        return backend->wem_register_sd_wallbox_daily_data_points_low_level_callback(handler, user_data);
    }

    inline int wem_register_sd_energy_manager_data_points_low_level_callback(IEMBackend::WEM_SDEnergyManagerDataPointsLowLevelHandler handler, void *user_data)
    {
        return backend->wem_register_sd_energy_manager_data_points_low_level_callback(handler, user_data);
    }

    inline int wem_register_sd_energy_manager_daily_data_points_low_level_callback(IEMBackend::WEM_SDEnergyManagerDailyDataPointsLowLevelHandler handler, void *user_data)
    {
        return backend->wem_register_sd_energy_manager_daily_data_points_low_level_callback(handler, user_data);
    }

    inline int wem_set_sd_wallbox_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, uint16_t power, uint8_t *ret_status)
    {
        int rc = backend->wem_set_sd_wallbox_data_point(wallbox_id, year, month, day, hour, minute, flags, power, ret_status);
        check_bricklet_reachable(rc, "set_sd_wallbox_data_point");
        return rc;
    }

    inline int wem_get_sd_wallbox_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status)
    {
        int rc = backend->wem_get_sd_wallbox_data_points(wallbox_id, year, month, day, hour, minute, amount, ret_status);
        check_bricklet_reachable(rc, "get_sd_wallbox_data_points");
        return rc;
    }

    inline int wem_set_sd_wallbox_daily_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint32_t energy, uint8_t *ret_status)
    {
        int rc = backend->wem_set_sd_wallbox_daily_data_point(wallbox_id, year, month, day, energy, ret_status);
        check_bricklet_reachable(rc, "set_sd_wallbox_daily_data_point");
        return rc;
    }

    inline int wem_get_sd_wallbox_daily_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status)
    {
        int rc = backend->wem_get_sd_wallbox_daily_data_points(wallbox_id, year, month, day, amount, ret_status);
        check_bricklet_reachable(rc, "get_sd_wallbox_daily_data_points");
        return rc;
    }

    inline int wem_set_sd_energy_manager_data_point(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, int32_t power_grid, const int32_t power_general[6], uint32_t price, uint8_t *ret_status)
    {
        int rc = backend->wem_set_sd_energy_manager_data_point(year, month, day, hour, minute, flags, power_grid, power_general, price, ret_status);
        check_bricklet_reachable(rc, "set_sd_energy_manager_data_point");
        return rc;
    }

    inline int wem_get_sd_energy_manager_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status)
    {
        int rc = backend->wem_get_sd_energy_manager_data_points(year, month, day, hour, minute, amount, ret_status);
        check_bricklet_reachable(rc, "get_sd_energy_manager_data_points");
        return rc;
    }

    inline int wem_set_sd_energy_manager_daily_data_point(uint8_t year, uint8_t month, uint8_t day, uint32_t energy_grid_in, uint32_t energy_grid_out, const uint32_t energy_general_in[6], const uint32_t energy_general_out[6], uint32_t price, uint8_t *ret_status)
    {
        int rc = backend->wem_set_sd_energy_manager_daily_data_point(year, month, day, energy_grid_in, energy_grid_out, energy_general_in, energy_general_out, price, ret_status);
        check_bricklet_reachable(rc, "set_sd_energy_manager_daily_data_point");
        return rc;
    }

    inline int wem_get_sd_energy_manager_daily_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status)
    {
        int rc = backend->wem_get_sd_energy_manager_daily_data_points(year, month, day, amount, ret_status);
        check_bricklet_reachable(rc, "get_sd_energy_manager_daily_data_points");
        return rc;
    }

    inline int wem_set_data_storage(uint8_t page, const uint8_t data[63])
    {
        return backend->wem_set_data_storage(page, data);
    }

    inline int wem_get_data_storage(uint8_t page, uint8_t ret_data[63])
    {
        return backend->wem_get_data_storage(page, ret_data);
    }

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

protected:
    void clr_error(uint32_t error_mask);
    bool is_error(uint32_t error_bit_pos) const;
    void set_config_error(uint32_t config_error_mask);
    inline bool is_bricklet_reachable() const {return bricklet_reachable;}
    void check_bricklet_reachable(int rc, const char *context);

    ConfigRoot state;
    ConfigRoot low_level_state;
    ConfigRoot config;

    uint32_t consecutive_bricklet_errors = 0;

private:
    void start_network_check_task();

    IEMBackend *backend = nullptr;

    uint32_t error_flags = 0;
    uint32_t config_error_flags = 0;
    bool     bricklet_reachable = true;
};
