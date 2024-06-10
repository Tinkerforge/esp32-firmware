/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "current_allocator.h"

#include <assert.h>
#include <string.h> // For strlen
#include <math.h> // For isnan
#include <algorithm>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h" // Only for snprintf_u and deadline_elapsed
#include "modules/cm_networking/cm_networking_defs.h"
#include "current_allocator_private.h"

//#include "gcc_warnings.h"

#define LOCAL_LOG(fmt, ...) if(local_log) local_log += snprintf_u(local_log, cfg->distribution_log_len - (local_log - cfg->distribution_log.get()), "    " fmt "%c", __VA_ARGS__, '\0');

#define TIMEOUT_MS 32000

int filter_chargers(filter_fn filter_, int *idx_array, const int32_t *current_allocation, const uint8_t *phase_allocation, const ChargerState *charger_state, size_t charger_count) {
    int matches = 0;
    for(int i = 0; i < charger_count; ++i) {
        if (!filter_(current_allocation[idx_array[i]], phase_allocation[idx_array[i]], &charger_state[idx_array[i]]))
            continue;

        int tmp = idx_array[matches];
        idx_array[matches] = idx_array[i];
        idx_array[i] = tmp;
        ++matches;
    }
    return matches;
}

void sort_chargers(group_fn group, compare_fn compare, int *idx_array, const int32_t *current_allocation, const uint8_t *phase_allocation, const ChargerState *charger_state, size_t charger_count, CurrentLimits *limits) {
    int groups[MAX_CONTROLLED_CHARGERS] = {};

    for(int i = 0; i < charger_count; ++i)
        groups[idx_array[i]] = group(current_allocation[idx_array[i]], phase_allocation[idx_array[i]], &charger_state[idx_array[i]]);

    std::stable_sort(
        idx_array,
        idx_array + charger_count,
        [&groups, &compare, &current_allocation, &phase_allocation, &charger_state, &limits] (int left, int right) {
            if (groups[left] != groups[right])
                return groups[left] < groups[right];

            return compare({current_allocation[left], phase_allocation[left], &charger_state[left]},
                           {current_allocation[right], phase_allocation[right], &charger_state[right]},
                           limits);
        }
    );
}


GridPhase get_phase(PhaseRotation rot, ChargerPhase phase) {
    return (GridPhase)(((int)rot >> (6 - 2 * (int)phase)) & 0x3);
}

Cost get_cost(int32_t current_to_allocate,
              ChargerPhase phases_to_allocate,
              PhaseRotation rot,
              int32_t allocated_current,
              ChargerPhase allocated_phases)
{
    Cost cost{};

    // Reclaim allocated current before reallocating
    if (allocated_current != 0) {
        cost = get_cost(allocated_current, allocated_phases, rot, 0, (ChargerPhase) 0);
        for(int i = 0; i < 4; ++i)
            cost[i] = -cost[i];
    }

    cost.pv += (int)phases_to_allocate * current_to_allocate;

    if (rot == PhaseRotation::Unknown) {
        // Phase rotation unknown. We have to assume that each phase could be used
        cost.l1 += current_to_allocate;
        cost.l2 += current_to_allocate;
        cost.l3 += current_to_allocate;
    } else {
        for (int i = 1; i <= (int)phases_to_allocate; ++i) {
            cost[get_phase(rot, (ChargerPhase)i)] += current_to_allocate;
        }
    }

    return cost;
}

bool cost_exceeds_limits(Cost cost, const CurrentLimits* limits, int stage, bool stage_1_2_hysteresis_elapsed)
{
    {
        // Immediately return if supply cable would be overloaded.
        bool supply_exceeded = false;
        for (size_t i = (size_t)GridPhase::L1; i <= (size_t)GridPhase::L3; ++i) {
            supply_exceeded |= limits->supply[i] < cost[i];
        }

        if (supply_exceeded)
            return true;
    }

    bool phases_exceeded = false;
    bool phases_filtered_exceeded = false;
    for (size_t i = (size_t)GridPhase::L1; i <= (size_t)GridPhase::L3; ++i) {
        phases_exceeded |= limits->raw[i] < cost[i];
        phases_filtered_exceeded |= limits->filtered[i] < cost[i];
    }

    bool pv_excess_exceeded = limits->raw.pv < cost.pv;
    bool pv_excess_filtered_exceeded = limits->filtered.pv < cost.pv;

    switch(stage) {
        case 1:
        case 2:
            return phases_exceeded || (stage_1_2_hysteresis_elapsed && pv_excess_exceeded && pv_excess_filtered_exceeded);
        case 3:
        case 4:
        case 7:
        case 8:
            return phases_exceeded || pv_excess_exceeded;
        case 5:
        case 6:
        case 9:
            return phases_exceeded || phases_filtered_exceeded || pv_excess_exceeded || pv_excess_filtered_exceeded;
        default:
            assert(false);
    }
}

