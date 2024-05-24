#include "tests.h"

#include "current_allocator.h"
#include "string.h"

#include "modules/cm_networking/cm_networking_defs.h"

#include <iostream>

#define _assert(x, eq, y) do {auto _x = (x); auto _y = (y); if (!(_x eq _y)){ std::cout << __FILE__ << ':' << __LINE__ << ' ' << __func__ << " Assertion \x1b[31m" << #x << ' ' << #eq << ' ' << #y "\x1b[00m failed: !(" << _x << ' ' << #eq << ' ' << _y << ")\n";return;}} while(0)

#define MINIMUM_CURRENT 6000
#define ENABLE_CURRENT 9000

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreorder-init-list"
const ChargerState charger_state[] = {
/* 0 */ {.phases = 1, .phase_switch_supported = false, .phase_rotation = PhaseRotation::Unknown, .is_charging = true,                  .supported_current = 10000, .allowed_current = 10000, .charger_state = 3 },
/* 1 */ {.phases = 1, .phase_switch_supported = false, .phase_rotation = PhaseRotation::L123,    },
/* 2 */ {.phases = 1, .phase_switch_supported = true,  .phase_rotation = PhaseRotation::Unknown, .wants_to_charge = true,              .supported_current = 12000,                           .charger_state = 1 },
/* 3 */ {.phases = 1, .phase_switch_supported = true,  .phase_rotation = PhaseRotation::L231,    .is_charging = true,                  .supported_current = 13000, .allowed_current = 13000, .charger_state = 3 },
/* 4 */ {.phases = 3, .phase_switch_supported = false, .phase_rotation = PhaseRotation::Unknown, .wants_to_charge = true,              .supported_current = 14000,                           .charger_state = 1 },
/* 5 */ {.phases = 3, .phase_switch_supported = false, .phase_rotation = PhaseRotation::L123,    .wants_to_charge_low_priority = true, .supported_current = 15000,                           .charger_state = 1 },
/* 6 */ {.phases = 3, .phase_switch_supported = true,  .phase_rotation = PhaseRotation::Unknown, .is_charging = true,                  .supported_current = 16000, .allowed_current = 16000, .charger_state = 3 },
/* 7 */ {.phases = 3, .phase_switch_supported = true,  .phase_rotation = PhaseRotation::L312,    }
};
#pragma clang diagnostic pop

