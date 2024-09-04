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

#include "energy_manager.h"

#include <type_traits>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "musl_libc_timegm.h"
#include "bindings/errors.h"
#include "build.h"
#include "tools.h"
#include "warp_energy_manager_bricklet_firmware_bin.embedded.h"

#include "gcc_warnings.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

EnergyManager::EnergyManager() : DeviceModule(warp_energy_manager_bricklet_firmware_bin_data,
                                              warp_energy_manager_bricklet_firmware_bin_length,
                                              "energy_manager",
                                              "WARP Energy Manager",
                                              "Energy Manager",
                                              [this](){this->setup_energy_manager();}) {}

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

void EnergyManager::pre_setup()
{
    this->DeviceModule::pre_setup();

    // States
    em_common.state = Config::Object({
        {"phases_switched", Config::Uint8(0)},
        {"input3_state", Config::Bool(false)},
        {"input4_state", Config::Bool(false)},
        {"relay_state", Config::Bool(false)},
        {"error_flags", Config::Uint32(0)},
        {"config_error_flags", Config::Uint32(0)},
    });

    em_common.low_level_state = Config::Object({
        {"consecutive_bricklet_errors", Config::Uint32(0)},
        // Bricklet states below
        {"contactor", Config::Bool(false)},
        {"contactor_check_state", Config::Uint8(0)},
        {"input_voltage", Config::Uint16(0)},
        {"led_rgb", Config::Array({Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)},
            new Config{Config::Uint8(0)}, 3, 3, Config::type_id<Config::ConfUint>())
        },
        {"uptime", Config::Uint32(0)},
    });

    // Config
    em_common.config = Config::Object({
        {"contactor_installed", Config::Bool(false)},
    });

    // history
    history_wallbox_5min = Config::Object({
        {"uid", Config::Uint32(0)},
        // date in UTC to avoid DST overlap problems
        {"year", Config::Uint(0, 2000, 2255)},
        {"month", Config::Uint(0, 1, 12)},
        {"day", Config::Uint(0, 1, 31)},
    });

    history_wallbox_daily = Config::Object({
        {"uid", Config::Uint32(0)},
        // date in local time to have the days properly aligned
        {"year", Config::Uint(0, 2000, 2255)},
        {"month", Config::Uint(0, 1, 12)},
    });

    history_energy_manager_5min = Config::Object({
        // date in UTC to avoid DST overlap problems
        {"year", Config::Uint(0, 2000, 2255)},
        {"month", Config::Uint(0, 1, 12)},
        {"day", Config::Uint(0, 1, 31)},
    });

    history_energy_manager_daily = Config::Object({
        // date in local time to have the days properly aligned
        {"year", Config::Uint(0, 2000, 2255)},
        {"month", Config::Uint(0, 1, 12)},
    });

    for (uint32_t slot = 0; slot < METERS_SLOTS; ++slot) {
        history_meter_setup_done[slot] = false;
        history_meter_power_value[slot] = NAN;
    }

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
bool EnergyManager::has_triggered(const Config *conf, void *data)
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

void EnergyManager::setup_energy_manager()
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

void EnergyManager::setup()
{
    setup_energy_manager();
    if (!device_found) {
        em_common.set_error(ERROR_FLAGS_BRICKLET_MASK);
        return;
    }

    api.addFeature("energy_manager");

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
    }, 0, EM_TASK_DELAY_MS);

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
    }, 0);
#endif

    task_scheduler.scheduleWithFixedDelay([this](){collect_data_points();}, 15000, 10000);
    task_scheduler.scheduleWithFixedDelay([this](){set_pending_data_points();}, 15000, 100);

    start_network_check_task();

    task_scheduler.scheduleOnce([this](){this->show_blank_value_id_update_warnings = true;}, 250);
}

void EnergyManager::register_urls()
{
    api.addState("energy_manager/state", &em_common.state);

    api.addPersistentConfig("energy_manager/config", &em_common.config);
    api.addState("energy_manager/low_level_state", &em_common.low_level_state);

    api.addResponse("energy_manager/history_wallbox_5min", &history_wallbox_5min, {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_wallbox_5min_response(response, ownership, owner_id);});
    api.addResponse("energy_manager/history_wallbox_daily", &history_wallbox_daily, {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_wallbox_daily_response(response, ownership, owner_id);});
    api.addResponse("energy_manager/history_energy_manager_5min", &history_energy_manager_5min, {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_energy_manager_5min_response(response, ownership, owner_id);});
    api.addResponse("energy_manager/history_energy_manager_daily", &history_energy_manager_daily, {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_energy_manager_daily_response(response, ownership, owner_id);});

    this->DeviceModule::register_urls();
}

