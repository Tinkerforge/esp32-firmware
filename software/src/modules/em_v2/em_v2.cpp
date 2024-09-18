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

#include "em_v2.h"
#include "module_dependencies.h"

#include "bindings/errors.h"
#include "event_log_prefix.h"
#include "tools.h"
#include "warp_energy_manager_v2_bricklet_firmware_bin.embedded.h"

#include "gcc_warnings.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

EMV2::EMV2() : DeviceModule(warp_energy_manager_v2_bricklet_firmware_bin_data,
                                              warp_energy_manager_v2_bricklet_firmware_bin_length,
                                              "energy_manager",
                                              "WARP Energy Manager 2.0",
                                              "Energy Manager V2",
                                              [this](){this->setup_energy_manager();}) {}

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

void EMV2::pre_setup()
{
    this->DeviceModule::pre_setup();

    const Config *prototype_bool_false = new Config{Config::Bool(false)};

    // FIXME states and low_level_states are wrong
    // States
    em_common.state = Config::Object({
        // Common
        {"error_flags", Config::Uint32(0)},
        {"config_error_flags", Config::Uint32(0)},
        {"em_version", Config::Uint(2, 2, 2)},
        // EMv2
        {"inputs", Config::Array(
            {Config::Bool(false), Config::Bool(false), Config::Bool(false), Config::Bool(false)},
            prototype_bool_false,
            4, 4, Config::type_id<Config::ConfBool>())
        },
        {"sg_ready_outputs", Config::Array(
            {Config::Bool(false), Config::Bool(false)},
            prototype_bool_false,
            2, 2, Config::type_id<Config::ConfBool>())
        },
        {"relays", Config::Array(
            {Config::Bool(false), Config::Bool(false)},
            prototype_bool_false,
            2, 2, Config::type_id<Config::ConfBool>())
        },
    });

    em_common.low_level_state = Config::Object({
        // Common
        {"consecutive_bricklet_errors", Config::Uint32(0)},
        {"input_voltage", Config::Uint16(0)},
        {"uptime", Config::Uint32(0)},
        // EMv2
    });

    // Config
    //em_common.config = Config::Object({
    //});

#if MODULE_AUTOMATION_AVAILABLE()
//    automation.register_action(
//        AutomationActionID::EMRelaySwitch,
//        Config::Object({
//            {"closed", Config::Bool(false)}
//        }),
//        [this](const Config *cfg) {
//            this->set_output(cfg->get("closed")->asBool());
//        }
//    );
//
//    automation.register_trigger(
//        AutomationTriggerID::EMInputThree,
//        Config::Object({
//            {"closed", Config::Bool(false)}
//        }));
#endif
}

#if MODULE_AUTOMATION_AVAILABLE()
bool EMV2::has_triggered(const Config *conf, void *data)
{
    //const Config *cfg = static_cast<const Config *>(conf->get());

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

    switch (conf->getTag<AutomationTriggerID>()) {
//        case AutomationTriggerID::EMInputThree:
//            if (cfg->get("closed")->asBool() == em_common.state.get("input3_state")->asBool()) {
//                return true;
//            }
//            break;

        default:
            break;
    }
#pragma GCC diagnostic pop

    return false;
}
#endif

void EMV2::setup_energy_manager()
{
    if (!this->DeviceModule::setup_device()) {
        logger.printfln("setup_device error. Reboot in 5 Minutes.");

        task_scheduler.scheduleOnce([]() {
            trigger_reboot("Energy Manager");
        }, 5 * 60 * 1000);
        return;
    }

    initialized = true;
}

void EMV2::setup()
{
    setup_energy_manager();
    if (!device_found) {
        em_common.set_error(ERROR_FLAGS_BRICKLET_MASK);
        return;
    }

    //update_status_led();
    //debug_protocol.register_backend(this);

    //api.restorePersistentConfig("energy_manager/config", &em_common.config);

    // Cache config

    // Bricklet and meter access
    update_all_data();

    // Start this task even if a config error is set below: If only MeterEM::update_all_values runs, there will be 2.5 sec gaps in the meters data.
    task_scheduler.scheduleWithFixedDelay([this]() {
        this->update_all_data();
    }, 0, EM_TASK_DELAY_MS);
}

void EMV2::register_urls()
{
    this->DeviceModule::register_urls();
}

// for IEMBackend

bool EMV2::is_initialized() const
{
    return initialized;
}

bool EMV2::device_module_is_in_bootloader(int rc)
{
    return is_in_bootloader(rc);
}

uint32_t EMV2::get_em_version() const
{
    return 2;
}

const EMAllDataCommon *EMV2::get_all_data_common() const
{
    return &all_data.common;
}

