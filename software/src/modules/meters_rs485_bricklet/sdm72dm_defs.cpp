#include "sdm72dm_defs.h"
#include "module_dependencies.h"

static const RegRead sdm72dm_slow[]{};

static const RegRead sdm72dm_fast[]{
    {53, 2},  // total system power
    {73, 4},  // total import/export kWh
    {343, 2}, // energy_abs
    {385, 8},  // energy_rel, [...], resettable import/export
};

static const uint16_t sdm72dm_registers_fast_to_read[] = {
	53, 73, 75, 343, 385, 389, 391
};

static void sdm72dm_fast_read_done(const uint16_t *all_regs, uint32_t meter_slot, uint32_t idx_power, uint32_t idx_energy_rel, uint32_t idx_energy_abs, uint32_t idx_current_l1, uint32_t idx_voltage_l1)
{
    float fast_values[7];
    convert_to_float(all_regs, fast_values, sdm72dm_registers_fast_to_read, sizeof(sdm72dm_registers_fast_to_read) / sizeof(sdm72dm_registers_fast_to_read[0]));
    meters.update_all_values(meter_slot, fast_values);
}

static void sdm72dm_slow_read_done(const uint16_t *all_regs, uint32_t meter_slot, ConfigRoot *reset)
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