void apply_cost(Cost cost, CurrentLimits* limits) {
    for (size_t i = (size_t)GridPhase::PV; i <= (size_t)GridPhase::L3; ++i) {
        limits->raw[i] -= cost[i];
        limits->filtered[i] -= cost[i];
        limits->supply[i] -= cost[i];
    }
}

// Stage 1: Allocate minimum current on the minimal number of phases to all already active (i.e. charging) chargers.
//          Every charger gets only one phase allocated except if it is a three phase charger without phase switch support.
void stage_1(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(state->is_charging);

    sort(
          (state->phases == 1 && state->phase_rotation == PhaseRotation::Unknown) ? 0
        : (state->phases == 3 && !state->phase_switch_supported) ? 1
        : (state->phases == 3 && state->phase_switch_supported && state->phase_rotation == PhaseRotation::Unknown) ? 2
        : (state->phases == 3 && state->phase_switch_supported && state->phase_rotation != PhaseRotation::Unknown) ? 3
        : (state->phases == 1 && state->phase_rotation != PhaseRotation::Unknown) ? 4
        : 999,
        true // TODO lowest allocated charge (not charge current!) first
    );

    auto min_1p = cfg->minimum_current_1p;
    auto min_3p = cfg->minimum_current_3p;

    assert(min_1p <= min_3p);

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        // Give one phase minimum current to all chargers that are 1p or can be switched to 1p.
        // Only give three phase current to fixed 3p chargers.
        bool activate_3p = state->phases != 1 && !state->phase_switch_supported;

        auto current = activate_3p ? min_1p : min_3p;
        if (state->supported_current < current) {
            logger.printfln("Charger %d does not support %s phase minimum current. Skipping", activate_3p ? "three" : "one", idx_array[i]);
            // TODO: reset global hysteresis here?
            // This charger was active (because inactive chargers are filtered out)
            // so we've just now decided to shut it down.
            continue;
        }

        auto cost = get_cost(current, activate_3p ? ChargerPhase::P3 : ChargerPhase::P1, state->phase_rotation, 0, (ChargerPhase) 0);

        if (cost_exceeds_limits(cost, limits, 1, ca_state->global_hysteresis_elapsed)) {
            ca_state->reset_global_hysteresis = true;
            continue;
        }

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;
        phase_allocation[idx_array[i]] = activate_3p ? 3 : 1;

        if (state->phase_rotation != PhaseRotation::Unknown && !activate_3p) {
            ++ca_state->allocated_minimum_current_packets[(size_t)get_phase(state->phase_rotation, ChargerPhase::P1)];
        } else {
            ++ca_state->allocated_minimum_current_packets[(size_t)GridPhase::L1];
            ++ca_state->allocated_minimum_current_packets[(size_t)GridPhase::L2];
            ++ca_state->allocated_minimum_current_packets[(size_t)GridPhase::L3];
        }
    }
}

// Stage 2: Allocate minimum current on three phases to already active chargers
//          that are currently charging on three phases and are phase switchable.
//          Three phase chargers that can't switch phases were allocated three phases in stage 1.
void stage_2(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_current > 0 && state->phase_switch_supported && state->phases == 3);

    sort(0,
        true // TODO lowest allocated charge (not charge current!) first
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto current = cfg->minimum_current_3p;
        if (state->supported_current < current) {
            logger.printfln("Charger %d does not support three phase minimum current. Skipping", idx_array[i]);
            // TODO: reset global hysteresis here?
            // This charger was active (because inactive chargers are filtered out)
            // so we've just now decided to shut it down.
            continue;
        }

        auto cost = get_cost(current, ChargerPhase::P3, state->phase_rotation, current_allocation[idx_array[i]], phase_allocation[idx_array[i]] == 3 ? ChargerPhase::P3 : ChargerPhase::P1);

        if (cost_exceeds_limits(cost, limits, 2, ca_state->global_hysteresis_elapsed)) {
            ca_state->reset_global_hysteresis = true;
            continue;
        }

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;
        phase_allocation[idx_array[i]] = 3;

        // Chargers with unknown phase rotation already had the min current packet counted on every phase in stage 1.
        // Ignore those here.
        if (state->phase_rotation != PhaseRotation::Unknown) {
            ++ca_state->allocated_minimum_current_packets[(size_t)get_phase(state->phase_rotation, ChargerPhase::P2)];
            ++ca_state->allocated_minimum_current_packets[(size_t)get_phase(state->phase_rotation, ChargerPhase::P3)];
        }
    }
}


