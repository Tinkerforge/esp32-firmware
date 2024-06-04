#include <stdint.h>
#include <memory>

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
    int allocated_minimum_current_packets[4] = {};
};

#define PHASE_ROTATION(x, y, z) (0 << 6) | (((int)x << 4) | ((int)y << 2) | (int)z)

enum class ChargerPhase {
    PV,
    P1, // L1 if rotation is L123
    P2,
    P3
};

enum class GridPhase {
    PV,
    L1,
    L2,
    L3
};

enum class PhaseRotation {
    Unknown = 0, // Make unknown 0 so that memsetting the ChargerState struct sets this as expected.
    NotApplicable = 1, // Put this here so that enum >> 1 is the number of the first phase and enum & 1 is is the rotation
    L123 = PHASE_ROTATION(GridPhase::L1,GridPhase::L2,GridPhase::L3), // Standard Reference Phasing (= RST in OCPP)
    L132 = PHASE_ROTATION(GridPhase::L1,GridPhase::L3,GridPhase::L2), // Reversed Reference Phasing (= RTS in OCPP)
    L231 = PHASE_ROTATION(GridPhase::L2,GridPhase::L3,GridPhase::L1), // Standard 120 degree rotation (= STR in OCPP)
    L213 = PHASE_ROTATION(GridPhase::L2,GridPhase::L1,GridPhase::L3), // Reversed 240 degree rotation (= SRT in OCPP)
    L321 = PHASE_ROTATION(GridPhase::L3,GridPhase::L2,GridPhase::L1), // Reversed 120 degree rotation (= TSR in OCPP)
    L312 = PHASE_ROTATION(GridPhase::L3,GridPhase::L1,GridPhase::L2), // Standard 240 degree rotation (= TRS in OCPP)
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