// for IEMBackend

bool EnergyManager::is_initialized() const
{
    return initialized;
}

bool EnergyManager::device_module_is_in_bootloader(int rc)
{
    return is_in_bootloader(rc);
}

uint32_t EnergyManager::get_em_version() const
{
    return 1;
}

const EMAllDataCommon *EnergyManager::get_all_data_common() const
{
    return &all_data.common;
}

int EnergyManager::wem_register_sd_wallbox_data_points_low_level_callback(WEM_SDWallboxDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_register_sd_wallbox_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManager_SDWallboxDataPointsLowLevelHandler>(handler), user_data);
}

int EnergyManager::wem_register_sd_wallbox_daily_data_points_low_level_callback(WEM_SDWallboxDailyDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_register_sd_wallbox_daily_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManager_SDWallboxDailyDataPointsLowLevelHandler>(handler), user_data);
}

int EnergyManager::wem_register_sd_energy_manager_data_points_low_level_callback(WEM_SDEnergyManagerDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_register_sd_energy_manager_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManager_SDEnergyManagerDataPointsLowLevelHandler>(handler), user_data);
}

int EnergyManager::wem_register_sd_energy_manager_daily_data_points_low_level_callback(WEM_SDEnergyManagerDailyDataPointsLowLevelHandler handler, void *user_data)
{
    return tf_warp_energy_manager_register_sd_energy_manager_daily_data_points_low_level_callback(&device, reinterpret_cast<TF_WARPEnergyManager_SDEnergyManagerDailyDataPointsLowLevelHandler>(handler), user_data);
}

int EnergyManager::wem_set_sd_wallbox_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, uint16_t power, uint8_t *ret_status)
{
    return tf_warp_energy_manager_set_sd_wallbox_data_point(&device, wallbox_id, year, month, day, hour, minute, flags, power, ret_status);
}

int EnergyManager::wem_get_sd_wallbox_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_get_sd_wallbox_data_points(&device, wallbox_id, year, month, day, hour, minute, amount, ret_status);
}

int EnergyManager::wem_set_sd_wallbox_daily_data_point(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint32_t energy, uint8_t *ret_status)
{
    return tf_warp_energy_manager_set_sd_wallbox_daily_data_point(&device, wallbox_id, year, month, day, energy, ret_status);
}

int EnergyManager::wem_get_sd_wallbox_daily_data_points(uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_get_sd_wallbox_daily_data_points(&device, wallbox_id, year, month, day, amount, ret_status);
}

int EnergyManager::wem_set_sd_energy_manager_data_point(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, int32_t power_grid, const int32_t power_general[6], uint8_t *ret_status)
{
    return tf_warp_energy_manager_set_sd_energy_manager_data_point(&device, year, month, day, hour, minute, flags, power_grid, power_general, ret_status);
}

int EnergyManager::wem_get_sd_energy_manager_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_get_sd_energy_manager_data_points(&device, year, month, day, hour, minute, amount, ret_status);
}

int EnergyManager::wem_set_sd_energy_manager_daily_data_point(uint8_t year, uint8_t month, uint8_t day, uint32_t energy_grid_in, uint32_t energy_grid_out, const uint32_t energy_general_in[6], const uint32_t energy_general_out[6], uint8_t *ret_status)
{
    return tf_warp_energy_manager_set_sd_energy_manager_daily_data_point(&device, year, month, day, energy_grid_in, energy_grid_out, energy_general_in, energy_general_out, ret_status);
}

int EnergyManager::wem_get_sd_energy_manager_daily_data_points(uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status)
{
    return tf_warp_energy_manager_get_sd_energy_manager_daily_data_points(&device, year, month, day, amount, ret_status);
}

int EnergyManager::wem_get_data_storage(uint8_t page, uint8_t ret_data[63])
{
    return tf_warp_energy_manager_get_data_storage(&device, page, ret_data);
}

int EnergyManager::wem_set_data_storage(uint8_t page, const uint8_t data[63])
{
    return tf_warp_energy_manager_set_data_storage(&device, page, data);
}

// for PhaseSwitcherBackend

bool EnergyManager::phase_switching_capable()
{
    return contactor_installed;
}

bool EnergyManager::can_switch_phases_now(bool /*wants_3phase*/)
{
    if (!contactor_installed) {
        return false;
    }

    if (get_phase_switching_state() != PhaseSwitcherBackend::SwitchingState::Ready) {
        return false;
    }

    return true;
}

bool EnergyManager::get_is_3phase()
{
    return all_data.contactor_value;
}

PhaseSwitcherBackend::SwitchingState EnergyManager::get_phase_switching_state()
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