void EMV2::get_input_output_states(bool *inputs, size_t *inputs_len, bool *outputs, size_t *outputs_len) const
{
    if (*inputs_len < 4) {
        *inputs_len = 0;
    } else {
        inputs[0] = all_data.input[0];
        inputs[1] = all_data.input[1];
        inputs[2] = all_data.input[2];
        inputs[3] = all_data.input[3];
        *inputs_len = 4;
    }

    if (*outputs_len < 4) {
        *outputs_len = 0;
    } else {
        outputs[0] = all_data.output_relay[0];
        outputs[1] = all_data.output_relay[1];
        outputs[2] = all_data.output_sg_ready[0];
        outputs[3] = all_data.output_sg_ready[1];
        *outputs_len = 4;
    }
}

int EMV2::wem_register_sd_wallbox_data_points_low_level_callback(WEM_SDWallboxDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_v2_register_sd_wallbox_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManagerV2_SDWallboxDataPointsLowLevelHandler>(handler), user_data);
}

int EMV2::wem_register_sd_wallbox_daily_data_points_low_level_callback(WEM_SDWallboxDailyDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_v2_register_sd_wallbox_daily_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManagerV2_SDWallboxDailyDataPointsLowLevelHandler>(handler), user_data);
}

int EMV2::wem_register_sd_energy_manager_data_points_low_level_callback(WEM_SDEnergyManagerDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_v2_register_sd_energy_manager_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManagerV2_SDEnergyManagerDataPointsLowLevelHandler>(handler), user_data);
}

int EMV2::wem_register_sd_energy_manager_daily_data_points_low_level_callback(WEM_SDEnergyManagerDailyDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_v2_register_sd_energy_manager_daily_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManagerV2_SDEnergyManagerDailyDataPointsLowLevelHandler>(handler), user_data);
}

int EMV2::wem_get_sd_information(uint32_t *ret_sd_status, uint32_t *ret_lfs_status, uint16_t *ret_sector_size, uint32_t *ret_sector_count, uint32_t *ret_card_type, uint8_t *ret_product_rev, char ret_product_name[5], uint8_t *ret_manufacturer_id)
{
    return tf_warp_energy_manager_v2_get_sd_information(&device, ret_sd_status, ret_lfs_status, ret_sector_size, ret_sector_count, ret_card_type, ret_product_rev, ret_product_name, ret_manufacturer_id);
}

int EMV2::wem_set_sd_wallbox_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, uint16_t power, uint8_t *ret_status)
{
    return tf_warp_energy_manager_v2_set_sd_wallbox_data_point(&device, wallbox_id, year, month, day, hour, minute, flags, power, ret_status);
}

int EMV2::wem_get_sd_wallbox_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_v2_get_sd_wallbox_data_points(&device, wallbox_id, year, month, day, hour, minute, amount, ret_status);
}

int EMV2::wem_set_sd_wallbox_daily_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint32_t energy, uint8_t *ret_status)
{
    return tf_warp_energy_manager_v2_set_sd_wallbox_daily_data_point(&device, wallbox_id, year, month, day, energy, ret_status);
}

int EMV2::wem_get_sd_wallbox_daily_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_v2_get_sd_wallbox_daily_data_points(&device, wallbox_id, year, month, day, amount, ret_status);
}

int EMV2::wem_set_sd_energy_manager_data_point(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, int32_t power_grid, const int32_t power_general[6], uint32_t price, uint8_t *ret_status)
{
    return tf_warp_energy_manager_v2_set_sd_energy_manager_data_point(&device, year, month, day, hour, minute, flags, power_grid, power_general, price, ret_status);
}

int EMV2::wem_get_sd_energy_manager_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_v2_get_sd_energy_manager_data_points(&device, year, month, day, hour, minute, amount, ret_status);
}

int EMV2::wem_set_sd_energy_manager_daily_data_point(uint8_t year, uint8_t month, uint8_t day, uint32_t energy_grid_in, uint32_t energy_grid_out, const uint32_t energy_general_in[6], const uint32_t energy_general_out[6], uint32_t price, uint8_t *ret_status)
{
    return tf_warp_energy_manager_v2_set_sd_energy_manager_daily_data_point(&device, year, month, day, energy_grid_in, energy_grid_out, energy_general_in, energy_general_out, price, ret_status);
}

int EMV2::wem_get_sd_energy_manager_daily_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_v2_get_sd_energy_manager_daily_data_points(&device, year, month, day, amount, ret_status);
}

int EMV2::wem_format_sd(uint32_t password, uint8_t *ret_format_status)
{
    return tf_warp_energy_manager_v2_format_sd(&device, password, ret_format_status);
}

int EMV2::wem_set_date_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t days_of_week, uint8_t month, uint16_t year)
{
    return tf_warp_energy_manager_v2_set_date_time(&device, seconds, minutes, hours, days, days_of_week, month, year);
}

