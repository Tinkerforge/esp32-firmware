#include "sdm72dm_defs.h"

static const RegRead sdm72dm_slow[]{};

static const RegRead sdm72dm_fast[]{
    {53, 2},  // power
    {343, 2}, // energy_abs
    {385, 2}  // energy_rel
};

enum FastValues {
    Power,
    EnergyAbs,
    EnergyRel
};

static const uint16_t sdm72dm_registers_fast_to_read[] = {
	53, 343, 385 // power, energy_abs, energy_rel
};

static void sdm72dm_fast_read_done(const uint16_t *all_regs)
{
    float fast_values[3];
    convert_to_float(all_regs, fast_values, sdm72dm_registers_fast_to_read, sizeof(sdm72dm_registers_fast_to_read) / sizeof(sdm72dm_registers_fast_to_read[0]));

    energy_meter.updateMeterValues(fast_values[Power], fast_values[EnergyRel], fast_values[EnergyAbs]);
}

static void sdm72dm_slow_read_done(const uint16_t *all_regs)
{
}

MeterInfo sdm72dm {
    0x0200, //0x0084 was told to us by eastron. However every SDM72DM we have here reports 0x0200 instead.
    1,
    sdm72dm_slow,
    sizeof(sdm72dm_slow) / sizeof(sdm72dm_slow[0]),
    sdm72dm_fast,
    sizeof(sdm72dm_fast) / sizeof(sdm72dm_fast[0]),
    sdm72dm_slow_read_done,
    sdm72dm_fast_read_done,
    "SDM72DM",
    nullptr
};


