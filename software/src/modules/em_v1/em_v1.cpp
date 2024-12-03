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

#include "em_v1.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "bindings/errors.h"
#include "tools.h"
#include "warp_energy_manager_bricklet_firmware_bin.embedded.h"

#include "gcc_warnings.h"

static constexpr auto EM_TASK_DELAY = 250_ms;

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

EMV1::EMV1() : DeviceModule(warp_energy_manager_bricklet_firmware_bin_data,
                            warp_energy_manager_bricklet_firmware_bin_length,
                            "energy_manager",
                            "WARP Energy Manager",
                            "Energy Manager",
                            [this](){this->setup_energy_manager();}) {}

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

void EMV1::pre_setup()
{
    this->DeviceModule::pre_setup();

    // States
    em_common.state = Config::Object({
        // Common
        {"error_flags", Config::Uint32(0)},
        {"config_error_flags", Config::Uint32(0)},
        {"em_version", Config::Uint(1, 1, 1)},
        // EMv1
        {"phases_switched", Config::Uint8(0)},
        {"input3_state", Config::Bool(false)},
        {"input4_state", Config::Bool(false)},
        {"relay_state", Config::Bool(false)},
    });

    em_common.low_level_state = Config::Object({
        // Common
        {"consecutive_bricklet_errors", Config::Uint32(0)},
        {"input_voltage", Config::Uint16(0)},
        {"uptime", Config::Uint32(0)},
        // EMv1
        {"contactor", Config::Bool(false)},
        {"contactor_check_state", Config::Uint8(0)},
        {"led_rgb", Config::Array({Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)},
            Config::get_prototype_uint8_0(), 3, 3, Config::type_id<Config::ConfUint>())
        },
    });

    // Config
    em_common.config = Config::Object({
        {"contactor_installed", Config::Bool(false)},
    });

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_action(
        AutomationActionID::EMRelaySwitch,
        Config::Object({
            {"closed", Config::Bool(false)}
        }),
        [this](const Config *cfg) {
            this->set_output(cfg->get("closed")->asBool());
        }
    );

    automation.register_trigger(
        AutomationTriggerID::EMInputThree,
        Config::Object({
            {"closed", Config::Bool(false)}
        }));

    automation.register_trigger(
        AutomationTriggerID::EMInputFour,
        Config::Object({
            {"closed", Config::Bool(false)}
        }));

    automation.register_trigger(
        AutomationTriggerID::EMPhaseSwitch,
        Config::Object({
            {"phases", Config::Uint(1)}
        }));

    automation.register_trigger(
        AutomationTriggerID::EMContactorMonitoring,
        Config::Object({
            {"contactor_okay", Config::Bool(false)}
        }));
#endif
}

#if MODULE_AUTOMATION_AVAILABLE()
bool EMV1::has_triggered(const Config *conf, void *data)
{
    const Config *cfg = static_cast<const Config *>(conf->get());

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

    switch (conf->getTag<AutomationTriggerID>()) {
        case AutomationTriggerID::EMInputThree:
            if (cfg->get("closed")->asBool() == em_common.state.get("input3_state")->asBool()) {
                return true;
            }
            break;

        case AutomationTriggerID::EMInputFour:
            if (cfg->get("closed")->asBool() == em_common.state.get("input4_state")->asBool()) {
                return true;
            }
            break;

        case AutomationTriggerID::EMPhaseSwitch:
            if (cfg->get("phases")->asUint() == em_common.state.get("phases_switched")->asUint()) {
                return true;
            }
            break;

        case AutomationTriggerID::EMContactorMonitoring:
            return (*static_cast<bool *>(data) == cfg->get("contactor_okay")->asBool());

        default:
            break;
    }
#pragma GCC diagnostic pop

    return false;
}
#endif

void EMV1::setup_energy_manager()
{
    if (!setup_device()) {
        return;
    }

    initialized = true;
}

void EMV1::setup()
{
    setup_energy_manager();
    if (!device_found) {
        em_common.set_error(ERROR_FLAGS_BRICKLET_MASK);
        return;
    }

    update_status_led();
    debug_protocol.register_backend(this);

    api.restorePersistentConfig("energy_manager/config", &em_common.config);

    // Cache config
    contactor_installed = em_common.config.get("contactor_installed")->asBool();

    // Initialize contactor check state so that the check doesn't trip immediately if the first response from the bricklet is invalid.
    all_data.contactor_check_state = 1;

    // Bricklet and meter access
    update_all_data();

    // Start this task even if a config error is set below: If only MeterEM::update_all_values runs, there will be 2.5 sec gaps in the meters data.
    task_scheduler.scheduleWithFixedDelay([this]() {
        this->update_all_data();
    }, EM_TASK_DELAY);

    power_manager.register_phase_switcher_backend(this);

#if MODULE_AUTOMATION_AVAILABLE()
    if (!contactor_installed) {
        automation.set_enabled(AutomationTriggerID::EMPhaseSwitch, false);
        automation.set_enabled(AutomationTriggerID::EMContactorMonitoring, false);
    }

    task_scheduler.scheduleOnce([this]() {
        automation.trigger(AutomationTriggerID::EMInputThree, nullptr, this);
        automation.trigger(AutomationTriggerID::EMInputFour,  nullptr, this);

        if (this->contactor_installed) {
            automation.trigger(AutomationTriggerID::EMPhaseSwitch, nullptr, this);

            bool contactor_okay = all_data.contactor_check_state & 1;
            automation.trigger(AutomationTriggerID::EMContactorMonitoring, &contactor_okay, this);
        }
    });
#endif
}

