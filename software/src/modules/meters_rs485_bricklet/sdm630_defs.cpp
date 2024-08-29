/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

#include "sdm630_defs.h"

#include "module_dependencies.h"
#include "modules/meters/sdm_helpers.h"

static const RegRead sdm630_slow[] {
    {1, 88},
    {101, 8},
    {201, 70},
    {335, 48}
};

static const RegRead sdm630_fast[]{
    {7, 6}, // current per phase
    {53, 2}, // power
};

static const uint16_t sdm630_registers_to_read[] = {
	1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,47,49,53,57,61,63,67,71,73,75,77,79,81,83,85,87,101,103,105,107,201,203,205,207,225,235,237,239,241,243,245,249,251,259,261,263,265,267,269,335,337,339,341,343,345,347,349,351,353,355,357,359,361,363,365,367,369,371,373,375,377,379,381
};

enum FastValues {
    Power,
    CurrentPhase0,
    CurrentPhase1,
    CurrentPhase2,
};

static const uint16_t sdm630_registers_fast_to_read[] = {
	53, 7, 9, 11 // power, current per phase
};

static void sdm630_fast_read_done(const uint16_t *all_regs, uint32_t meter_slot, uint32_t idx_power, uint32_t idx_energy_rel, uint32_t idx_energy_abs, uint32_t idx_current_l1, uint32_t idx_voltage_l1)
{
    float fast_values[7];
    convert_to_float(all_regs, fast_values, sdm630_registers_fast_to_read, ARRAY_SIZE(sdm630_registers_fast_to_read));

    meters.update_value(meter_slot, idx_power,          fast_values[Power]);
    meters.update_value(meter_slot, idx_current_l1,     fast_values[CurrentPhase0]);
    meters.update_value(meter_slot, idx_current_l1 + 1, fast_values[CurrentPhase1]);
    meters.update_value(meter_slot, idx_current_l1 + 2, fast_values[CurrentPhase2]);

    meters.finish_update(meter_slot);
}

static void sdm630_slow_read_done(const uint16_t *all_regs, uint32_t meter_slot, ConfigRoot *reset)
{
    static bool first_run = true;
    if (first_run) {
        first_run = false;
        api.restorePersistentConfig(meters.get_path(meter_slot, Meters::PathType::Base) + "sdm630_reset", reset);
    }

    float all_values[METER_ALL_VALUES_RESETTABLE_COUNT];
    convert_to_float(all_regs, all_values, sdm630_registers_to_read, ARRAY_SIZE(sdm630_registers_to_read));
    all_values[METER_ALL_VALUES_RESETTABLE_COUNT - 3] = all_values[METER_ALL_VALUES_TOTAL_KWH_SUM] - reset->get("energy_total")->asFloat();
    all_values[METER_ALL_VALUES_RESETTABLE_COUNT - 2] = all_values[METER_ALL_VALUES_TOTAL_IMPORT_KWH] - reset->get("energy_import")->asFloat();
    all_values[METER_ALL_VALUES_RESETTABLE_COUNT - 1] = all_values[METER_ALL_VALUES_TOTAL_EXPORT_KWH] - reset->get("energy_export")->asFloat();

    size_t values = METER_ALL_VALUES_RESETTABLE_COUNT;
    sdm_helper_pack_all_values(METER_TYPE_SDM630, all_values, &values);
    meters.update_all_values(meter_slot, all_values);
}

MeterInfo sdm630 {
    0x0070,
    2,
    sdm630_slow,
    ARRAY_SIZE(sdm630_slow),
    sdm630_fast,
    ARRAY_SIZE(sdm630_fast),
    sdm630_slow_read_done,
    sdm630_fast_read_done,
    "SDM630",
    [](uint32_t meter_slot, ConfigRoot *reset){
        float total = 0;
        float import = 0;
        float export_ = 0;

        meters.get_energy_imexsum(meter_slot, &total);
        meters.get_energy_import(meter_slot, &import);
        meters.get_energy_export(meter_slot, &export_);

        reset->get("energy_total")->updateFloat(total);
        reset->get("energy_import")->updateFloat(import);
        reset->get("energy_export")->updateFloat(export_);

        api.writeConfig(meters.get_path(meter_slot, Meters::PathType::Base) + "sdm630_reset", reset);
    }
};