int EMV2::wem_get_date_time(uint8_t *ret_seconds, uint8_t *ret_minutes, uint8_t *ret_hours, uint8_t *ret_days, uint8_t *ret_days_of_week, uint8_t *ret_month, uint16_t *ret_year)
{
    return tf_warp_energy_manager_v2_get_date_time(&device, ret_seconds, ret_minutes, ret_hours, ret_days, ret_days_of_week, ret_month, ret_year);
}

int EMV2::wem_set_data_storage(uint8_t page, const uint8_t data[63])
{
    return tf_warp_energy_manager_v2_set_data_storage(&device, page, data);
}

int EMV2::wem_get_data_storage(uint8_t page, uint8_t ret_data[63])
{
    return tf_warp_energy_manager_v2_get_data_storage(&device, page, ret_data);
}

int EMV2::wem_reset_energy_meter_relative_energy()
{
    return tf_warp_energy_manager_v2_reset_energy_meter_relative_energy(&device);
}

int EMV2::wem_get_energy_meter_detailed_values(float *ret_values, uint16_t *ret_values_length)
{
    return tf_warp_energy_manager_v2_get_energy_meter_detailed_values(&device, ret_values, ret_values_length);
}

void EMV2::update_all_data()
{
    update_all_data_struct();

    em_common.low_level_state.get("input_voltage")->updateUint(all_data.common.voltage);
    em_common.low_level_state.get("uptime")->updateUint(all_data.common.uptime);

    Config *state_inputs = static_cast<Config *>(em_common.state.get("inputs"));
    bool *inputs = all_data.input;
    for (uint16_t i = 0; i < ARRAY_SIZE(all_data.input); i++) {
        state_inputs->get(i)->updateBool(inputs[i]);
    }

    Config *state_sg_ready = static_cast<Config *>(em_common.state.get("sg_ready_outputs"));
    state_sg_ready->get(0)->updateBool(all_data.output_sg_ready[0]);
    state_sg_ready->get(1)->updateBool(all_data.output_sg_ready[1]);

    Config *state_relays = static_cast<Config *>(em_common.state.get("relays"));
    state_relays->get(0)->updateBool(all_data.output_relay[0]);
    state_relays->get(1)->updateBool(all_data.output_relay[1]);

#if MODULE_METERS_EM_AVAILABLE()
    meters_em.update_from_em_all_data(all_data.common);
#endif
}

void EMV2::update_all_data_struct()
{
    int rc = tf_warp_energy_manager_v2_get_all_data_1(
        &device,
        &all_data.common.power,
        all_data.common.current,
        &all_data.common.energy_meter_type,
        all_data.common.error_count,
        all_data.input,
        all_data.output_sg_ready,
        all_data.output_relay,
        &all_data.common.voltage,
        &all_data.common.uptime
    );

    em_common.check_bricklet_reachable(rc, "update_all_data_struct");

    if (rc == TF_E_OK) {
        all_data.common.last_update = millis();
        all_data.common.is_valid = true;
    }
}

bool EMV2::get_input(uint8_t channel)
{
    if (channel > 3) {
        logger.printfln("get_input channel out of range: %u > 3", channel);
        return false;
    }

    return all_data.input[channel];
}

void EMV2::set_sg_ready_output(uint8_t channel, bool value)
{
    if (channel > 1) {
        logger.printfln("set_sg_ready_output channel out of range: %u > 1", channel);
        return;
    }

    int rc = tf_warp_energy_manager_v2_set_sg_ready_output(&device, channel, value);

    // Don't check if bricklet is reachable because the setter call won't tell us.

    if (rc != TF_E_OK) {
        logger.printfln("Failed to set SG Ready output %u: error %i", channel, rc);
    }
}

bool EMV2::get_sg_ready_output(uint8_t channel)
{
    if (channel > 1) {
        logger.printfln("get_sg_ready_output channel out of range: %u > 1", channel);
        return false;
    }

    return all_data.output_sg_ready[channel];
}

void EMV2::set_relay_output(uint8_t channel, bool value)
{
    if (channel > 1) {
        logger.printfln("set_relay_output channel out of range: %u > 1", channel);
        return;
    }

    int rc = tf_warp_energy_manager_v2_set_relay_output(&device, channel, value);

    // Don't check if bricklet is reachable because the setter call won't tell us.

    if (rc != TF_E_OK) {
        logger.printfln("Failed to set SG Ready output %u: error %i", channel, rc);
    }
}

bool EMV2::get_relay_output(uint8_t channel)
{
    if (channel > 1) {
        logger.printfln("get_relay_output channel out of range: %u > 1", channel);
        return false;
    }

    return all_data.output_relay[channel];
}