void EMV1::register_urls()
{
    this->DeviceModule::register_urls();
}

// for IEMBackend

bool EMV1::is_initialized() const
{
    return initialized;
}

bool EMV1::device_module_is_in_bootloader(int rc)
{
    return is_in_bootloader(rc);
}

uint32_t EMV1::get_em_version() const
{
    return 1;
}

const EMAllDataCommon *EMV1::get_all_data_common() const
{
    return &all_data.common;
}

void EMV1::get_input_output_states(bool *inputs, size_t *inputs_len, bool *outputs, size_t *outputs_len) const
{
    if (*inputs_len < 2) {
        *inputs_len = 0;
    } else {
        inputs[0] = all_data.input[0];
        inputs[1] = all_data.input[1];
        *inputs_len = 2;
    }

    if (*outputs_len < 1) {
        *outputs_len = 0;
    } else {
        outputs[0] = all_data.relay;
        *outputs_len = 1;
    }
}

int EMV1::wem_register_sd_wallbox_data_points_low_level_callback(WEM_SDWallboxDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_register_sd_wallbox_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManager_SDWallboxDataPointsLowLevelHandler>(handler), user_data);
}

int EMV1::wem_register_sd_wallbox_daily_data_points_low_level_callback(WEM_SDWallboxDailyDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_register_sd_wallbox_daily_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManager_SDWallboxDailyDataPointsLowLevelHandler>(handler), user_data);
}

int EMV1::wem_register_sd_energy_manager_data_points_low_level_callback(WEM_SDEnergyManagerDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_register_sd_energy_manager_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManager_SDEnergyManagerDataPointsLowLevelHandler>(handler), user_data);
}

int EMV1::wem_register_sd_energy_manager_daily_data_points_low_level_callback(WEM_SDEnergyManagerDailyDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_register_sd_energy_manager_daily_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManager_SDEnergyManagerDailyDataPointsLowLevelHandler>(handler), user_data);
}

int EMV1::wem_get_sd_information(uint32_t *ret_sd_status, uint32_t *ret_lfs_status, uint16_t *ret_sector_size, uint32_t *ret_sector_count, uint32_t *ret_card_type, uint8_t *ret_product_rev, char ret_product_name[5], uint8_t *ret_manufacturer_id)
{
    return tf_warp_energy_manager_get_sd_information(&device, ret_sd_status, ret_lfs_status, ret_sector_size, ret_sector_count, ret_card_type, ret_product_rev, ret_product_name, ret_manufacturer_id);
}

int EMV1::wem_set_sd_wallbox_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t flags, uint16_t power, uint8_t *ret_status)
{
    if ((flags & 0xFF00) != 0) {
        logger.printfln("Losing the upper 8 flags while storing wallbox 5min data point");
    }

    return tf_warp_energy_manager_set_sd_wallbox_data_point(&device, wallbox_id, year, month, day, hour, minute, static_cast<uint8_t>(flags & 0xFF), power, ret_status);
}

int EMV1::wem_get_sd_wallbox_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_get_sd_wallbox_data_points(&device, wallbox_id, year, month, day, hour, minute, amount, ret_status);
}

int EMV1::wem_set_sd_wallbox_daily_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint32_t energy, uint8_t *ret_status)
{
    return tf_warp_energy_manager_set_sd_wallbox_daily_data_point(&device, wallbox_id, year, month, day, energy, ret_status);
}

int EMV1::wem_get_sd_wallbox_daily_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_get_sd_wallbox_daily_data_points(&device, wallbox_id, year, month, day, amount, ret_status);
}

int EMV1::wem_set_sd_energy_manager_data_point(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t flags, int32_t power_grid, const int32_t power_general[6], uint32_t price, uint8_t *ret_status)
{
    if ((flags & 0xFF00) != 0) {
        logger.printfln("Losing the upper 8 flags while storing Energy Manager 5min data point");
    }

    return tf_warp_energy_manager_set_sd_energy_manager_data_point(&device, year, month, day, hour, minute, static_cast<uint8_t>(flags & 0xFF), power_grid, power_general, price, ret_status);
}

