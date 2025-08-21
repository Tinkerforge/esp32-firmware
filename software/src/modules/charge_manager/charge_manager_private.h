#pragma once

#include <stdint.h>
#include <memory>

#include "current_limits.h"
#include "allocator_decision.enum.h"
#include "global_allocator_decision.enum.h"

#include "tools.h"

#define DISTRIBUTION_LOG_LEN 2048

#define CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE 128
#define CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE 129
#define CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE 130
#define CHARGE_MANAGER_CLIENT_ERROR_START 192

struct ChargerDecision {
    AllocatorDecision decision;
    int param_1;
    int param_2;
    int param_3;
};

struct GlobalDecision {
    GlobalAllocatorDecision decision;
    int param_1;
    int param_2;
    int param_3;
};


// Initialized by charge_manager.cpp; never changed
struct CurrentAllocatorConfig {
    // How often to run the allocation algorithm
    micros_t allocation_interval;

    // Only switch phases, start or stop chargers if this is elapsed.
    // Don't reset hysteresis when stopping chargers:
    // Stopping and immediately starting again is fine, as is done when switching phases.
    micros_t global_hysteresis;

    // Allow charging for this time to attempt to wake-up a "full" vehicle,
    // i.e. one that triggered a C -> B2 transition and/or waited in B2 for too long
    micros_t wakeup_time;

    // Require a charger to be active this long before clearing just_plugged_in_timestamp.
    micros_t plug_in_time;

    seconds_t rotation_interval;

    int minimum_current_3p;
    int minimum_current_1p;
    float enable_current_factor;
    std::unique_ptr<char[]> distribution_log;
    size_t distribution_log_len;
    size_t charger_count;
    uint16_t requested_current_margin;
    uint16_t requested_current_threshold;
};

// R+W _only_ by current_allocator.cpp
struct CurrentAllocatorState {
    bool last_print_local_log_was_error = false;

    bool global_hysteresis_elapsed = false;
    micros_t last_hysteresis_reset = 0_us;

    micros_t next_rotation = 0_us;

    Cost control_window_min = {0, 0, 0, 0};
    Cost control_window_max = {0, 0, 0, 0};
};

namespace ChargeMode {
    enum Type {
        _min = 1,
        PV = 1,
        Min = 2,
        Eco = 4,
        Fast = 8,
        _max = 8
    };
    const char * const Strings[Type::_max + 1] {
        "Off",
        "PV",
        "Min",
        "Min+PV",
        "Eco",
        "Eco+PV",
        "Eco+Min",
        "Eco+Min+PV",
        "Fast"
    };
}

// Check alignment/padding when adding stuff.
struct ChargerState {
    micros_t last_update;
    uint32_t uid;
    uint32_t uptime;
    uint32_t power_total_count;
    float power_total_sum;
    float energy_abs;

    // maximum current supported by the charger
    uint16_t supported_current;

    // last current limit reported by the charger
    uint16_t allowed_current;

    // requested current calculated with the line currents reported by the charger
    uint16_t requested_current;

    // 0 - no vehicle, 1 - user blocked, 2 - manager blocked, 3 - car blocked, 4 - charging, 5 - error, 6 - charged
    //uint8_t state;

    // 0 - okay, 1 - unreachable, 2 - FW mismatch, 3 - not managed
    //uint8_t error;
    uint8_t charger_state;
    bool wants_to_charge;
    bool wants_to_charge_low_priority;
    bool is_charging;

    // last CP disconnect support reported by the charger: false - CP disconnect not supported, true - CP disconnect supported
    bool cp_disconnect_supported;

    // last CP disconnect state reported by the charger: false - automatic, true - disconnected
    bool cp_disconnect_state;

    bool meter_supported;

    bool phase_switch_supported;

    // TODO move everything below into charger allocation state.

    // Phases that are currently used or will be used if current is allocated.
    uint8_t phases;

    PhaseRotation phase_rotation;

    float allocated_energy;
    float allocated_average_power;

    // The last time we've allocated more than 0 phases to this charger.
    micros_t last_switch_on;

    // This is the last time that either
    // - we've changed the phase allocation of this charger
    // - the contactor is switched on
    // In both cases, we block another phase switch
    // (i.e. change of the phase allocation except to/from 0)
    // for one hysteresis, to make sure the vehicle does not
    // observe a phase switch directly after the contactor is switched on.
    micros_t last_phase_switch;

    // The last time a vehicle was plugged in.
    // If this is not 0, this charger has to be allocated current to as fast as possible,
    // or else the vehicle could power down its charge controller.
    // Is set to 0 the first time a charger gets current allocated.
    micros_t just_plugged_in_timestamp;

    // The last time a vehicle was plugged in or 0 if no car is currently plugged in.
    micros_t last_plug_in;

    micros_t last_wakeup;

    // If set, the last allocation was less than the requested current.
    // Ignore phase currents in this case for a faster ramp up.
    micros_t use_supported_current;

    // Incremented every iteration the connected vehicle was in state C.
    micros_t time_in_state_c;

    uint8_t charge_mode;

    // Those are translated from the charge_mode before allocate_current.
    // We should not overwrite charge_mode (for example for the Eco -> Fast/Off decision)
    bool off;
    bool observe_pv_limit;
    bool eco_fast;
    int guaranteed_pv_current;
};

struct ChargerAllocationState {
    micros_t last_sent_config;

    // last current limit send to the charger
    uint16_t allocated_current;

    int8_t allocated_phases;

    // last CP disconnect request sent to charger: false - automatic/don't care, true - disconnect
    bool cp_disconnect;

    // 0 - okay, 1 - unreachable, 2 - FW mismatch, 3 - not managed
    uint8_t error;

    // 0 - no vehicle, 1 - user blocked, 2 - manager blocked, 3 - car blocked, 4 - charging, 5 - error, 6 - charged
    uint8_t state;
};