bool EnergyManager::switch_phases_3phase(bool wants_3phase)
{
    if (!contactor_installed) {
        logger.printfln("Requested phase switch without contactor installed.");
        return false;
    }

    if (get_phase_switching_state() != PhaseSwitcherBackend::SwitchingState::Ready) {
        logger.printfln("Requested phase switch while not ready.");
        return false;
    }

    tf_warp_energy_manager_set_contactor(&device, wants_3phase);
    phase_switch_deadtime_us = now_us() + micros_t{2000000}; // 2s

    return true;
}

#if MODULE_AUTOMATION_AVAILABLE()
template<typename T>
void EnergyManager::update_all_data_triggers(T id, void *data_)
{
    // Don't attempt to trigger actions during the setup stage because the automation rules are probably not loaded yet.
    // Start-up triggers are dispatched from a task started in our setup().
    if (boot_stage > BootStage::SETUP) {
        automation.trigger(id, data_, this);
    }
}
#define AUTOMATION_TRIGGER(TRIGGER_ID, DATA) update_all_data_triggers(AutomationTriggerID::TRIGGER_ID, DATA)
#else
#define AUTOMATION_TRIGGER(TRIGGER_ID, DATA) do {} while (0)
#endif

void EnergyManager::update_all_data()
{
    update_all_data_struct();

    em_common.low_level_state.get("contactor")->updateBool(all_data.contactor_value);
    em_common.low_level_state.get("led_rgb")->get(0)->updateUint(all_data.rgb_value_r);
    em_common.low_level_state.get("led_rgb")->get(1)->updateUint(all_data.rgb_value_g);
    em_common.low_level_state.get("led_rgb")->get(2)->updateUint(all_data.rgb_value_b);
    if (em_common.state.get("input3_state")->updateBool(all_data.input[0])) AUTOMATION_TRIGGER(EMInputThree, nullptr);
    if (em_common.state.get("input4_state")->updateBool(all_data.input[1])) AUTOMATION_TRIGGER(EMInputFour, nullptr);
    em_common.state.get("relay_state")->updateBool(all_data.relay);
    em_common.low_level_state.get("input_voltage")->updateUint(all_data.voltage);
    em_common.low_level_state.get("contactor_check_state")->updateUint(all_data.contactor_check_state);
    em_common.low_level_state.get("uptime")->updateUint(all_data.uptime);

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

void EnergyManager::update_all_data_struct()
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
        &all_data.voltage,
        &all_data.contactor_check_state,
        &all_data.uptime
    );

    em_common.check_bricklet_reachable(rc, "update_all_data_struct");

    if (rc == TF_E_OK) {
        all_data.common.last_update = millis();
        all_data.common.is_valid = true;
    }
}

void EnergyManager::update_status_led()
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

void EnergyManager::start_network_check_task()
{
    task_scheduler.scheduleWithFixedDelay([this]() {
        bool disconnected;
        do {
#if MODULE_ETHERNET_AVAILABLE()
            if (ethernet.get_connection_state() == EthernetState::Connected) {
                disconnected = false;
                break;
            }
#endif
#if MODULE_WIFI_AVAILABLE()
            if (wifi.get_connection_state() == WifiState::Connected) {
                disconnected = false;
                break;
            }
#endif
#if MODULE_ETHERNET_AVAILABLE()
            if (ethernet.is_enabled()) {
                disconnected = true;
                break;
            }
#endif
#if MODULE_WIFI_AVAILABLE()
            if (wifi.is_sta_enabled()) {
                disconnected = true;
                break;
            }
#endif
            disconnected = false;
        } while (0);

        if (disconnected) {
            em_common.set_error(ERROR_FLAGS_NETWORK_MASK);
        } else {
            if (em_common.is_error(ERROR_FLAGS_NETWORK_BIT_POS))
                em_common.clr_error(ERROR_FLAGS_NETWORK_MASK);
        }
    }, 0, 5000);
}

bool EnergyManager::get_sdcard_info(struct sdcard_info *data)
{
    int rc = tf_warp_energy_manager_get_sd_information(
        &device,
        &data->sd_status,
        &data->lfs_status,
        &data->sector_size,
        &data->sector_count,
        &data->card_type,
        &data->product_rev,
        data->product_name,
        &data->manufacturer_id
    );

    // Product name retrieved from the SD card is an unterminated 5-character string, so we have to terminate it here.
    data->product_name[sizeof(data->product_name) - 1] = 0;

    em_common.check_bricklet_reachable(rc, "get_sdcard_info");

    if (rc != TF_E_OK) {
        em_common.set_error(ERROR_FLAGS_SDCARD_MASK);
        logger.printfln("Failed to get SD card information. Error %i", rc);
        return false;
    }

    if (em_common.is_error(ERROR_FLAGS_SDCARD_BIT_POS))
        em_common.clr_error(ERROR_FLAGS_SDCARD_MASK);

    return true;
}