// Stage 3: Allocate fair current <= enable_current to active 3p and (1p with unknown rotation) chargers.
void stage_3(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_current > 0 && (state->phases == 3 || state->phase_rotation == PhaseRotation::Unknown));

    sort(0,
        left.state->supported_current < right.state->supported_current
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto allocated_current = current_allocation[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];

        auto current = cfg->enable_current - allocated_current;
        for (size_t i = (size_t)GridPhase::L1; i <= (size_t)GridPhase::L3; ++i)
            current = std::min(current, limits->raw[i] / ca_state->allocated_minimum_current_packets[i]);

        current += allocated_current;
        current = std::min(current, (int32_t)state->supported_current);

        auto charger_phases = allocated_phases == 3 ? ChargerPhase::P3 : ChargerPhase::P1;

        auto cost = get_cost(current, charger_phases, state->phase_rotation, allocated_current, charger_phases);

        if (cost_exceeds_limits(cost, limits, 3))
            continue;

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;

        // For both a 3p and a (1p with unknown rotation) charger,
        // we've counted a min current packet on each phase in stage 1
        --ca_state->allocated_minimum_current_packets[GridPhase::L1];
        --ca_state->allocated_minimum_current_packets[GridPhase::L2];
        --ca_state->allocated_minimum_current_packets[GridPhase::L3];
    }
}

// Stage 4: Allocate fair current <= enable_current to 1p with known rotation chargers.
void stage_4(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_current > 0 && (state->phases == 1 && state->phase_rotation != PhaseRotation::Unknown));

    sort(0,
        left.state->supported_current < right.state->supported_current
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto allocated_current = current_allocation[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];

        auto current = cfg->enable_current - allocated_current;
        auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
        current = std::min(current, limits->raw[phase] / ca_state->allocated_minimum_current_packets[(size_t)phase]);

        current += allocated_current;
        current = std::min(current, (int32_t)state->supported_current);

        auto cost = get_cost(current,  ChargerPhase::P1, state->phase_rotation, allocated_current,  ChargerPhase::P1);

        if (cost_exceeds_limits(cost, limits, 4))
            continue;

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;

        --ca_state->allocated_minimum_current_packets[(size_t)phase];
    }
}

// Stage 5: Enable chargers that want to charge
void stage_5(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    if (!ca_state->global_hysteresis_elapsed)
        return;

    int matched = 0;

    filter(allocated_current == 0 && state->wants_to_charge);

    sort(
          (state->phases == 3 && !state->phase_switch_supported) ? 0
        : (state->phases == 1 && !state->phase_switch_supported && state->phase_rotation == PhaseRotation::Unknown) ? 1
        : (state->phase_switch_supported && state->phase_rotation == PhaseRotation::Unknown) ? 2
        : (state->phases == 1 && !state->phase_switch_supported && state->phase_rotation != PhaseRotation::Unknown) ? 3
        : (state->phase_switch_supported && state->phase_rotation != PhaseRotation::Unknown) ? 4
        : 999,
        true // TODO lowest allocated charge (not charge current!) first
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto current = cfg->enable_current;

        // Give one phase minimum current to all chargers that are 1p or can be switched to 1p.
        // Only give three phase current to fixed 3p chargers.
        bool activate_3p = state->phases != 1 && !state->phase_switch_supported;

        if (state->supported_current < current) {
            logger.printfln("Charger %d does not support %s phase minimum current. Skipping", activate_3p ? "three" : "one", idx_array[i]);
            continue;
        }

        auto cost = get_cost(current, activate_3p ? ChargerPhase::P3 : ChargerPhase::P1, state->phase_rotation, 0, (ChargerPhase) 0);

        if (cost_exceeds_limits(cost, limits, 5))
            continue;

        ca_state->reset_global_hysteresis = true;

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;
        phase_allocation[idx_array[i]] = activate_3p ? 3 : 1;
    }
}

// Stage 6: Immediately switch to 3p on newly activated chargers
void stage_6(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    return;
}


int32_t current_capacity(const CurrentLimits *limits, const ChargerState *state, int32_t allocated_current, uint8_t allocated_phases) {
    if (allocated_phases == 3 || state->phase_rotation == PhaseRotation::Unknown) {
        return 3 * std::min({state->supported_current - allocated_current, limits->raw.l1, limits->raw.l2, limits->raw.l3});
    }

    auto capacity = state->supported_current - allocated_current;
    for (size_t i = (size_t)ChargerPhase::P1; i < (size_t)ChargerPhase::P1 + allocated_phases; ++i) {
        auto phase = get_phase(state->phase_rotation, (ChargerPhase)((size_t)ChargerPhase::P1 + i));
        capacity = std::min(capacity, limits->raw[phase]);
    }

    return allocated_phases * capacity;
}

void stage_7(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_current > 0);

    sort(
        3 - allocated_phases,
        true // TODO lowest allocated charge (not charge current!) first
    );

    auto fair_current = limits->raw.pv / matched;

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto allocated_current = current_allocation[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];

        auto current = std::min(fair_current / allocated_phases, current_capacity(limits, state, allocated_current, allocated_phases));
        current += allocated_current;

        auto cost = get_cost(current, allocated_phases == 3 ? ChargerPhase::P3 : ChargerPhase::P1, state->phase_rotation, allocated_current, allocated_phases == 3 ? ChargerPhase::P3 : ChargerPhase::P1);

        if (cost_exceeds_limits(cost, limits, 8))
            continue;

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;
    }
}

