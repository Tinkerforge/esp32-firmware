#include <stdint.h>
#include <memory>

#include "current_limits.h"

#define DISTRIBUTION_LOG_LEN 2048

#define CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE 128
#define CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE 129
#define CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE 130
#define CHARGE_MANAGER_CLIENT_ERROR_START 192

// Initialized by charge_manager.cpp; never changed
struct CurrentAllocatorConfig {
    int32_t minimum_current_3p;
    int32_t minimum_current_1p;
    int32_t enable_current;
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
    bool reset_global_hysteresis = false;
    Cost allocated_minimum_current_packets;
};

struct ChargerState {
    uint32_t last_update;
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

    // Phases that are currently used or will be used if current is allocated.
    uint8_t phases;

    PhaseRotation phase_rotation;
};

struct ChargerAllocationState {
    uint32_t last_sent_config;

    // last current limit send to the charger
    uint16_t allocated_current;
    // last CP disconnect request sent to charger: false - automatic/don't care, true - disconnect
    bool cp_disconnect;

    // 0 - okay, 1 - unreachable, 2 - FW mismatch, 3 - not managed
    uint8_t error;

    // 0 - no vehicle, 1 - user blocked, 2 - manager blocked, 3 - car blocked, 4 - charging, 5 - error, 6 - charged
    uint8_t state;
};
