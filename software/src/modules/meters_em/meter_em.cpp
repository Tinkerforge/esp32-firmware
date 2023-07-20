/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "meter_em.h"
#include "module_dependencies.h"

#include "modules/meters/meter_class_defs.h"
#include "task_scheduler.h"

#include "gcc_warnings.h"

_ATTRIBUTE((const))
uint32_t MeterEM::get_class() const
{
    return METER_CLASS_LOCAL_EM;
}

void MeterEM::setup()
{
    IMeter::setup();

    errors = Config::Object({
        {"local_timeout",        Config::Uint32(0)},
        {"global_timeout",       Config::Uint32(0)},
        {"illegal_function",     Config::Uint32(0)},
        {"illegal_data_access",  Config::Uint32(0)},
        {"illegal_data_value",   Config::Uint32(0)},
        {"slave_device_failure", Config::Uint32(0)},
    });

    all_values_conf = Config::Array({},
        new Config{Config::Float(NAN)},
        0, METER_ALL_VALUES_COUNT, Config::type_id<Config::ConfFloat>()
    );

    for (ssize_t i = all_values_conf.count(); i < METER_ALL_VALUES_COUNT; i++) {
        all_values_conf.add();
    }

    all_values_names = Config::Array({},
        new Config(Config::Str("", 0, 64)),
        METER_ALL_VALUES_COUNT,
        METER_ALL_VALUES_COUNT,
        Config::type_id<Config::ConfString>()
    );
    for (ssize_t i = all_values_names.count(); i < METER_ALL_VALUES_COUNT; i++) {
        all_values_names.add();
        all_values_names.get(static_cast<uint16_t>(i))->updateString(meter_all_values_names[i]);
    }
}

void MeterEM::register_urls(String base_url)
{
    IMeter::register_urls(base_url);

    api.addState(base_url + "/error_counters", &errors, {}, 1000);
    api.addState(base_url + "/all_values", &all_values_conf, {}, 1000);
    api.addState(base_url + "/all_values_names", &all_values_names, {}, 1000);
}

bool MeterEM::get_power(float *power_w)
{
    float power = all_values_float[METER_ALL_VALUES_TOTAL_SYSTEM_POWER_W];

    if (isnan(power))
        return false;

    *power_w = power;
    return true;
}

bool have_values = false;

bool MeterEM::get_import_export(float *energy_import_kwh, float *energy_export_kwh)
{
    return have_values;
}

bool MeterEM::get_line_currents(float *l1_current_ma, float *l2_current_ma, float *l3_current_ma)
{
    return have_values;
}

void MeterEM::update_from_em_all_data(EnergyManagerAllData &all_data)
{
    if (all_data.energy_meter_type != METER_TYPE_NONE) {
        state->get("type")->updateUint(all_data.energy_meter_type);

        errors.get("local_timeout"       )->updateUint(all_data.error_count[0]);
        errors.get("global_timeout"      )->updateUint(all_data.error_count[1]);
        errors.get("illegal_function"    )->updateUint(all_data.error_count[2]);
        errors.get("illegal_data_access" )->updateUint(all_data.error_count[3]);
        errors.get("illegal_data_value"  )->updateUint(all_data.error_count[4]);
        errors.get("slave_device_failure")->updateUint(all_data.error_count[5]);

        float power = all_data.power;
        all_values_float[METER_ALL_VALUES_TOTAL_SYSTEM_POWER_W] = power;
        all_values_conf.get(METER_ALL_VALUES_TOTAL_SYSTEM_POWER_W)->updateFloat(power);

        //TODO API change: replace import/export with phase currents
        //METER_ALL_VALUES_CURRENT_L1_A
        //METER_ALL_VALUES_CURRENT_L2_A
        //METER_ALL_VALUES_CURRENT_L3_A

        power_hist.add_sample(power);

        if (!all_values_task_started) {
            task_scheduler.scheduleWithFixedDelay([this](){
                update_all_values();
            }, 0, 5000);
            all_values_task_started = true;
        }
    }
}

void MeterEM::update_all_values()
{
    float new_values[METER_ALL_VALUES_COUNT] = {NAN};
    if (energy_manager.get_energy_meter_detailed_values(new_values) != METER_ALL_VALUES_COUNT)
        return;

    bool changed = false;

    for (uint16_t i = 0; i < METER_ALL_VALUES_COUNT; ++i) {
        if (!isnan(new_values[i])) {
            changed |= all_values_conf.get(i)->updateFloat(new_values[i]) && !isnan(all_values_float[i]);
            all_values_float[i] = new_values[i];
        }
    }

    (void)changed; //TODO Use for require_meter or something?
}