void stage_8(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_current > 0);

    sort(
        3 - allocated_phases,
        current_capacity(limits, left.state, left.allocated_current, left.allocated_phases) < current_capacity(limits, right.state, right.allocated_current, right.allocated_phases)
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto allocated_current = current_allocation[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];

        auto current = std::min(limits->raw.pv, current_capacity(limits, state, allocated_current, allocated_phases));
        current += allocated_current;

        auto cost = get_cost(current, allocated_phases == 3 ? ChargerPhase::P3 : ChargerPhase::P1, state->phase_rotation, allocated_current, allocated_phases == 3 ? ChargerPhase::P3 : ChargerPhase::P1);

        // TODO: This should never be true
        if (cost_exceeds_limits(cost, limits, 8))
            continue;

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;
    }
}

/*

Accumulate (allocation cycle time * allocated current * allocated phases) per charger -> allocated charge

Charger types
3P  = currently connected phases 0 = 1p 1 = 3p
SPS = Supports phase switch
ROT = Phase rotation known

|   | 3P|SPS|ROT|
|---|---|---|---|
| 0 | 0 | 0 | 0 |
| 1 | 0 | 0 | 1 |
| 2 | 0 | 1 | 0 |
| 3 | 0 | 1 | 1 |
| 4 | 1 | 0 | 0 |
| 5 | 1 | 0 | 1 |
| 6 | 1 | 1 | 0 |
| 7 | 1 | 1 | 1 |

Filter

1. charger_state.is_charging
2. current_array[i] > 0  && 6,7 (charger_state.supports_phase_switch && charger_state.phases == 3)
3. current_array[i] > 0  && 0,2,4,5,6,7 (charger_state.phases == 3 || charger_state.phase_rotation == UNKNOWN)
4. current_array[i] > 0  && 1,3 (charger_state.phases == 1 && charger_state.phase_rotation != UNKNOWN)
5. current_array[i] == 0 && charger_state.wants_to_charge // TODO: current_array[i] == 0 should not be necessary. We've only allocated current to already charging chargers before this stage.
6. current_array[i] > 0  && charger_state.wants_to_charge && 2,3 (charger_state.supports_phase_switch && charger_state.phases == 1)
7. current_array[i] > 0
8. current_array[i] > 0
9. current_array[i] > 0  && charger_state.is_charging && 2,3 (charger_state.supports_phase_switch && charger_state.phases == 1)
10. charger_state.wants_to_charge_low_priority

Sort

1.
    Groups
    - 0,2: charger_state.phases == 1 && charger_state.phase_rotation == UNKNOWN
    - 4,5: charger_state.phases == 3 && !charger_state.supports_phase_switch
    - 6: charger_state.phases == 3 && charger_state.supports_phase_switch && charger_state.phase_rotation == UNKNOWN
    - 7: charger_state.phases == 3 && charger_state.supports_phase_switch && charger_state.phase_rotation != UNKNOWN
    - 1,3: charger_state.phases == 1 && charger_state.phase_rotation != UNKNOWN
    in groups: lowest allocated charge first

2. lowest allocated charge first
3. unsorted because every charger gets a "fair" part of the left-over current?
4. unsorted because every charger gets a "fair" part of the left-over current?
5.
    Groups
    - 4,5: charger_state.phases == 3 && !charger_state.supports_phase_switch
    - 0:   charger_state.phases == 1 && !charger_state.supports_phase_switch && charger_state.phase_rotation == UNKNOWN
    - 2,6: charger_state.supports_phase_switch && charger_state.phase_rotation == UNKNOWN
    - 1:   charger_state.phases == 1 && !charger_state.supports_phase_switch && charger_state.phase_rotation != UNKNOWN
    - 3,7  charger_state.supports_phase_switch && charger_state.phase_rotation != UNKNOWN
    in groups: lowest allocated charge first

6. lowest allocated charge first
7.
    Groups
    - 4,5,6,7
    - 0,1,2,3
    in groups: lowest allocated charge first
8.
    Groups
    - 4,5,6,7
    - 0,1,2,3
    in groups: (supported current - allocated current) - (lowest phase limit) (same as old charge management but with phase information)
9. lowest allocated charge first
10. TODO
*/