bool EnergyManager::format_sdcard()
{
    uint8_t ret_format_status;
    int rc = tf_warp_energy_manager_format_sd(&device, 0x4223ABCD, &ret_format_status);

    em_common.check_bricklet_reachable(rc, "format_sdcard");

    return rc == TF_E_OK && ret_format_status == TF_WARP_ENERGY_MANAGER_FORMAT_STATUS_OK;
}

uint16_t EnergyManager::get_energy_meter_detailed_values(float *ret_values)
{
    uint16_t len = 0;
    int rc = tf_warp_energy_manager_get_energy_meter_detailed_values(&device, ret_values, &len);

    em_common.check_bricklet_reachable(rc, "get_energy_meter_detailed_values");

    return rc == TF_E_OK ? len : 0;
}

bool EnergyManager::reset_energy_meter_relative_energy()
{
    int rc = tf_warp_energy_manager_reset_energy_meter_relative_energy(&device);

    em_common.check_bricklet_reachable(rc, "reset_energy_meter_relative_energy");

    return rc == TF_E_OK;
}

void EnergyManager::set_output(bool output_value)
{
    int result = tf_warp_energy_manager_set_output(&device, output_value);

    // Don't check if bricklet is reachable because the setter call won't tell us.

    if (result != TF_E_OK)
        logger.printfln("Failed to set output relay: error %i", result);
}

void EnergyManager::set_rgb_led(uint8_t pattern, uint16_t hue)
{
    int rc = tf_warp_energy_manager_set_led_state(&device, pattern, hue);

    // Don't check if bricklet is reachable because the setter call won't tell us.

    if (rc != TF_E_OK)
        logger.printfln("Failed to set LED state: error %i. Continuing anyway.", rc);
}

void EnergyManager::set_time(const tm &date_time)
{
    uint32_t retries = 3;
    int rc;

    do {
        rc = tf_warp_energy_manager_set_date_time(&device,
                                                  static_cast<uint8_t >(date_time.tm_sec),
                                                  static_cast<uint8_t >(date_time.tm_min),
                                                  static_cast<uint8_t >(date_time.tm_hour),
                                                  static_cast<uint8_t >(date_time.tm_mday - 1),
                                                  static_cast<uint8_t >(date_time.tm_wday),
                                                  static_cast<uint8_t >(date_time.tm_mon),
                                                  static_cast<uint16_t>(date_time.tm_year - 100));
        if (rc == TF_E_OK)
            return;
    } while (retries-- > 0);

    logger.printfln("Failed to set datetime: error %i", rc);
}

struct timeval EnergyManager::get_time()
{
    struct tm date_time;
    struct timeval time;
    time.tv_usec = 0;

    uint8_t tm_sec;
    uint8_t tm_min;
    uint8_t tm_hour;
    uint8_t tm_mday;
    uint8_t tm_wday;
    uint8_t tm_mon;
    uint16_t tm_year;

    uint32_t retries = 1;
    int rc;

    do {
        rc = tf_warp_energy_manager_get_date_time(&device, &tm_sec, &tm_min, &tm_hour, &tm_mday, &tm_wday, &tm_mon, &tm_year);

        em_common.check_bricklet_reachable(rc, "get_time");

        if (rc != TF_E_OK)
            continue;

        date_time.tm_sec  = tm_sec;
        date_time.tm_min  = tm_min;
        date_time.tm_hour = tm_hour;
        date_time.tm_mday = tm_mday + 1;
        date_time.tm_wday = tm_wday;
        date_time.tm_mon  = tm_mon;
        date_time.tm_year = tm_year + 100;

        time.tv_sec = timegm(&date_time);

        // Allow time to be 24h older than the build timestamp,
        // in case the RTC is set by hand to test something.
        //FIXME not Y2038-safe
        if (time.tv_sec < static_cast<time_t>(build_timestamp() - 24 * 3600))
            time.tv_sec = 0;

        return time;
    } while (retries-- > 0);

    logger.printfln("Failed to get datetime: error %i", rc);
    time.tv_sec = 0;
    return time;
}

void EnergyManager::update_grid_balance_led(EmRgbLed::GridBalance balance)
{
    rgb_led.update_grid_balance(balance);
}

bool EnergyManager::block_firmware_update_with_vehicle_connected()
{
    return contactor_installed;
}
