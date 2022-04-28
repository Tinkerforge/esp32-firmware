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

#include "em_meter.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"
#include "task_scheduler.h"
#include "web_server.h"
#include "modules.h"

extern EventLog logger;

extern TF_HAL hal;
extern WebServer server;
extern TaskScheduler task_scheduler;
extern Config modules;

extern API api;

EMMeter::EMMeter()
{
    errors = Config::Object({
        {"local_timeout", Config::Uint32(0)},
        {"global_timeout", Config::Uint32(0)},
        {"illegal_function", Config::Uint32(0)},
        {"illegal_data_access", Config::Uint32(0)},
        {"illegal_data_value", Config::Uint32(0)},
        {"slave_device_failure", Config::Uint32(0)},
    });
}

void EMMeter::updateMeterValues()
{
    float power = energy_manager.all_data.power;
    energy_meter.updateMeterValues(energy_manager.all_data.power,
                                   energy_manager.all_data.energy_relative,
                                   energy_manager.all_data.energy_absolute);

    energy_meter.updateMeterPhases(energy_manager.all_data.phases_connected, energy_manager.all_data.phases_active);

    errors.get("local_timeout")->updateUint(energy_manager.all_data.error_count[0]);
    errors.get("global_timeout")->updateUint(energy_manager.all_data.error_count[1]);
    errors.get("illegal_function")->updateUint(energy_manager.all_data.error_count[2]);
    errors.get("illegal_data_access")->updateUint(energy_manager.all_data.error_count[3]);
    errors.get("illegal_data_value")->updateUint(energy_manager.all_data.error_count[4]);
    errors.get("slave_device_failure")->updateUint(energy_manager.all_data.error_count[5]);
}

void EMMeter::setupEM(bool update_module_initialized)
{
    energy_manager.update_all_data();

    uint8_t meter_type = energy_manager.all_data.energy_meter_type;

    if (meter_type == 0) {
        task_scheduler.scheduleOnce([this](){
            this->setupEM(true);
        }, 3000);
        return;
    }

    energy_meter.updateMeterState(2, meter_type);

    // We _have_ to update the meter values here:
    // Other modules may in their setup check if the meter feature is available
    // and if so, read the meter values.
    updateMeterValues();

    task_scheduler.scheduleWithFixedDelay([this](){
        this->updateMeterValues();
    }, 500, 500);

    task_scheduler.scheduleWithFixedDelay([this](){
        uint16_t len;
        float result[ALL_VALUES_COUNT] = {0};
        if (tf_warp_energy_manager_get_energy_meter_detailed_values(&energy_manager.device, result, &len) != TF_E_OK)
            return;

        energy_meter.updateMeterAllValues(result);
    }, 1000, 1000);

    initialized = true;

    if (update_module_initialized)
        modules.get("em_meter")->updateBool(true);
}

void EMMeter::setup()
{
    initialized = false;
    hardware_available = false;

    if (!energy_manager.initialized) {
        // If the Energy Manager is not initialized, we will never be able to reach the energy meter.
        return;
    }

    setupEM(false);
}

void EMMeter::register_urls()
{
    api.addState("meter/error_counters", &errors, {}, 1000);

    energy_meter.registerResetCallback([this]() {
        if (!initialized) {
            return;
        }

        tf_warp_energy_manager_reset_energy_meter_relative_energy(&energy_manager.device);
    });
}

void EMMeter::loop()
{
}
