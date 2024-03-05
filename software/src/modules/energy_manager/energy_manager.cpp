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

#include <type_traits>

#include "energy_manager.h"
#include "module_dependencies.h"
#include "musl_libc_timegm.h"

#include "bindings/errors.h"

#include "api.h"
#include "build.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "web_server.h"

#include "gcc_warnings.h"

void EnergyManager::pre_setup()
{
    this->DeviceModule::pre_setup();

    // States
    state = Config::Object({
        {"phases_switched", Config::Uint8(0)},
        {"input3_state", Config::Bool(false)},
        {"input4_state", Config::Bool(false)},
        {"relay_state", Config::Bool(false)},
        {"error_flags", Config::Uint32(0)},
        {"config_error_flags", Config::Uint32(0)},
    });

    low_level_state = Config::Object({
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
    config = Config::Object({
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
bool EnergyManager::action_triggered(const Config *automation_config, void *data)
{
    const Config *cfg = static_cast<const Config *>(automation_config->get());

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

    switch (automation_config->getTag<AutomationTriggerID>()) {
        case AutomationTriggerID::EMInputThree:
            if (cfg->get("closed")->asBool() == state.get("input3_state")->asBool()) {
                return true;
            }
            break;

        case AutomationTriggerID::EMInputFour:
            if (cfg->get("closed")->asBool() == state.get("input4_state")->asBool()) {
                return true;
            }
            break;

        case AutomationTriggerID::EMPhaseSwitch:
            if (cfg->get("phases")->asUint() == state.get("phases_switched")->asUint()) {
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
        logger.printfln("energy_manager: setup_device error. Reboot in 5 Minutes.");

        task_scheduler.scheduleOnce([]() {
            trigger_reboot("Energy Manager");
        }, 5 * 60 * 1000);
        return;
    }

    initialized = true;
}

void EnergyManager::check_debug()
{
    task_scheduler.scheduleOnce([this]() {
        if (deadline_elapsed(last_debug_keep_alive + 60000) && debug) {
            logger.printfln("Debug log creation canceled because no continue call was received for more than 60 seconds.");
            debug = false;
        } else if (debug) {
            check_debug();
        }
    }, 10000);
}

void EnergyManager::setup()
{
    setup_energy_manager();
    if (!device_found) {
        set_error(ERROR_FLAGS_BRICKLET_MASK);
        return;
    }

    api.addFeature("energy_manager");

    update_status_led();

    api.restorePersistentConfig("energy_manager/config", &config);

    // Cache config
    contactor_installed = config.get("contactor_installed")->asBool();

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
    task_scheduler.scheduleOnce([this]() {
        auto trigger_action = [this](const Config *cfg, void *data) -> bool {return this->action_triggered(cfg, data);};
        automation.trigger_action(AutomationTriggerID::EMInputThree,  nullptr, trigger_action);
        automation.trigger_action(AutomationTriggerID::EMInputFour,   nullptr, trigger_action);
        automation.trigger_action(AutomationTriggerID::EMPhaseSwitch, nullptr, trigger_action);

        if (this->contactor_installed) {
            bool contactor_okay = all_data.contactor_check_state & 1;
            automation.trigger_action(AutomationTriggerID::EMContactorMonitoring, &contactor_okay, trigger_action);
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
    api.addState("energy_manager/state", &state);

    api.addPersistentConfig("energy_manager/config", &config);
    api.addState("energy_manager/low_level_state", &low_level_state);

    api.addResponse("energy_manager/history_wallbox_5min", &history_wallbox_5min, {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_wallbox_5min_response(response, ownership, owner_id);});
    api.addResponse("energy_manager/history_wallbox_daily", &history_wallbox_daily, {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_wallbox_daily_response(response, ownership, owner_id);});
    api.addResponse("energy_manager/history_energy_manager_5min", &history_energy_manager_5min, {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_energy_manager_5min_response(response, ownership, owner_id);});
    api.addResponse("energy_manager/history_energy_manager_daily", &history_energy_manager_daily, {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_energy_manager_daily_response(response, ownership, owner_id);});

    this->DeviceModule::register_urls();

    server.on("/energy_manager/start_debug", HTTP_GET, [this](WebServerRequest request) {
        last_debug_keep_alive = millis();
        check_debug();
        ws.pushRawStateUpdate(this->get_energy_manager_debug_header(), "energy_manager/debug_header");
        debug = true;
        return request.send(200);
    });

    server.on("/energy_manager/continue_debug", HTTP_GET, [this](WebServerRequest request) {
        last_debug_keep_alive = millis();
        return request.send(200);
    });

    server.on("/energy_manager/stop_debug", HTTP_GET, [this](WebServerRequest request) {
        debug = false;
        return request.send(200);
    });
}

void EnergyManager::loop()
{
    this->DeviceModule::loop();

    static uint32_t last_debug = 0;
    if (debug && deadline_elapsed(last_debug + 50)) {
        last_debug = millis();
        ws.pushRawStateUpdate(this->get_energy_manager_debug_line(), "energy_manager/debug");
    }
}

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
    if (!contactor_installed || contactor_check_tripped || !bricklet_reachable) {
        return PhaseSwitcherBackend::SwitchingState::Error;
    }

    if (phase_switch_deadtime_us == 0_usec) {
        return PhaseSwitcherBackend::SwitchingState::Ready;
    }

    if (!deadline_elapsed(phase_switch_deadtime_us)) {
        return PhaseSwitcherBackend::SwitchingState::Busy;
    }

    phase_switch_deadtime_us = 0_usec;

    return PhaseSwitcherBackend::SwitchingState::Ready;
}

bool EnergyManager::switch_phases_3phase(bool wants_3phase)
{
    if (!contactor_installed) {
        logger.printfln("energy_manager: Requested phase switch without contactor installed.");
        return false;
    }

    if (get_phase_switching_state() != PhaseSwitcherBackend::SwitchingState::Ready) {
        logger.printfln("energy_manager: Requested phase switch while not ready.");
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
        automation.trigger_action(id, data_, [this](const Config *cfg, void *data) -> bool {return this->action_triggered(cfg, data);});
    }
}
#define AUTOMATION_TRIGGER_ACTION(TRIGGER_ID, DATA) update_all_data_triggers(AutomationTriggerID::TRIGGER_ID, DATA)
#else
#define AUTOMATION_TRIGGER_ACTION(TRIGGER_ID, DATA) do {} while (0)
#endif

void EnergyManager::update_all_data()
{
    update_all_data_struct();

    low_level_state.get("contactor")->updateBool(all_data.contactor_value);
    low_level_state.get("led_rgb")->get(0)->updateUint(all_data.rgb_value_r);
    low_level_state.get("led_rgb")->get(1)->updateUint(all_data.rgb_value_g);
    low_level_state.get("led_rgb")->get(2)->updateUint(all_data.rgb_value_b);
    if (state.get("input3_state")->updateBool(all_data.input[0])) AUTOMATION_TRIGGER_ACTION(EMInputThree, nullptr);
    if (state.get("input4_state")->updateBool(all_data.input[1])) AUTOMATION_TRIGGER_ACTION(EMInputFour, nullptr);
    state.get("relay_state")->updateBool(all_data.relay);
    low_level_state.get("input_voltage")->updateUint(all_data.voltage);
    low_level_state.get("contactor_check_state")->updateUint(all_data.contactor_check_state);
    low_level_state.get("uptime")->updateUint(all_data.uptime);

    // Update derived states
    uint32_t have_phases = 1 + static_cast<uint32_t>(all_data.contactor_value) * 2;
    if (state.get("phases_switched")->updateUint(have_phases)) AUTOMATION_TRIGGER_ACTION(EMPhaseSwitch, nullptr);

#if MODULE_METERS_EM_AVAILABLE()
    meters_em.update_from_em_all_data(all_data);
#endif

    // Update meter values even if the config is bad.
    if (is_error(ERROR_FLAGS_BAD_CONFIG_MASK))
        return;

    if (contactor_installed) {
        if ((all_data.contactor_check_state & 1) == 0) {
            logger.printfln("Contactor check tripped. Check contactor.");
            if (!contactor_check_tripped) {
                bool contactor_okay = all_data.contactor_check_state & 1;
                AUTOMATION_TRIGGER_ACTION(EMContactorMonitoring, &contactor_okay);
            }
            contactor_check_tripped = true;
            set_error(ERROR_FLAGS_CONTACTOR_MASK);
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
        &all_data.power,
        all_data.current,
        &all_data.energy_meter_type,
        all_data.error_count,
        all_data.input,
        &all_data.relay,
        &all_data.voltage,
        &all_data.contactor_check_state,
        &all_data.uptime
    );

    check_bricklet_reachable(rc, "update_all_data_struct");

    if (rc == TF_E_OK) {
        all_data.last_update = millis();
        all_data.is_valid = true;
    }
}

void EnergyManager::update_status_led()
{
    if (error_flags & ERROR_FLAGS_BAD_CONFIG_MASK)
        rgb_led.set_status(EmRgbLed::Status::BadConfig);
    else if (error_flags & ERROR_FLAGS_ALL_ERRORS_MASK)
        rgb_led.set_status(EmRgbLed::Status::Error);
    else if (error_flags & ERROR_FLAGS_ALL_WARNINGS_MASK)
        rgb_led.set_status(EmRgbLed::Status::Warning);
    else
        rgb_led.set_status(EmRgbLed::Status::OK);
}

void EnergyManager::clr_error(uint32_t error_mask)
{
    error_flags &= ~error_mask;
    state.get("error_flags")->updateUint(error_flags);
    update_status_led();
}

bool EnergyManager::is_error(uint32_t error_bit_pos)
{
    return (error_flags >> error_bit_pos) & 1;
}

void EnergyManager::set_error(uint32_t error_mask)
{
    error_flags |= error_mask;
    state.get("error_flags")->updateUint(error_flags);

    if (device_found)
        update_status_led();
}

void EnergyManager::set_config_error(uint32_t config_error_mask)
{
    config_error_flags |= config_error_mask;
    state.get("config_error_flags")->updateUint(config_error_flags);

    set_error(ERROR_FLAGS_BAD_CONFIG_MASK);
}

void EnergyManager::check_bricklet_reachable(int rc, const char *context)
{
    if (rc == TF_E_OK) {
        consecutive_bricklet_errors = 0;
        if (!bricklet_reachable) {
            bricklet_reachable = true;
            clr_error(ERROR_FLAGS_BRICKLET_MASK);
            logger.printfln("energy_manager: Bricklet is reachable again.");
        }
    } else {
        if (rc == TF_E_TIMEOUT) {
            logger.printfln("energy_manager (%s): Bricklet access timed out.", context);
        } else {
            logger.printfln("energy_manager (%s): Bricklet access returned error %d.", context, rc);
        }
        if (bricklet_reachable && ++consecutive_bricklet_errors >= 8) {
            bricklet_reachable = false;
            set_error(ERROR_FLAGS_BRICKLET_MASK);
            logger.printfln("energy_manager (%s): Bricklet is unreachable.", context);
        }
    }
    low_level_state.get("consecutive_bricklet_errors")->updateUint(consecutive_bricklet_errors);
}

void EnergyManager::start_network_check_task()
{
    task_scheduler.scheduleWithFixedDelay([this]() {
        bool disconnected;
        do {
#if MODULE_ETHERNET_AVAILABLE()
            if (ethernet.get_connection_state() == EthernetState::CONNECTED) {
                disconnected = false;
                break;
            }
#endif
#if MODULE_WIFI_AVAILABLE()
            if (wifi.get_connection_state() == WifiState::CONNECTED) {
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
            set_error(ERROR_FLAGS_NETWORK_MASK);
        } else {
            if (is_error(ERROR_FLAGS_NETWORK_BIT_POS))
                clr_error(ERROR_FLAGS_NETWORK_MASK);
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

    check_bricklet_reachable(rc, "get_sdcard_info");

    if (rc != TF_E_OK) {
        set_error(ERROR_FLAGS_SDCARD_MASK);
        logger.printfln("energy_manager: Failed to get SD card information. Error %i", rc);
        return false;
    }

    if (is_error(ERROR_FLAGS_SDCARD_BIT_POS))
        clr_error(ERROR_FLAGS_SDCARD_MASK);

    return true;
}

bool EnergyManager::format_sdcard()
{
    uint8_t ret_format_status;
    int rc = tf_warp_energy_manager_format_sd(&device, 0x4223ABCD, &ret_format_status);

    check_bricklet_reachable(rc, "format_sdcard");

    return rc == TF_E_OK && ret_format_status == TF_WARP_ENERGY_MANAGER_FORMAT_STATUS_OK;
}

uint16_t EnergyManager::get_energy_meter_detailed_values(float *ret_values)
{
    uint16_t len = 0;
    int rc = tf_warp_energy_manager_get_energy_meter_detailed_values(&device, ret_values, &len);

    check_bricklet_reachable(rc, "get_energy_meter_detailed_values");

    return rc == TF_E_OK ? len : 0;
}

bool EnergyManager::reset_energy_meter_relative_energy()
{
    int rc = tf_warp_energy_manager_reset_energy_meter_relative_energy(&device);

    check_bricklet_reachable(rc, "reset_energy_meter_relative_energy");

    return rc == TF_E_OK;
}

void EnergyManager::set_output(bool output_value)
{
    int result = tf_warp_energy_manager_set_output(&device, output_value);

    // Don't check if bricklet is reachable because the setter call won't tell us.

    if (result != TF_E_OK)
        logger.printfln("energy_manager: Failed to set output relay: error %i", result);
}

void EnergyManager::set_rgb_led(uint8_t pattern, uint16_t hue)
{
    int rc = tf_warp_energy_manager_set_led_state(&device, pattern, hue);

    // Don't check if bricklet is reachable because the setter call won't tell us.

    if (rc != TF_E_OK)
        logger.printfln("energy_manager: Failed to set LED state: error %i. Continuing anyway.", rc);
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

    logger.printfln("energy_manager: Failed to set datetime: error %i", rc);
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

        check_bricklet_reachable(rc, "get_time");

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

    logger.printfln("energy_manager: Failed to get datetime: error %i", rc);
    time.tv_sec = 0;
    return time;
}

void EnergyManager::update_grid_balance_led(EmRgbLed::GridBalance balance)
{
    rgb_led.update_grid_balance(balance);
}

bool EnergyManager::disallow_fw_update_with_vehicle_connected()
{
    return contactor_installed;
}
