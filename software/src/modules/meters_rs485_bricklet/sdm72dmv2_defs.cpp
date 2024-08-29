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

#include "sdm72dmv2_defs.h"

#include "module_dependencies.h"

static const RegRead sdm72dmv2_slow[] {
    {1, 76},
    {201, 26},
    {343, 4},
    {385, 8}
};

static const RegRead sdm72dmv2_fast[]{
    {7, 6},  // current per phase
    {53, 2},  // power
};

static const uint16_t sdm72dmv2_registers_to_read[] = {
	1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,43,47,49,53,57,61,63,71,73,75,201,203,205,207,225,343,345,385,389,391
};

enum FastValues {
    Power,
    CurrentPhase0,
    CurrentPhase1,
    CurrentPhase2,
};

static const uint16_t sdm72dmv2_registers_fast_to_read[] = {
	53, 7, 9, 11 // power, energy_abs, energy_rel, voltage per phase, current per phase
};

static void sdm72dmv2_fast_read_done(const uint16_t *all_regs, uint32_t meter_slot, uint32_t idx_power, uint32_t idx_energy_rel, uint32_t idx_energy_abs, uint32_t idx_current_l1, uint32_t idx_voltage_l1)
{
    float fast_values[ARRAY_SIZE(sdm72dmv2_registers_fast_to_read)];
    convert_to_float(all_regs, fast_values, sdm72dmv2_registers_fast_to_read, ARRAY_SIZE(sdm72dmv2_registers_fast_to_read));

    meters.update_value(meter_slot, idx_power,          fast_values[Power]);
    meters.update_value(meter_slot, idx_current_l1,     fast_values[CurrentPhase0]);
    meters.update_value(meter_slot, idx_current_l1 + 1, fast_values[CurrentPhase1]);
    meters.update_value(meter_slot, idx_current_l1 + 2, fast_values[CurrentPhase2]);

    meters.finish_update(meter_slot);
}

static void sdm72dmv2_slow_read_done(const uint16_t *all_regs, uint32_t meter_slot, ConfigRoot *reset)
{
    float all_values[ARRAY_SIZE(sdm72dmv2_registers_to_read)];
    convert_to_float(all_regs, all_values, sdm72dmv2_registers_to_read, ARRAY_SIZE(sdm72dmv2_registers_to_read));
    meters.update_all_values(meter_slot, all_values);
}

MeterInfo sdm72dmv2 {
    0x0089,
    3,
    sdm72dmv2_slow,
    ARRAY_SIZE(sdm72dmv2_slow),
    sdm72dmv2_fast,
    ARRAY_SIZE(sdm72dmv2_fast),
    sdm72dmv2_slow_read_done,
    sdm72dmv2_fast_read_done,
    "SDM72DM-V2",
    nullptr
};
