/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "evse_v2_meter.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"
#include "task_scheduler.h"
#include "web_server.h"
#include "modules.h"

void EVSEV2Meter::updateMeterValues()
{
    meter.updateMeterValues(evse_v2.energy_meter_values.get("power")->asFloat(),
                            evse_v2.energy_meter_values.get("energy_rel")->asFloat(),
                            evse_v2.energy_meter_values.get("energy_abs")->asFloat());

    bool phases_active[3];
    bool phases_connected[3];

    for (int i = 0; i < 3; ++i)
        phases_active[i] = evse_v2.energy_meter_values.get("phases_active")->get(i)->asBool();

    for (int i = 0; i < 3; ++i)
        phases_connected[i] = evse_v2.energy_meter_values.get("phases_connected")->get(i)->asBool();

    meter.updateMeterPhases(phases_connected, phases_active);
}

void EVSEV2Meter::setupEVSE(bool update_module_initialized)
{
    evse_v2.update_all_data();

    uint8_t meter_type = evse_v2.hardware_configuration.get("energy_meter_type")->asUint();
    if (meter_type == 0) {
        task_scheduler.scheduleOnce([this](){
            this->setupEVSE(true);
        }, 3000);
        return;
    }

    // We _have_ to update the meter values here:
    // Other modules may in their setup check if the meter feature is available
    // and if so, read the meter values.
    float result[METER_ALL_VALUES_COUNT] = {0};
    if (evse_v2.get_all_energy_meter_values(result) != METER_ALL_VALUES_COUNT) {
        task_scheduler.scheduleOnce([this](){
            this->setupEVSE(true);
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
        if (evse_v2.get_all_energy_meter_values(inner_result) != METER_ALL_VALUES_COUNT)
            return;

        meter.updateMeterAllValues(inner_result);
    }, 1000, 1000);

    initialized = true;

    if (update_module_initialized)
        modules.get("evse_v2_meter")->updateBool(true);
}

void EVSEV2Meter::setup()
{
    if (!evse_v2.initialized) {
        // If the EVSE is not initialized, we will never be able to reach the energy meter.
        return;
    }

    setupEVSE(false);
}

void EVSEV2Meter::register_urls()
{
    api.addState("meter/error_counters", &evse_v2.energy_meter_errors, {}, 1000);

    meter.registerResetCallback([this]() {
        if (!initialized) {
            return;
        }

        evse_v2.reset_energy_meter_relative_energy();
    });
}
