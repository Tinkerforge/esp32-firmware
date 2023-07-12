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
#include "module_dependencies.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"
#include "task_scheduler.h"
#include "web_server.h"

void EMMeter::pre_setup()
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
    meter.updateMeterValues(energy_manager.all_data.power, 0, 0);

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

    // We _have_ to update the meter values here:
    // Other modules may in their setup check if the meter feature is available
    // and if so, read the meter values.
    float result[METER_ALL_VALUES_COUNT] = {0};
    if (energy_manager.get_energy_meter_detailed_values(result) != METER_ALL_VALUES_COUNT) {
        task_scheduler.scheduleOnce([this](){
            this->setupEM(true);
        }, 3000);
        return;
    }

    meter.updateMeterState(2, meter_type);
    updateMeterValues();
    meter.updateMeterAllValues(result);

    task_scheduler.scheduleWithFixedDelay([this](){
        this->updateMeterValues();
    }, 500, 500);

    task_scheduler.scheduleWithFixedDelay([this](){
        float inner_result[METER_ALL_VALUES_COUNT] = {0};
        if (energy_manager.get_energy_meter_detailed_values(inner_result) != METER_ALL_VALUES_COUNT)
            return;

        meter.updateMeterAllValues(inner_result);
    }, 1000, 1000);

    bool triple_true[3] = {true, true, true};
    meter.updateMeterPhases(triple_true, triple_true);

    initialized = true;

    if (update_module_initialized)
        modules.get("em_meter")->updateBool(true);
}

void EMMeter::setup()
{
    if (!energy_manager.initialized) {
        // If the Energy Manager is not initialized, we will never be able to reach the energy meter.
        return;
    }

    setupEM(false);
}

void EMMeter::register_urls()
{
    api.addState("meter/error_counters", &errors, {}, 1000);
}
