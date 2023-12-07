#include "sdm630_defs.h"
#include "module_dependencies.h"

#include "api.h"

// FIXME: Move this into the meter_rs485_bricklet?
static ConfigRoot sdm630_reset;

static const RegRead sdm630_slow[] {
    {1, 88},
    {101, 8},
    {201, 70},
    {335, 48}
};

static const RegRead sdm630_fast[]{
    {1, 12}, // current per phase
    {53, 2}, // power
    {343, 2} // energy_abs
};

static const uint16_t sdm630_registers_to_read[] = {
	1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,47,49,53,57,61,63,67,71,73,75,77,79,81,83,85,87,101,103,105,107,201,203,205,207,225,235,237,239,241,243,245,249,251,259,261,263,265,267,269,335,337,339,341,343,345,347,349,351,353,355,357,359,361,363,365,367,369,371,373,375,377,379,381
};

enum FastValues {
    Power,
    EnergyAbs,
    VoltagePhase0,
    VoltagePhase1,
    VoltagePhase2,
    CurrentPhase0,
    CurrentPhase1,
    CurrentPhase2,
};

static const uint16_t sdm630_registers_fast_to_read[] = {
	53, 343, 1, 3, 5, 7, 9, 11 // power, energy_abs, voltage per phase, current per phase
};

static void sdm630_fast_read_done(const uint16_t *all_regs, uint32_t meter_slot, uint32_t idx_power, uint32_t idx_energy_rel, uint32_t idx_energy_abs, uint32_t idx_current_l1, uint32_t idx_voltage_l1)
{
    static bool first_run = true;
    if (first_run) {
        first_run = false;
        sdm630_reset = Config::Float(0);
        api.restorePersistentConfig(meters.get_path(meter_slot, Meters::PathType::Base) + "sdm630_reset", &sdm630_reset);
    }

    float fast_values[8];
    convert_to_float(all_regs, fast_values, sdm630_registers_fast_to_read, sizeof(sdm630_registers_fast_to_read) / sizeof(sdm630_registers_fast_to_read[0]));

    meters.update_value(meter_slot, idx_power,          fast_values[Power]);
    meters.update_value(meter_slot, idx_energy_abs,     fast_values[EnergyAbs]);
    // TODO
    //meters.update_value(meter_slot, idx_energy_rel,     fast_values[EnergyAbs] - sdm630_reset.asFloat());
    meters.update_value(meter_slot, idx_voltage_l1,     fast_values[VoltagePhase0]);
    meters.update_value(meter_slot, idx_voltage_l1 + 1, fast_values[VoltagePhase1]);
    meters.update_value(meter_slot, idx_voltage_l1 + 2, fast_values[VoltagePhase2]);
    meters.update_value(meter_slot, idx_current_l1,     fast_values[CurrentPhase0]);
    meters.update_value(meter_slot, idx_current_l1 + 1, fast_values[CurrentPhase1]);
    meters.update_value(meter_slot, idx_current_l1 + 2, fast_values[CurrentPhase2]);
}

static void sdm630_slow_read_done(const uint16_t *all_regs, uint32_t meter_slot)
{
    float all_values[METER_ALL_VALUES_COUNT];
    convert_to_float(all_regs, all_values, sdm630_registers_to_read, sizeof(sdm630_registers_to_read) / sizeof(sdm630_registers_to_read[0]));

    meters.update_all_values(meter_slot, all_values);
}

MeterInfo sdm630 {
    0x0070,
    2,
    sdm630_slow,
    sizeof(sdm630_slow) / sizeof(sdm630_slow[0]),
    sdm630_fast,
    sizeof(sdm630_fast) / sizeof(sdm630_fast[0]),
    sdm630_slow_read_done,
    sdm630_fast_read_done,
    "SDM630",
    [](uint32_t meter_slot){
        logger.printfln("SDM630 reset not implemented yet!");
        // TODO: Why is meters.get_single_value private?
        sdm630_reset.updateFloat(0);
        api.writeConfig(meters.get_path(meter_slot, Meters::PathType::Base) + "sdm630_reset", &sdm630_reset);
    }
};
