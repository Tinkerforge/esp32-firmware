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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modules/meters/meter_value_id.h"
#include "modules/meters/sdm_helpers.h"
#include "tools.h"

#include "gcc_warnings.h"

[[gnu::const]]
MeterClassID MeterEM::get_class() const
{
    return MeterClassID::EnergyManager;
}

void MeterEM::update_from_em_all_data(const EMAllDataCommon &all_data)
{
    // Reject stale data older than five seconds.
    if (deadline_elapsed(all_data.last_update + 5 * 1000))
        return;

    // Always update error counters, even if no meter could be detected.
    errors->get("local_timeout"       )->updateUint(all_data.error_count[0]);
    errors->get("global_timeout"      )->updateUint(all_data.error_count[1]);
    errors->get("illegal_function"    )->updateUint(all_data.error_count[2]);
    errors->get("illegal_data_access" )->updateUint(all_data.error_count[3]);
    errors->get("illegal_data_value"  )->updateUint(all_data.error_count[4]);
    errors->get("slave_device_failure")->updateUint(all_data.error_count[5]);

    // No data to handle if no meter was detected.
    if (all_data.energy_meter_type == METER_TYPE_NONE)
        return;

    if (meter_type != all_data.energy_meter_type) {
        if (meter_type != METER_TYPE_NONE) {
            // Don't print warning if this is a not-none -> none transition.
            // This happens if the EVSE restarts without the ESP also restarting.
            // The meter will be detected again in a few seconds.
            if (!meter_change_warning_printed && all_data.energy_meter_type != METER_TYPE_NONE) {
                logger.printfln("Meter change detected. This is not supported.");
                meter_change_warning_printed = true;
            }
            return;
        }

        // No need to initialize the array because either all values are written or it is rejected entirely.
        float all_values[METER_ALL_VALUES_RESETTABLE_COUNT];
        if (em_common.get_energy_meter_detailed_values(all_values) != METER_ALL_VALUES_RESETTABLE_COUNT)
            return;

        meter_type = all_data.energy_meter_type;
        state->get("type")->updateUint(meter_type);

        MeterValueID ids[METER_ALL_VALUES_RESETTABLE_COUNT];
        uint32_t id_count = METER_ALL_VALUES_RESETTABLE_COUNT;
        sdm_helper_parse_values(meter_type, all_values, &id_count, ids, value_packing_cache);
        value_count = id_count;
        meters.declare_value_ids(slot, ids, id_count);

        value_index_power       = meters_find_id_index(ids, id_count, MeterValueID::PowerActiveLSumImExDiff);
        value_index_currents[0] = meters_find_id_index(ids, id_count, MeterValueID::CurrentL1ImExSum);
        value_index_currents[1] = meters_find_id_index(ids, id_count, MeterValueID::CurrentL2ImExSum);
        value_index_currents[2] = meters_find_id_index(ids, id_count, MeterValueID::CurrentL3ImExSum);

        update_all_values(all_values);

        task_scheduler.scheduleWithFixedDelay([this](){
            update_all_values(nullptr);
        }, 990_ms, 990_ms);

        return;
    }

    meters.update_value(slot, value_index_power, all_data.power);
    for (size_t i = 0; i < ARRAY_SIZE(value_index_currents); i++) {
        meters.update_value(slot, value_index_currents[i], all_data.current[i]);
    }

    meters.finish_update(slot);
}

void MeterEM::update_all_values(float *values)
{
    // No need to initialize the array because either all values are written or it is rejected entirely.
    float local_values[METER_ALL_VALUES_RESETTABLE_COUNT];

    if (!values) {
        values = local_values;
        if (em_common.get_energy_meter_detailed_values(values) != METER_ALL_VALUES_RESETTABLE_COUNT)
            return;
    }

    sdm_helper_pack_all_values(values, value_count, value_packing_cache);

    meters.update_all_values(slot, values);
}

bool MeterEM::reset()
{
    return em_common.reset_energy_meter_relative_energy();
}
