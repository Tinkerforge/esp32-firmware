#include "sdm630_defs.h"
#include "module_dependencies.h"

#include "api.h"

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

static void sdm630_fast_read_done(const uint16_t *all_regs)
{
    static bool first_run = true;
    if (first_run) {
        first_run = false;
        sdm630_reset = Config::Float(0);
        api.restorePersistentConfig("meter/sdm630_reset", &sdm630_reset);
    }

    float fast_values[8];
    convert_to_float(all_regs, fast_values, sdm630_registers_fast_to_read, sizeof(sdm630_registers_fast_to_read) / sizeof(sdm630_registers_fast_to_read[0]));

    // TODO: Handle reset
    meter.updateMeterValues(fast_values[Power], fast_values[EnergyAbs] - sdm630_reset.asFloat(), fast_values[EnergyAbs]);

    bool phases_active[3] = {
        fast_values[CurrentPhase0] > PHASE_ACTIVE_CURRENT_THRES,
        fast_values[CurrentPhase1] > PHASE_ACTIVE_CURRENT_THRES,
        fast_values[CurrentPhase2] > PHASE_ACTIVE_CURRENT_THRES
    };

    bool phases_connected[3] = {
        fast_values[VoltagePhase0] > PHASE_CONNECTED_VOLTAGE_THRES,
        fast_values[VoltagePhase1] > PHASE_CONNECTED_VOLTAGE_THRES,
        fast_values[VoltagePhase2] > PHASE_CONNECTED_VOLTAGE_THRES
    };

    meter.updateMeterPhases(phases_connected, phases_active);
}

static void sdm630_slow_read_done(const uint16_t *all_regs)
{
    float all_values[METER_ALL_VALUES_COUNT];
    convert_to_float(all_regs, all_values, sdm630_registers_to_read, sizeof(sdm630_registers_to_read) / sizeof(sdm630_registers_to_read[0]));

    meter.updateMeterAllValues(all_values);
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
    [](){
        sdm630_reset.updateFloat(meter.values.get("energy_abs")->asFloat());
        api.writeConfig("meter/sdm630_reset", &sdm630_reset);
    }
};