int EMV1::wem_get_sd_energy_manager_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_get_sd_energy_manager_data_points(&device, year, month, day, hour, minute, amount, ret_status);
}

int EMV1::wem_set_sd_energy_manager_daily_data_point(uint8_t year, uint8_t month, uint8_t day, uint32_t energy_grid_in, uint32_t energy_grid_out, const uint32_t energy_general_in[6], const uint32_t energy_general_out[6], uint32_t price, uint8_t *ret_status)
{
    return tf_warp_energy_manager_set_sd_energy_manager_daily_data_point(&device, year, month, day, energy_grid_in, energy_grid_out, energy_general_in, energy_general_out, price, ret_status);
}

int EMV1::wem_get_sd_energy_manager_daily_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_get_sd_energy_manager_daily_data_points(&device, year, month, day, amount, ret_status);
}

int EMV1::wem_format_sd(uint32_t password, uint8_t *ret_format_status)
{
    return tf_warp_energy_manager_format_sd(&device, password, ret_format_status);
}

int EMV1::wem_set_date_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t days_of_week, uint8_t month, uint16_t year)
{
    return tf_warp_energy_manager_set_date_time(&device, seconds, minutes, hours, days, days_of_week, month, year);
}

int EMV1::wem_get_date_time(uint8_t *ret_seconds, uint8_t *ret_minutes, uint8_t *ret_hours, uint8_t *ret_days, uint8_t *ret_days_of_week, uint8_t *ret_month, uint16_t *ret_year)
{
    return tf_warp_energy_manager_get_date_time(&device, ret_seconds, ret_minutes, ret_hours, ret_days, ret_days_of_week, ret_month, ret_year);
}

int EMV1::wem_set_data_storage(uint8_t page, const uint8_t data[63])
{
    return tf_warp_energy_manager_set_data_storage(&device, page, data);
}

int EMV1::wem_get_data_storage(uint8_t page, uint8_t *status, uint8_t ret_data[63])
{
    return tf_warp_energy_manager_get_data_storage(&device, page, status, ret_data);
}

int EMV1::wem_reset_energy_meter_relative_energy()
{
    return tf_warp_energy_manager_reset_energy_meter_relative_energy(&device);
}

int EMV1::wem_get_energy_meter_detailed_values(float *ret_values, uint16_t *ret_values_length)
{
    return tf_warp_energy_manager_get_energy_meter_detailed_values(&device, ret_values, ret_values_length);
}

// for PhaseSwitcherBackend

bool EMV1::phase_switching_capable()
{
    return contactor_installed;
}

bool EMV1::can_switch_phases_now(uint32_t /*phases_wanted*/)
{
    if (!contactor_installed) {
        return false;
    }

    if (get_phase_switching_state() != PhaseSwitcherBackend::SwitchingState::Ready) {
        return false;
    }

    return true;
}

uint32_t EMV1::get_phases()
{
    return all_data.contactor_value ? 3 : 1;
}

PhaseSwitcherBackend::SwitchingState EMV1::get_phase_switching_state()
{
    if (!contactor_installed) {
        // Don't report an error when phase_switching_capable() is false.
        return PhaseSwitcherBackend::SwitchingState::Ready;
    }

    if (contactor_check_tripped || !em_common.is_bricklet_reachable()) {
        return PhaseSwitcherBackend::SwitchingState::Error;
    }

    if (phase_switch_deadtime_us == 0_us) {
        return PhaseSwitcherBackend::SwitchingState::Ready;
    }

    if (!deadline_elapsed(phase_switch_deadtime_us)) {
        return PhaseSwitcherBackend::SwitchingState::Busy;
    }

    phase_switch_deadtime_us = 0_us;

    return PhaseSwitcherBackend::SwitchingState::Ready;
}

bool EMV1::switch_phases(uint32_t phases_wanted)
{
    if (!contactor_installed) {
        logger.printfln("Requested phase switch without contactor installed.");
        return false;
    }

    if (get_phase_switching_state() != PhaseSwitcherBackend::SwitchingState::Ready) {
        logger.printfln("Requested phase switch while not ready.");
        return false;
    }

    tf_warp_energy_manager_set_contactor(&device, phases_wanted == 3);
    phase_switch_deadtime_us = now_us() + micros_t{2000000}; // 2s

    return true;
}

#if MODULE_AUTOMATION_AVAILABLE()
template<typename T>
void EMV1::update_all_data_triggers(T id, void *data_)
{
    // Don't attempt to trigger actions during the setup stage because the automation rules are probably not loaded yet.
    // Start-up triggers are dispatched from a task started in our setup().
    if (boot_stage > BootStage::SETUP) {
        automation.trigger(id, data_, this);
    }
}
#define AUTOMATION_TRIGGER(TRIGGER_ID, DATA) update_all_data_triggers(AutomationTriggerID::TRIGGER_ID, DATA)
#else
#define AUTOMATION_TRIGGER(TRIGGER_ID, DATA) do {(void)DATA;} while (0)
#endif

