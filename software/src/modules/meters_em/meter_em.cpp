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

#include "modules/meters/meter_value_id.h"
#include "modules/meters/sdm_helpers.h"
#include "task_scheduler.h"
#include "tools.h"

#include "gcc_warnings.h"

_ATTRIBUTE((const))
MeterClassID MeterEM::get_class() const
{
    return MeterClassID::EnergyManager;
}

void MeterEM::update_from_em_all_data(EnergyManagerAllData &all_data)
{
    // Reject stale data older than five seconds.
    if (deadline_elapsed(all_data.last_update + 5 * 1000))
        return;

    // Do nothing if no meter was detected.
    if (all_data.energy_meter_type == METER_TYPE_NONE)
        return;

    if (meter_type != all_data.energy_meter_type) {
        if (meter_type != METER_TYPE_NONE) {
            if (!meter_change_warning_printed) {
                logger.printfln("meter_em: Meter change detected. This is not supported.");
                meter_change_warning_printed = true;
            }
            return;
        }

        meter_type = all_data.energy_meter_type;
        state->get("type")->updateUint(meter_type);

        MeterValueID ids[METER_ALL_VALUES_RESETTABLE_COUNT];
        uint32_t id_count = METER_ALL_VALUES_RESETTABLE_COUNT;
        sdm_helper_get_value_ids(meter_type, ids, &id_count);
        meters.declare_value_ids(slot, ids, id_count);

        value_index_power      = meters_find_id_index(ids, id_count, MeterValueID::PowerActiveLSumImExDiff);
        value_index_current[0] = meters_find_id_index(ids, id_count, MeterValueID::CurrentL1ImExSum);
        value_index_current[1] = meters_find_id_index(ids, id_count, MeterValueID::CurrentL2ImExSum);
        value_index_current[2] = meters_find_id_index(ids, id_count, MeterValueID::CurrentL3ImExSum);

        task_scheduler.scheduleWithFixedDelay([this](){
            update_all_values();
        }, 0, 990);
    }

    errors->get("local_timeout"       )->updateUint(all_data.error_count[0]);
    errors->get("global_timeout"      )->updateUint(all_data.error_count[1]);
    errors->get("illegal_function"    )->updateUint(all_data.error_count[2]);
    errors->get("illegal_data_access" )->updateUint(all_data.error_count[3]);
    errors->get("illegal_data_value"  )->updateUint(all_data.error_count[4]);
    errors->get("slave_device_failure")->updateUint(all_data.error_count[5]);

    meters.update_value(slot, value_index_power,  all_data.power);
    for (uint32_t i = 0; i < ARRAY_SIZE(value_index_current); i++) {
        meters.update_value(slot, value_index_current[i], all_data.current[i]);
    }
}

void MeterEM::update_all_values()
{
    // No need to initialize the array because either all values are written or it is rejected entirely.
    float values[METER_ALL_VALUES_RESETTABLE_COUNT];
    if (energy_manager.get_energy_meter_detailed_values(values) != METER_ALL_VALUES_RESETTABLE_COUNT)
        return;

    uint32_t values_len = ARRAY_SIZE(values);
    sdm_helper_pack_all_values(meter_type, values, &values_len);

    if (values_len == 0) {
        logger.printfln("meter_em: Cannot pack values into array of size %u.", ARRAY_SIZE(values));
    } else {
        meters.update_all_values(slot, values);
    }
}

bool MeterEM::reset()
{
    return energy_manager.reset_energy_meter_relative_energy();
}
