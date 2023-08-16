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
#include "modules/meters/meter_value_id.h"
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
}

void MeterEM::register_urls(String base_url)
{
    IMeter::register_urls(base_url);

    api.addState(base_url + "/error_counters", &errors, {}, 1000);
}

void MeterEM::update_from_em_all_data(EnergyManagerAllData &all_data)
{
    if (all_data.energy_meter_type == METER_TYPE_NONE)
        return;

    if (!first_values_seen) {
        uint32_t ids[] = {
            static_cast<uint32_t>(MeterValueID::VoltageL1N),
            static_cast<uint32_t>(MeterValueID::VoltageL2N),
            static_cast<uint32_t>(MeterValueID::VoltageL3N),
            static_cast<uint32_t>(MeterValueID::PowerActiveLSumImExDiff),
            static_cast<uint32_t>(MeterValueID::EnergyActiveLSumImport),
            static_cast<uint32_t>(MeterValueID::EnergyActiveLSumExport),
        };
        meters.declare_value_ids(slot, ids, ARRAY_SIZE(ids));

        value_index_power  = 3;
        value_index_import = 4;
        value_index_export = 5;

        task_scheduler.scheduleWithFixedDelay([this](){
            update_all_values();
        }, 0, 5000);

        first_values_seen = true;
    }

    state->get("type")->updateUint(all_data.energy_meter_type);

    errors.get("local_timeout"       )->updateUint(all_data.error_count[0]);
    errors.get("global_timeout"      )->updateUint(all_data.error_count[1]);
    errors.get("illegal_function"    )->updateUint(all_data.error_count[2]);
    errors.get("illegal_data_access" )->updateUint(all_data.error_count[3]);
    errors.get("illegal_data_value"  )->updateUint(all_data.error_count[4]);
    errors.get("slave_device_failure")->updateUint(all_data.error_count[5]);

    float power = all_data.power;
    meters.update_value(slot, value_index_power,  power);
    meters.update_value(slot, value_index_import, all_data.energy_import);
    meters.update_value(slot, value_index_export, all_data.energy_export);

    //TODO API change: replace import/export with phase currents
    //METER_ALL_VALUES_CURRENT_L1_A
    //METER_ALL_VALUES_CURRENT_L2_A
    //METER_ALL_VALUES_CURRENT_L3_A

    power_hist.add_sample(power);
}

void MeterEM::update_all_values()
{
    float new_values[METER_ALL_VALUES_COUNT] = {NAN};
    if (energy_manager.get_energy_meter_detailed_values(new_values) != METER_ALL_VALUES_COUNT)
        return;

    float all_values_update[] = {
        new_values[METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L1],
        new_values[METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L2],
        new_values[METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L3],
        new_values[METER_ALL_VALUES_TOTAL_SYSTEM_POWER_W],
        new_values[METER_ALL_VALUES_TOTAL_IMPORT_KWH],
        new_values[METER_ALL_VALUES_TOTAL_EXPORT_KWH],
    };

    meters.update_all_values(slot, all_values_update);
}