int allocate_current(
    const CurrentAllocatorConfig *cfg,
    const bool seen_all_chargers,
    const uint32_t pv_excess_current,
    const int phases_available,
    const bool cp_disconnect_requested,
    const ChargerState *charger_state,
    const char * const *hosts,
    const std::function<const char *(uint8_t)> get_charger_name,
    const std::function<void(uint8_t)> clear_dns_cache_entry,

    CurrentAllocatorState *ca_state,
    ChargerAllocationState *charger_allocation_state,
    uint32_t *allocated_current
    )
{
    // TODO use enum for this. See charge_manager.cpp state definition for constants.
    int result = 1;

    uint32_t available = seen_all_chargers ? pv_excess_current : 0;

    // TODO: will be replaced if we control the phases of each charger separately
    bool use_3phase_minimum_current = phases_available >= 3;
    uint32_t minimum_current = use_3phase_minimum_current ? cfg->minimum_current_3p :
                                                            cfg->minimum_current_1p;

    bool print_local_log = false;
    char *local_log = cfg->distribution_log.get();
    if (local_log)
        local_log += snprintf_u(local_log, cfg->distribution_log_len - (local_log - cfg->distribution_log.get()), "Redistributing current%c", '\0');

    bool vehicle_connected = false;

    assert(cfg->charger_count > 0 && cfg->charger_count <= MAX_CONTROLLED_CHARGERS);
    uint32_t current_array[MAX_CONTROLLED_CHARGERS] = {0};
    int idx_array[MAX_CONTROLLED_CHARGERS] = {0};
    for(int i = 0; i < cfg->charger_count; ++i)
        idx_array[i] = i;


    // Update control pilot disconnect
    {
        for (size_t i = 0; i < cfg->charger_count; ++i) {
            auto &charger_alloc = charger_allocation_state[i];

            charger_alloc.cp_disconnect = cp_disconnect_requested;
        }
    }

    // Handle unreachable EVSEs
    {
        // If any EVSE is unreachable or in another error state, we set the available current to 0.
        // The distribution algorithm can then run normally and will block all chargers.
        bool unreachable_evse_found = false;
        for (int i = 0; i < cfg->charger_count; ++i) {
            auto &charger = charger_state[i];
            auto &charger_alloc = charger_allocation_state[i];

            auto charger_error = charger_alloc.error;
            if (charger_error != CM_NETWORKING_ERROR_NO_ERROR &&
                charger_error != CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE &&
                charger_error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE &&
                charger_error < CHARGE_MANAGER_CLIENT_ERROR_START) {
                unreachable_evse_found = true;
                LOCAL_LOG("stage 0: %s (%s) reports error %u.", get_charger_name(i), hosts[i], charger_error);

                print_local_log = !ca_state->last_print_local_log_was_error;
                ca_state->last_print_local_log_was_error = true;
            }

            // Charger does not respond anymore
            if (deadline_elapsed(charger.last_update + TIMEOUT_MS)) {
                unreachable_evse_found = true;
                LOCAL_LOG("stage 0: Can't reach EVSE of %s (%s): last_update too old.", get_charger_name(i), hosts[i]);

                bool state_was_not_five = charger_alloc.state != 5;
                charger_alloc.state = 5;
                if (state_was_not_five || charger_error < CHARGE_MANAGER_CLIENT_ERROR_START) {
                    charger_alloc.error = CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE;
                    clear_dns_cache_entry(i);

                    print_local_log = !ca_state->last_print_local_log_was_error;
                    ca_state->last_print_local_log_was_error = true;
                }
            } else if (charger_alloc.error == CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE) {
                charger_alloc.error = CM_NETWORKING_ERROR_NO_ERROR;
            }

            // Charger did not update the charging current in time
            if (charger_alloc.allocated_current < charger.allowed_current && deadline_elapsed(charger_alloc.last_sent_config + TIMEOUT_MS)) {
                unreachable_evse_found = true;
                LOCAL_LOG("stage 0: EVSE of %s (%s) did not react in time.", get_charger_name(i), hosts[i]);

                bool state_was_not_five = charger_alloc.state != 5;
                charger_alloc.state = 5;
                if (state_was_not_five || charger_error < CHARGE_MANAGER_CLIENT_ERROR_START) {
                    charger_alloc.error = CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE;
                    print_local_log = !ca_state->last_print_local_log_was_error;
                    ca_state->last_print_local_log_was_error = true;
                }
            } else if (charger_alloc.error == CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE) {
                charger_alloc.error = CM_NETWORKING_ERROR_NO_ERROR;
            }

            // Block firmware update if charger has a vehicle connected.
            if (charger_alloc.state != 0)
                vehicle_connected = true;
        }

        if (unreachable_evse_found) {
            // Shut down everything.
            available = 0;
            LOCAL_LOG("%s", "stage 0: Unreachable, unreactive or misconfigured EVSE(s) found. Setting available current to 0 mA.");
            result = 2;

            // Any unreachable EVSE will block a firmware update.
            vehicle_connected = true;
        } else {
            if (ca_state->last_print_local_log_was_error) {
                ca_state->last_print_local_log_was_error = false;
                print_local_log = true;
            }
        }
    }

    // Sort chargers.
    {
        // Sort the chargers by their minimum supported current,
        // then sort chargers that are already charging before those that
        // want to charge but are not charging yet.
        // Sorting by the minimum current allows us to distribute the current "perfectly"
        // with a single pass over the chargers.
        int chargers_requesting_current = 0;
        int chargers_requesting_current_low_priority = 0;
        for (int i = 0; i < cfg->charger_count; ++i) {
            auto &charger = charger_state[i];
            if (charger.is_charging || charger.wants_to_charge) {
                ++chargers_requesting_current;
            } else if (charger.wants_to_charge_low_priority) {
                ++chargers_requesting_current_low_priority;
            }
        }

        LOCAL_LOG("%d charger%s request%s current. %u mA available.",
                  chargers_requesting_current,
                  chargers_requesting_current == 1 ? "" : "s",
                  chargers_requesting_current == 1 ? "s" : "",
                  available);

        std::stable_sort(idx_array, idx_array + cfg->charger_count, [charger_state](int left, int right) {
            return charger_state[left].requested_current < charger_state[right].requested_current;
        });

        std::stable_sort(idx_array, idx_array + cfg->charger_count, [charger_state](int left, int right) {
            bool left_charging = charger_state[left].is_charging;
            bool right_charging = charger_state[right].is_charging;
            return left_charging && !right_charging;
        });
    }

    // Allocate current to chargers.
    {
        // First allocate the minimum supported current to each charger.
        // Then distribute the rest of the available current to those
        // that received the minimum.
        int chargers_allocated_current_to = 0;

        uint16_t current_to_set = minimum_current;
        for (int i = 0; i < cfg->charger_count; ++i) {
            auto &charger = charger_state[idx_array[i]];

            if (!charger.is_charging && !charger.wants_to_charge) {
                continue;
            }

            uint16_t supported_current = charger.supported_current;
            if (supported_current < current_to_set) {
                LOCAL_LOG("stage 0: Can't unblock %s (%s): It only supports %u mA, but %u mA is the configured minimum current. Handling as low priority charger.",
                          get_charger_name(idx_array[i]),
                          hosts[idx_array[i]],
                          supported_current,
                          current_to_set);
                continue;
            }

            if (available < current_to_set) {
                LOCAL_LOG("stage 0: %u mA left, but %u mA required to unblock another charger. Blocking all following chargers.",available, current_to_set);
                current_to_set = 0;
            }

            if (current_to_set > 0) {
                ++chargers_allocated_current_to;
            }

            current_array[idx_array[i]] = current_to_set;
            available -= current_to_set;

            LOCAL_LOG("stage 0: Calculated target for %s (%s) of %u mA. %u mA left.",
                      get_charger_name(idx_array[i]),
                      hosts[idx_array[i]],
                      current_to_set,
                      available);
        }

        if (available > 0) {
            LOCAL_LOG("stage 0: %u mA still available. Recalculating targets.", available);

            int chargers_reallocated = 0;
            for (int i = 0; i < cfg->charger_count; ++i) {
                if (current_array[idx_array[i]] == 0)
                    continue;

                auto &charger = charger_state[idx_array[i]];
                uint16_t current_per_charger = std::min(32000u, available / (chargers_allocated_current_to - chargers_reallocated));

                uint16_t requested_current = charger.requested_current;

                // If exactly one charger is charging, double the current margin for faster power manager control.
                // If the requested current is exactly 6 A, the car limits itself to less than 6 A - requested_current_margin,
                // so we don't have to increase it.
                if (chargers_allocated_current_to == 1 && requested_current > 6000) {
                    requested_current += cfg->requested_current_margin;
                }

                // Protect against overflow.
                if (requested_current < current_array[idx_array[i]])
                    continue;

                uint16_t max_current = std::min((uint16_t)32000u, requested_current) - current_array[idx_array[i]];
                uint16_t current_to_add = std::min(max_current, current_per_charger);

                ++chargers_reallocated;

                current_array[idx_array[i]] += current_to_add;
                available -= current_to_add;

                LOCAL_LOG("stage 0: Recalculated target for %s (%s) of %u mA. %u mA left.",
                          get_charger_name(idx_array[i]),
                          hosts[idx_array[i]],
                          current_array[idx_array[i]],
                          available);
            }
        }
    }

    // Wake up chargers that already charged once.
    {
        if (available > 0) {
            LOCAL_LOG("stage 0: %u mA still available. Attempting to wake up chargers that already charged their vehicle once.", available);

            uint16_t current_to_set = minimum_current;
            for (int i = 0; i < cfg->charger_count; ++i) {
                auto &charger = charger_state[idx_array[i]];

                uint16_t supported_current = charger.supported_current;

                bool high_prio = charger.is_charging || charger.wants_to_charge;
                bool low_prio = charger.wants_to_charge_low_priority;

                if (!low_prio && !(high_prio && supported_current < current_to_set)) {
                    continue;
                }

                if (supported_current < current_to_set) {
                    LOCAL_LOG("stage 0: %s (%s) only supports %u mA, but %u mA is the configured minimum current. Allocating %u mA.",
                              get_charger_name(idx_array[i]),
                              hosts[idx_array[i]],
                              supported_current,
                              current_to_set,
                              current_to_set);
                }

                if (available < current_to_set) {
                    LOCAL_LOG("stage 0: %u mA left, but %u mA required to unblock another charger. Blocking all following chargers.",available, current_to_set);
                    current_to_set = 0;
                }

                /*if (current_to_set > 0) {
                    ++chargers_allocated_current_to;
                }*/

                current_array[idx_array[i]] = current_to_set;
                available -= current_to_set;

                LOCAL_LOG("stage 0: Calculated target for %s (%s) of %u mA. %u mA left.",
                          get_charger_name(idx_array[i]),
                          hosts[idx_array[i]],
                          current_to_set,
                          available);
            }
        }
    }

    // Apply current limits.
    {
        // First, throttle chargers that have a higher current limit than the calculated one.
        // If no charger has to be throttled, then also unthrottle other chargers. Skip this
        // stage if even one charger needs to be throttled to be sure that the available current
        // is never exceeded.
        bool skip_stage_2 = false;
        for (int i = 0; i < cfg->charger_count; ++i) {
            auto &charger = charger_state[i];
            auto &charger_alloc = charger_allocation_state[i];

            uint16_t current_to_set = current_array[i];

            bool will_throttle = current_to_set < charger_alloc.allocated_current || current_to_set < charger.allowed_current;

            if (!will_throttle) {
                continue;
            }

            LOCAL_LOG("stage 1: Throttled %s (%s) to %d mA.",
                      get_charger_name(i),
                      hosts[i],
                      current_to_set);

            bool current_changed = charger_alloc.allocated_current != current_to_set;
            charger_alloc.allocated_current = current_to_set;
            if (current_changed) {
                print_local_log = true;
                if (charger_alloc.error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                    charger_alloc.last_sent_config = millis();
            }

            // Skip stage 2 to wait for the charger to adapt to the now smaller limit.
            // Some cars are slow to adapt to a new limit. The standard requires them to
            // react in 5 seconds.
            // More correct would be to detect whether the throttled current limit
            // was accepted by the box more than 5 seconds ago (so that we can be sure the timing fits)
            // However this is complicated and waiting a complete cycle (i.e. 10 seconds)
            // works good enough.
            if (!skip_stage_2) {
                LOCAL_LOG("%s", "stage 1: Throttled a charger. Skipping stage 2");
                skip_stage_2 = true;
            }
        }

        if (!skip_stage_2) {
            for (int i = 0; i < cfg->charger_count; ++i) {
                auto &charger = charger_state[i];
                auto &charger_alloc = charger_allocation_state[i];

                uint16_t current_to_set = current_array[i];

                // > instead of >= to only catch chargers that were not already modified in stage 1.
                bool will_not_throttle = current_to_set > charger_alloc.allocated_current || current_to_set > charger.allowed_current;

                if (!will_not_throttle) {
                    continue;
                }

                LOCAL_LOG("stage 2: Unthrottled %s (%s) to %d mA.",
                          get_charger_name(i),
                          hosts[i],
                          current_to_set);

                bool current_changed = charger_alloc.allocated_current != current_to_set;
                charger_alloc.allocated_current = current_to_set;
                if (current_changed) {
                    print_local_log = true;
                    if (charger_alloc.error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                        charger_alloc.last_sent_config = millis();
                }
            }
        } else {
            LOCAL_LOG("%s", "Skipping stage 2");
        }
    }

    if (print_local_log) {
        local_log = cfg->distribution_log.get();
        if (local_log) {
            size_t len = strlen(local_log);
            while (len > 0) {
                logger.write(local_log, len);
                local_log += len + 1;
                if ((local_log - cfg->distribution_log.get()) >= cfg->distribution_log_len)
                    break;
                len = strlen(local_log);
            }
        }
    }

    *allocated_current = 0;
    for (int i = 0; i < cfg->charger_count; ++i) {
        auto &charger_alloc = charger_allocation_state[i];
        *allocated_current += charger_alloc.allocated_current;
    }

#if MODULE_FIRMWARE_UPDATE_AVAILABLE() && MODULE_ENERGY_MANAGER_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
    firmware_update.vehicle_connected = vehicle_connected;
#else
    (void)vehicle_connected;
#endif

    return result;
}


static uint8_t get_charge_state(uint8_t charger_state, uint16_t supported_current, uint32_t charging_time, uint16_t target_allocated_current)
{
    if (charger_state == 0) // not connected
        return 0;
    if (charger_state == 3) // charging
        return 4;
    if (charger_state == 4) // error
        return 5;
    if (charger_state == 1 && supported_current == 0) // connected but blocked, supported current == 0 means another slot blocks
        return 1;
    if (charger_state == 1 && supported_current != 0) { // blocked by charge management (as supported current != 0)
        if (charging_time == 0)
            return 2; // Not charged this session
        else
            return 6; // Charged at least once
    }
    if (charger_state == 2)
        return 3; // Waiting for the car to start charging

    logger.printfln("Unknown state! cs %u sc %u ct %u tac %u", charger_state, supported_current, charging_time, target_allocated_current);
    return 5;
}


bool update_from_client_packet(
    uint8_t client_id,
    cm_state_v1 *v1,
    cm_state_v2 *v2,
    const CurrentAllocatorConfig *cfg,
    ChargerState *charger_state,
    ChargerAllocationState *charger_allocation_state,
    const char * const *hosts,
    const std::function<const char *(uint8_t)> get_charger_name
    )
{
    // TODO: bounds check
    auto &target = charger_state[client_id];
    auto &target_alloc = charger_allocation_state[client_id];

    // Don't update if the uptimes are the same.
    // This means, that the EVSE hangs or the communication
    // is not working. As last_update will now hang too,
    // the management will stop all charging after some time.
    if (target.uptime == v1->evse_uptime) {
        logger.printfln("Received stale charger state from %s (%s). Reported EVSE uptime (%u) is the same as in the last state. Is the EVSE still reachable?",
            get_charger_name(client_id), hosts[client_id],
            v1->evse_uptime);
        if (deadline_elapsed(target.last_update + 10000)) {
            target_alloc.state = 5;
            target_alloc.error = CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE;
        }

        return false;
    }

    target.uid = v1->esp32_uid;
    target.uptime = v1->evse_uptime;

#if MODULE_FIRMWARE_UPDATE_AVAILABLE() && MODULE_ENERGY_MANAGER_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
    // Immediately block firmware updates if this charger reports a connected vehicle.
    if (v1->charger_state != 0)
        firmware_update.vehicle_connected = true;
#endif

    // A charger wants to charge if:
    // the charging time is 0 (it has not charged this vehicle yet), no other slot blocks
    //     AND we are still in charger state 1 (i.e. blocked by a slot, so the charge management slot)
    //         or 2 (i.e. already have current allocated)
    // OR the charger is already charging
    bool wants_to_charge = (v1->charging_time == 0 && v1->supported_current != 0 && (v1->charger_state == 1 || v1->charger_state == 2)) || v1->charger_state == 3;
    target.wants_to_charge = wants_to_charge;

    // A charger wants to charge and has low priority if it has already charged this vehicle
    // AND only the charge manager slot (charger_state == 1, supported_current != 0) or no slot (charger_state == 2) blocks.
    bool low_prio = v1->charging_time != 0 && v1->supported_current != 0 && (v1->charger_state == 1 || v1->charger_state == 2);
    target.wants_to_charge_low_priority = low_prio;

    target.is_charging = v1->charger_state == 3;
    target.allowed_current = v1->allowed_charging_current;
    target.supported_current = v1->supported_current;
    target.cp_disconnect_supported = CM_FEATURE_FLAGS_CP_DISCONNECT_IS_SET(v1->feature_flags);
    target.cp_disconnect_state = CM_STATE_FLAGS_CP_DISCONNECTED_IS_SET(v1->state_flags);
    target.last_update = millis();
    target.charger_state = v1->charger_state;

    uint16_t requested_current = v1->supported_current;

    if (v2 != nullptr && v1->charger_state == 3 && v2->time_since_state_change >= cfg->requested_current_threshold * 1000) {
        int32_t max_phase_current = -1;

        for (int i = 0; i < 3; i++) {
            if (isnan(v1->line_currents[i])) {
                // Don't trust the line currents if one is missing.
                max_phase_current = 32000;
                break;
            }

            max_phase_current = std::max(max_phase_current, (int32_t)(v1->line_currents[i] * 1000.0f));
        }
        // The CM protocol sends 0 instead of nan.
        if (max_phase_current == 0)
            max_phase_current = 32000;

        max_phase_current += cfg->requested_current_margin;

        max_phase_current = std::max(6000, std::min(32000, max_phase_current));
        requested_current = std::min(requested_current, (uint16_t)max_phase_current);
    }
    target.requested_current = requested_current;

    target.meter_supported = CM_FEATURE_FLAGS_METER_IS_SET(v1->feature_flags);
    if (!isnan(v1->power_total)) {
        target.power_total_sum = target.power_total_sum + v1->power_total;
        target.power_total_count = target.power_total_count + 1;
    }
    if (!isnan(v1->energy_abs)) {
        target.energy_abs = v1->energy_abs;
    }

    if (v1->error_state != 0) {
        target_alloc.error = CHARGE_MANAGER_CLIENT_ERROR_START + static_cast<uint32_t>(v1->error_state);
    }

    if (target_alloc.error < 128 || target_alloc.error == CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE) {
        target_alloc.error = 0;
    }

    if (target_alloc.error == 0 || target_alloc.error >= CHARGE_MANAGER_CLIENT_ERROR_START)
        target_alloc.state = get_charge_state(v1->charger_state,
                                              v1->supported_current,
                                              v1->charging_time,
                                              target_alloc.allocated_current);
    return true;
}