void test_filter_chargers() {
    size_t charger_count = 8;
    uint32_t current_allocation[MAX_CONTROLLED_CHARGERS] = {};
    uint8_t phase_allocation[MAX_CONTROLLED_CHARGERS] = {};
    int idx_array[MAX_CONTROLLED_CHARGERS] = {};
    for(int i = 0; i < charger_count; ++i)
        idx_array[i] = i;

    int matched = 0;
#define filter(x) do { \
    matched = filter_chargers([](uint32_t allocated_current, uint8_t allocated_phases, const ChargerState *state) { \
            return (x); \
        }, \
        idx_array, \
        current_allocation, \
        phase_allocation, \
        charger_state, \
        charger_count); \
    } while(0)

    // Return 0 matches if filter rejects all. Don't swap any.
    filter(false);

    _assert(matched, ==, 0);
    for(int i = 0; i < charger_count; ++i)
        _assert(idx_array[i], ==, i);

    // Return all matches if filter accepts all. Don't swap any.
    filter(true);

    _assert(matched, ==, charger_count);
    for(int i = 0; i < charger_count; ++i)
        _assert(idx_array[i], ==, i);


    // Swap and return one if filter matches one
    current_allocation[1] = 16000;

    filter(allocated_current >= MINIMUM_CURRENT);

    _assert(matched, ==, 1);
    _assert(idx_array[0], ==, 1);
    _assert(idx_array[1], ==, 0);
    for(int i = 2; i < charger_count; ++i)
        _assert(idx_array[i], ==, i);
    current_allocation[1] = 0;

    // Handle 0 chargers
    matched = filter_chargers([](uint32_t allocated_current, uint8_t allocated_phases, const ChargerState *state) {
            return allocated_current >= MINIMUM_CURRENT;
        },
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        0);
    _assert(matched, ==, 0);

    // Test filter stage 1
    filter(state->is_charging);

    _assert(matched, ==, 3);
    _assert(idx_array[0], ==, 0);
    _assert(idx_array[1], ==, 3);
    _assert(idx_array[2], ==, 6);

    current_allocation[0] = MINIMUM_CURRENT;
    current_allocation[3] = MINIMUM_CURRENT;
    current_allocation[6] = MINIMUM_CURRENT;
    phase_allocation[0] = 1;
    phase_allocation[3] = 1;
    phase_allocation[6] = 1;

    // Test filter stage 2
    filter(allocated_current >= MINIMUM_CURRENT && state->phase_switch_supported && state->phases == 3);

    _assert(matched, ==, 1);
    _assert(idx_array[0], ==, 6);

    phase_allocation[6] = 3;

    // Test filter stage 3
    filter(allocated_current >= MINIMUM_CURRENT && (state->phases == 3 || state->phase_rotation == PhaseRotation::Unknown));

    _assert(matched, ==, 2);
    _assert(idx_array[0], ==, 6);
    _assert(idx_array[1], ==, 0);

    // Test filter stage 4
    filter(allocated_current >= MINIMUM_CURRENT && (state->phases == 1 && state->phase_rotation != PhaseRotation::Unknown));

    _assert(matched, ==, 1);
    _assert(idx_array[0], ==, 3);

    current_allocation[0] = ENABLE_CURRENT;
    current_allocation[3] = ENABLE_CURRENT;
    current_allocation[6] = ENABLE_CURRENT;

    // Test filter stage 5
    filter(allocated_current == 0 && state->wants_to_charge);

    _assert(matched, ==, 2);
    _assert(idx_array[0], ==, 4);
    _assert(idx_array[1], ==, 2);

    current_allocation[2] = ENABLE_CURRENT;
    current_allocation[4] = ENABLE_CURRENT;
    phase_allocation[2] = 1;
    phase_allocation[4] = 3;

    // Test filter stage 6
    filter(allocated_current >= MINIMUM_CURRENT && state->wants_to_charge && state->phase_switch_supported && state->phases == 1);

    _assert(matched, ==, 1);
    _assert(idx_array[0], ==, 2);

    phase_allocation[2] = 3;

    // Test filter stage 7
    filter(allocated_current >= MINIMUM_CURRENT);

    _assert(matched, ==, 5);
    _assert(idx_array[0], ==, 2);
    _assert(idx_array[1], ==, 4);
    _assert(idx_array[2], ==, 6);
    _assert(idx_array[3], ==, 3);
    _assert(idx_array[4], ==, 0);

    current_allocation[2] = ENABLE_CURRENT + 1000;
    current_allocation[4] = ENABLE_CURRENT + 1000;
    current_allocation[6] = ENABLE_CURRENT + 1000;
    current_allocation[3] = ENABLE_CURRENT + 1000;
    current_allocation[0] = ENABLE_CURRENT + 1000;

    // Test filter stage 8
    filter(allocated_current >= MINIMUM_CURRENT);

    _assert(matched, ==, 5);
    _assert(idx_array[0], ==, 2);
    _assert(idx_array[1], ==, 4);
    _assert(idx_array[2], ==, 6);
    _assert(idx_array[3], ==, 3);
    _assert(idx_array[4], ==, 0);

    // Test filter stage 9
    filter(allocated_current >= MINIMUM_CURRENT && state->is_charging && state->phase_switch_supported && state->phases == 1);

    _assert(matched, ==, 1);
    _assert(idx_array[0], ==, 3);

    phase_allocation[3] = 3;

    // Test filter stage 10
    filter(state->wants_to_charge_low_priority);

    _assert(matched, ==, 1);
    _assert(idx_array[0], ==, 5);

    current_allocation[5] = 6000;
    phase_allocation[5] = 3;
}

void test_sort_chargers() {
    size_t charger_count = 8;
    uint32_t current_allocation[MAX_CONTROLLED_CHARGERS] = {};
    uint8_t phase_allocation[MAX_CONTROLLED_CHARGERS] = {};
    int idx_array[MAX_CONTROLLED_CHARGERS] = {};
    for(int i = 0; i < charger_count; ++i)
        idx_array[i] = i;

#define sort(group, filter) do {\
    sort_chargers( \
        [](uint32_t allocated_current, uint8_t allocated_phases, const ChargerState *state) { \
            return (group); \
        }, \
        [](CompareInfo left, CompareInfo right) { \
            return (filter); \
        }, \
        idx_array, \
        current_allocation, \
        phase_allocation, \
        charger_state, \
        charger_count); \
    } while (0)

    // Test sort stablility
    for(int i = 0; i < charger_count; ++i) {
        current_allocation[i] = 1234;
    }

    sort(
        (int)allocated_current,
        left.allocated_current < right.allocated_current
    );

    _assert(idx_array[0], ==, 0);
    _assert(idx_array[1], ==, 1);
    _assert(idx_array[2], ==, 2);
    _assert(idx_array[3], ==, 3);
    _assert(idx_array[4], ==, 4);
    _assert(idx_array[5], ==, 5);
    _assert(idx_array[6], ==, 6);
    _assert(idx_array[7], ==, 7);

    memset(current_allocation, 0, sizeof(current_allocation));

    // Test sort stage 1
    sort(
          (state->phases == 1 && state->phase_rotation == PhaseRotation::Unknown) ? 0
        : (state->phases == 3 && !state->phase_switch_supported) ? 1
        : (state->phases == 3 && state->phase_switch_supported && state->phase_rotation == PhaseRotation::Unknown) ? 2
        : (state->phases == 3 && state->phase_switch_supported && state->phase_rotation != PhaseRotation::Unknown) ? 3
        : (state->phases == 1 && state->phase_rotation != PhaseRotation::Unknown) ? 4
        : 999,
        true // TODO
    );
}

void run_tests() {
    test_filter_chargers();
    test_sort_chargers();
}