void EMV1::update_all_data()
{
    update_all_data_struct();

    em_common.low_level_state.get("contactor")->updateBool(all_data.contactor_value);
    em_common.low_level_state.get("led_rgb")->get(0)->updateUint(all_data.rgb_value_r);
    em_common.low_level_state.get("led_rgb")->get(1)->updateUint(all_data.rgb_value_g);
    em_common.low_level_state.get("led_rgb")->get(2)->updateUint(all_data.rgb_value_b);
    if (em_common.state.get("input3_state")->updateBool(all_data.input[0])) AUTOMATION_TRIGGER(EMInputThree, nullptr);
    if (em_common.state.get("input4_state")->updateBool(all_data.input[1])) AUTOMATION_TRIGGER(EMInputFour, nullptr);
    em_common.state.get("relay_state")->updateBool(all_data.relay);
    em_common.low_level_state.get("input_voltage")->updateUint(all_data.common.voltage);
    em_common.low_level_state.get("contactor_check_state")->updateUint(all_data.contactor_check_state);
    em_common.low_level_state.get("uptime")->updateUint(all_data.common.uptime);

    // Update derived states
    uint32_t have_phases = 1 + static_cast<uint32_t>(all_data.contactor_value) * 2;
    if (em_common.state.get("phases_switched")->updateUint(have_phases)) AUTOMATION_TRIGGER(EMPhaseSwitch, nullptr);

#if MODULE_METERS_EM_AVAILABLE()
    meters_em.update_from_em_all_data(all_data.common);
#endif

    // Update meter values even if the config is bad.
    if (em_common.is_error(ERROR_FLAGS_BAD_CONFIG_MASK))
        return;

    if (contactor_installed) {
        if ((all_data.contactor_check_state & 1) == 0) {
            logger.printfln("Contactor check tripped. Check contactor.");
            if (!contactor_check_tripped) {
                bool contactor_okay = all_data.contactor_check_state & 1;
                AUTOMATION_TRIGGER(EMContactorMonitoring, &contactor_okay);
            }
            contactor_check_tripped = true;
            em_common.set_error(ERROR_FLAGS_CONTACTOR_MASK);
        }
    }
}

void EMV1::update_all_data_struct()
{
    int rc = tf_warp_energy_manager_get_all_data_1(
        &device,
        &all_data.contactor_value,
        &all_data.rgb_value_r,
        &all_data.rgb_value_g,
        &all_data.rgb_value_b,
        &all_data.common.power,
        all_data.common.current,
        &all_data.common.energy_meter_type,
        all_data.common.error_count,
        all_data.input,
        &all_data.relay,
        &all_data.common.voltage,
        &all_data.contactor_check_state,
        &all_data.common.uptime
    );

    em_common.check_bricklet_reachable(rc, "update_all_data_struct");

    if (rc == TF_E_OK) {
        all_data.common.last_update = millis();
        all_data.common.is_valid = true;
    }
}

void EMV1::update_status_led()
{
    if (!device_found)
        return;

    if (em_common.is_error(ERROR_FLAGS_BAD_CONFIG_BIT_POS))
        rgb_led.set_status(EmRgbLed::Status::BadConfig);
    else if (em_common.is_error(ERROR_FLAGS_ALL_ERRORS_MASK))
        rgb_led.set_status(EmRgbLed::Status::Error);
    else if (em_common.is_error(ERROR_FLAGS_ALL_WARNINGS_MASK))
        rgb_led.set_status(EmRgbLed::Status::Warning);
    else
        rgb_led.set_status(EmRgbLed::Status::OK);
}

void EMV1::set_output(bool output_value)
{
    int result = tf_warp_energy_manager_set_output(&device, output_value);

    // Don't check if bricklet is reachable because the setter call won't tell us.

    if (result != TF_E_OK)
        logger.printfln("Failed to set output relay: error %i", result);
}

void EMV1::set_rgb_led(uint8_t pattern, uint16_t hue)
{
    int rc = tf_warp_energy_manager_set_led_state(&device, pattern, hue);

    // Don't check if bricklet is reachable because the setter call won't tell us.

    if (rc != TF_E_OK)
        logger.printfln("Failed to set LED state: error %i. Continuing anyway.", rc);
}

void EMV1::update_grid_balance_led(EmRgbLed::GridBalance balance)
{
    rgb_led.update_grid_balance(balance);
}

bool EMV1::block_firmware_update_with_vehicle_connected()
{
    return contactor_installed;
}
