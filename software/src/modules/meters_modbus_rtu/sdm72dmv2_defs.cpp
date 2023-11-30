#include "sdm72dmv2_defs.h"
#include "module_dependencies.h"

static const RegRead sdm72dmv2_slow[] {
    {1, 76},
    {201, 26},
    {343, 4}
};

static const RegRead sdm72dmv2_fast[]{
    {1, 12},  // current per phase
    {53, 2},  // power
    {343, 2}, // energy_abs
    {385, 2}  // energy_rel
};

static const uint16_t sdm72dmv2_registers_to_read[] = {
	1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,47,49,53,57,61,63,71,73,75,201,203,205,207,225,343,345
};

static const bool sdm_registers_available_in_sdm72v2[] = {
	1,1,1,1,1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

enum FastValues {
    Power,
    EnergyAbs,
    EnergyRel,
    VoltagePhase0,
    VoltagePhase1,
    VoltagePhase2,
    CurrentPhase0,
    CurrentPhase1,
    CurrentPhase2,
};

static const uint16_t sdm72dmv2_registers_fast_to_read[] = {
	53, 343, 385, 1, 3, 5, 7, 9, 11 // power, energy_abs, energy_rel, voltage per phase, current per phase
};

static void sdm72dmv2_fast_read_done(const uint16_t *all_regs, uint32_t meter_slot, uint32_t idx_power, uint32_t idx_energy_rel, uint32_t idx_energy_abs, uint32_t idx_current_l1, uint32_t idx_voltage_l1)
{
    float fast_values[sizeof(sdm72dmv2_registers_fast_to_read)/sizeof(sdm72dmv2_registers_fast_to_read[0])];
    convert_to_float(all_regs, fast_values, sdm72dmv2_registers_fast_to_read, sizeof(sdm72dmv2_registers_fast_to_read) / sizeof(sdm72dmv2_registers_fast_to_read[0]));

    meters.update_value(meter_slot, idx_power,          fast_values[Power]);
    meters.update_value(meter_slot, idx_energy_abs,     fast_values[EnergyAbs]);
    // TODO
    //meters.update_value(meter_slot, idx_energy_rel,     fast_values[EnergyRel]);
    meters.update_value(meter_slot, idx_voltage_l1,     fast_values[VoltagePhase0]);
    meters.update_value(meter_slot, idx_voltage_l1 + 1, fast_values[VoltagePhase1]);
    meters.update_value(meter_slot, idx_voltage_l1 + 2, fast_values[VoltagePhase2]);
    meters.update_value(meter_slot, idx_current_l1,     fast_values[CurrentPhase0]);
    meters.update_value(meter_slot, idx_current_l1 + 1, fast_values[CurrentPhase1]);
    meters.update_value(meter_slot, idx_current_l1 + 2, fast_values[CurrentPhase2]);
}

static void sdm72dmv2_slow_read_done(const uint16_t *all_regs, uint32_t meter_slot)
{
    size_t read = 0;
    for (size_t i = 0; i < METER_ALL_VALUES_COUNT; ++i) {
        if (!sdm_registers_available_in_sdm72v2[i])
            continue;

        float val;
        convert_to_float(all_regs, &val, &sdm72dmv2_registers_to_read[read], 1),
        meters.update_value(meter_slot, read, val);
        ++read;
    }
}

MeterInfo sdm72dmv2 {
    0x0089,
    3,
    sdm72dmv2_slow,
    sizeof(sdm72dmv2_slow) / sizeof(sdm72dmv2_slow[0]),
    sdm72dmv2_fast,
    sizeof(sdm72dmv2_fast) / sizeof(sdm72dmv2_fast[0]),
    sdm72dmv2_slow_read_done,
    sdm72dmv2_fast_read_done,
    "SDM72DM-V2",
    nullptr
};
