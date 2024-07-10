/* esp32-firmware
 * Copyright (C) 2020-2024 Erik Fleckstein <erik@tinkerforge.com>
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


#include <assert.h>
#include <string.h> // For strlen
#include <math.h> // For isnan
#include <algorithm>
#include <stdio.h> // For snprintf

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h" // Only for snprintf_u and deadline_elapsed
#include "modules/cm_networking/cm_networking_defs.h"
#include "current_allocator_private.h"

//#include "gcc_warnings.h"

#define LOCAL_LOG(fmt, ...) if(local_log) local_log += snprintf_u(local_log, cfg->distribution_log_len - (local_log - cfg->distribution_log.get()), "    " fmt "%c", __VA_ARGS__, '\0');

#define TIMEOUT_MS 32000

#define PRINT_COST(x) logger.printfln("%s %d %d %d %d", #x, x[0], x[1], x[2], x[3])

#define ALLOCATION_TIMEOUT_S 5

// Only switch phases, start or stop chargers if this is elapsed.
// Don't reset hysteresis when stopping chargers:
// Stopping and immediately starting again is fine, see phase switch.
static constexpr micros_t GLOBAL_HYSTERESIS = 3_usec * 60_usec * 1000_usec * 1000_usec;

// Only consider charger for rotation if it has charged at least this amount of energy.
static constexpr int32_t ALLOCATED_ENERGY_ROTATION_THRESHOLD = 5; /*kWh*/

// Amount of time a charger should stay activated before considering it for rotation or phase switch.
static constexpr micros_t MINIMUM_ACTIVE_TIME = 15_usec * 60_usec * 1000_usec * 1000_usec;

// Allow charging for this time to attempt to wake-up a "full" vehicle,
// i.e. one that triggered a C -> B2 transition and/or waited in B2 for too long
static constexpr micros_t WAKEUP_TIME = 3_usec * 60_usec * 1000_usec * 1000_usec;

static constexpr int32_t UNLIMITED = 10 * 1000 * 1000; /* mA */

static void print_alloc(int stage, CurrentLimits *limits, int32_t *current_array, uint8_t *phases_array, size_t charger_count, const ChargerState *charger_state) {
    char buf[300] = {};
    logger.printfln("%d LIMITS raw(%6.3f,%6.3f,%6.3f,%6.3f) min(%6.3f,%6.3f,%6.3f,%6.3f) max(%6.3f,%6.3f,%6.3f,%6.3f)",
           stage,
           limits->raw[0] / 1000.0f,
           limits->raw[1] / 1000.0f,
           limits->raw[2] / 1000.0f,
           limits->raw[3] / 1000.0f,

           limits->min[0] / 1000.0f,
           limits->min[1] / 1000.0f,
           limits->min[2] / 1000.0f,
           limits->min[3] / 1000.0f,

           limits->max[0] / 1000.0f,
           limits->max[1] / 1000.0f,
           limits->max[2] / 1000.0f,
           limits->max[3] / 1000.0f);

    char *ptr = buf;
    ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "  ALLOC");
    for(size_t i = 0; i < charger_count; ++i) {
        if (phases_array[i] == 0 && current_array[i] == 0 && stage != 0)
            ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "|        %zu        |", i);
        else
            ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), " %6.3f@%dp;%.3fkWh", current_array[i] / 1000.0f, phases_array[i], charger_state[i].allocated_energy_this_rotation);
    }
    logger.printfln(buf);
}

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

// Returns how much current would be used
// if the allocation of a charger was changed from
// [allocated_current]@[allocated_phases]
// to [current_to_allocate]@{phases_to_allocate].
Cost get_cost(int32_t current_to_allocate,
              ChargerPhase phases_to_allocate,
              PhaseRotation rot,
              int32_t allocated_current,
              ChargerPhase allocated_phases)
{
    Cost cost{};

    // Reclaim old allocation before reallocating
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

// Checks stage-specific limits.
bool cost_exceeds_limits(Cost cost, const CurrentLimits* limits, int stage)
{
    bool phases_exceeded = false;
    for (size_t i = (size_t)GridPhase::L1; i <= (size_t)GridPhase::L3; ++i) {
        phases_exceeded |= limits->raw[i] < cost[i];
    }

    bool pv_excess_exceeded = limits->raw.pv < cost.pv;

    switch(stage) {
        case 7:
        case 8:
            return phases_exceeded || pv_excess_exceeded;
        case 9: {
            bool phases_min_exceeded = false;
            for (size_t i = (size_t)GridPhase::L1; i <= (size_t)GridPhase::L3; ++i) {
                phases_min_exceeded |= limits->min[i] < cost[i];
            }

            bool pv_excess_min_exceeded = limits->min.pv < cost.pv;
            return phases_exceeded || pv_excess_exceeded || phases_min_exceeded || pv_excess_min_exceeded;
        }
        default:
            assert(false && "Unknown charge management stage!");
    }
}

void apply_cost(Cost cost, CurrentLimits* limits) {
    for (size_t i = (size_t)GridPhase::PV; i <= (size_t)GridPhase::L3; ++i) {
        limits->raw[i] -= cost[i];
        limits->min[i] -= cost[i];
        limits->max[i] -= cost[i];
    }
}

// A charger is active if
// - we have allocated current to it or will in the future (thus checking the allocated phases)
// - it wants to charge (i.e. a vehicle is plugged in and no other slot blocks) or is charging (i.e. is in state C)
// - we are not currently attempting to wake up a "full" vehicle
static bool is_active(uint8_t allocated_phases, const ChargerState *state) {
    return allocated_phases > 0 && (state->wants_to_charge || state->is_charging) && state->last_wakeup == 0_usec;
}

// Stage 1: Rotate chargers
// If there is any charger that wants to charge but doesn't have current allocated,
// temporarily disable chargers that are currently active (see is_active) but have been allocated
// ALLOCATED_ENERGY_ROTATION_THRESHOLD energy since being activated.
//
// Also deallocate phases if the charger is not active anymore.
// We want to dealloc a charger if the connected vehicle is currently being woken up.
// If there is current left over at the end, we will reallocate some to this charger.
void stage_1(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    // Only rotate if there is at least one charger that does want to charge but doesn't have current allocated.
    bool have_b1 = false;
    for (int i = 0; i < charger_count; ++i) {
        have_b1 |= charger_state[i].wants_to_charge && phase_allocation[i] == 0;
    }

    for (int i = 0; i < charger_count; ++i) {
        const auto *state = &charger_state[i];

        bool dont_rotate = state->allocated_energy_this_rotation < ALLOCATED_ENERGY_ROTATION_THRESHOLD || !deadline_elapsed(state->last_switch + MINIMUM_ACTIVE_TIME);
        bool keep_active = is_active(phase_allocation[i], state) && (!have_b1 || !ca_state->global_hysteresis_elapsed || dont_rotate);

        if (!keep_active) {
            phase_allocation[i] = 0;
            continue;
        }

        // If a charger does not support phase switching (anymore),
        // the connected number of phases wins against the allocated number of phases.
        if (!state->phase_switch_supported) {
            phase_allocation[i] = state->phases;
            continue;
        }
    }
}

// Calculates the control window.
// The window is the range of current that could be allocated between
// throttling all chargers to their minimum current
// and unthrottling all chargers to their maximum
// The maximum of three phase chargers depends on the number of active one phase chargers
// because have to allocate at least the 1p minimum current to those
// before unthrottling the three phase chargers (both without exceeding the raw limit).
// Only check the raw limit because this is a current allocation decision,
// not a enable/disable/phase switch decision.
void calculate_window(const int *idx_array_const, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    auto min_1p = cfg->minimum_current_1p;
    auto min_3p = cfg->minimum_current_3p;

    Cost wnd_min{0, 0, 0, 0};
    Cost wnd_min_1p{0, 0, 0, 0};
    Cost wnd_max{0, 0, 0, 0};

    int matched = 0;

    // Work on copy of idx_array so that this function can be called in stages without destroying their sort order.
    int idx_array[MAX_CONTROLLED_CHARGERS];
    memcpy(idx_array, idx_array_const, sizeof(idx_array));

    filter(allocated_phases > 0);

    // Calculate minimum window
    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases == 3) {
            wnd_min.pv += 3 * min_3p;
            wnd_min.l1 += min_3p;
            wnd_min.l2 += min_3p;
            wnd_min.l3 += min_3p;
        }
        else if (state->phase_rotation == PhaseRotation::Unknown) {
            // unknown rot 1p
            wnd_min.pv += min_1p;
            wnd_min.l1 += min_1p;
            wnd_min.l2 += min_1p;
            wnd_min.l3 += min_1p;

            wnd_min_1p.l1 += min_1p;
            wnd_min_1p.l2 += min_1p;
            wnd_min_1p.l3 += min_1p;
        } else {
            // known rot 1p
            const auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
            wnd_min.pv += min_1p;
            wnd_min[phase] += min_1p;
            wnd_min_1p[phase] += min_1p;
        }
    }

    // Calculate left over current for 3p chargers after allocating the 1p minimum to 1p chargers
    auto current_avail_for_3p = UNLIMITED;

    for (size_t i = 0; i < 4; ++i) {
        auto avail_on_phase = limits->raw[i] - wnd_min_1p[i];
        current_avail_for_3p = std::min(current_avail_for_3p, avail_on_phase);
    }

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases != 3)
            continue;

        wnd_max += Cost{0,
                        state->requested_current,
                        state->requested_current,
                        state->requested_current};

        // It is sufficient to check one phase here, wnd_max should have the same value on every phase because only three phase chargers are included yet
        if (wnd_max.l1 > current_avail_for_3p) {
            wnd_max = Cost{wnd_max.l1 + (current_avail_for_3p - wnd_max.l1) * alloc_phases,
                           current_avail_for_3p,
                           current_avail_for_3p,
                           current_avail_for_3p};
            break;
        }

        wnd_max.pv += state->requested_current * alloc_phases;
    }

    // Calculate maximum window of 1p chargers with known rotation.
    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases == 3 || state->phase_rotation != PhaseRotation::Unknown)
            continue;

        // 1p unknown rotated chargers

        auto current = (int32_t)state->requested_current;
        for (size_t p = 1; p < 4; ++p) {
            auto avail_on_phase = limits->raw[p] - wnd_max[p];
            current = std::min(current, avail_on_phase);
        }

        wnd_max += Cost{current,
                        current,
                        current,
                        current};
    }

    // Add maximum window of 3p chargers and chargers with unknown rotation.
    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases == 3 || state->phase_rotation == PhaseRotation::Unknown)
            continue;

        const auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
        const auto current = std::min(limits->raw[phase] - wnd_max[phase], (int)state->requested_current);

        wnd_max[phase] += current;
        wnd_max.pv += current;
    }

    ca_state->control_window_min = wnd_min;
    ca_state->control_window_max = wnd_max;
}

static bool was_just_plugged_in(const ChargerState *state) {
    return state->last_plug_in != 0_usec && state->wants_to_charge;
}

// Stage 2: Immediately activate chargers were a vehicle was just plugged in.
// Do this before calculating the initial control window and ignore limits.
void stage_2(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(was_just_plugged_in(state));

    // Charger that is plugged in for the longest time first.
    sort(0,
        left.state->last_plug_in < right.state->last_plug_in
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        bool is_fixed_3p = state->phases == 3 && !state->phase_switch_supported;
        bool activate_3p = is_fixed_3p;

        phase_allocation[idx_array[i]] = activate_3p ? 3 : 1;
    }
}

// Stage 3: Shut down chargers if over limit
// - Calculate window
// - Only work with chargers that currently have phases allocated (i.e. those that were active in stage 1)
// - Sort those so that chargers that have received the most energy (overall, not only since rotating) will be shut down first
// - For each phase where the window minimum is greater than the raw limit
//   (this means that we are currently overloading this phase and can't fix this by throttling all chargers to their minimum current),
//   shut down chargers and recalculate the window minimum until it is less than the raw phase limit.
//   Ignore hysteresis, active times, etc. here
// - If the PV limit is less than the raw _AND_ filtered PV window minima and the global hysteresis is elapsed,
//   shut down chargers until at least one of the minima is less than the PV limit.
// - If any charger was shut down recalculate the window. The minima should already be correct, but updating the maxima is too complicated
void stage_3(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    calculate_window(idx_array, current_allocation, phase_allocation, limits, charger_state, cfg->charger_count, cfg, ca_state);

    Cost wnd_min = ca_state->control_window_min;
    auto min_1p = cfg->minimum_current_1p;
    auto min_3p = cfg->minimum_current_3p;

    int matched = 0;

    filter(allocated_phases > 0);

    // Reverse sort. Charger that was active for the longest time first.
    // Group chargers that were activated in stage 2 (because a vehicle was just plugged in) behind others.
    sort(
        was_just_plugged_in(state) ? 1 : 0,
        left.state->last_switch < right.state->last_switch
    );

    bool any_charger_shut_down = false;

    // If any phase is overloaded, shut down chargers using this phase until it is not.
    // Maybe try to be more clever here:
    // - Shut down 1p chargers first if only one or two phase limits are below wnd_min
    // - Shut down 1p unknown rotated chargers last
    for (size_t p = 1; p < 4; ++p) {
        for (int i = 0; i < matched; ++i) {
            if (limits->raw[p] >= wnd_min[p])
                break;

            const auto *state = &charger_state[idx_array[i]];
            const auto alloc_phases = phase_allocation[idx_array[i]];

            if (alloc_phases == 0)
                continue;

            if (state->phases == 3) {
                phase_allocation[idx_array[i]] = 0;
                wnd_min -= Cost {3 * min_3p, min_3p, min_3p, min_3p};
            } else if (state->phase_rotation == PhaseRotation::Unknown) {
                phase_allocation[idx_array[i]] = 0;
                wnd_min -= Cost {min_1p, min_1p, min_1p, min_1p};
            } else if ((GridPhase)p == get_phase(state->phase_rotation, ChargerPhase::P1)) {
                phase_allocation[idx_array[i]] = 0;
                wnd_min.pv -= min_1p;
                wnd_min[p] -= min_1p;
            } else {
                // This is a 1p charger with known rotation that is not active on this phase.
                // Disabling it will not improve the situation.
                continue;
            }

            any_charger_shut_down = true;
        }
    }

    // If the max PV excess over the last x minutes is less than the PV minimum window,
    // we want to shut down chargers and this decision was stable for x minutes.
    // Also check the hysteresis to make sure the last switch on/off decisions
    // did propagate to the calculated limits.
    for (int i = 0; i < matched; ++i) {
        if (limits->max.pv >= wnd_min.pv || !ca_state->global_hysteresis_elapsed)
            break;

        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases == 0 || was_just_plugged_in(state))
            continue;

        if (state->phases == 3) {
            phase_allocation[idx_array[i]] = 0;
            wnd_min -= Cost {3 * min_3p, min_3p, min_3p, min_3p};
        } else if (state->phase_rotation == PhaseRotation::Unknown) {
            phase_allocation[idx_array[i]] = 0;
            wnd_min -= Cost {min_1p, min_1p, min_1p, min_1p};
        } else {
            phase_allocation[idx_array[i]] = 0;
            wnd_min.pv -= min_1p;

            auto p = get_phase(state->phase_rotation, ChargerPhase::P1);
            wnd_min[p] -= min_1p;
        }
    }

    if (any_charger_shut_down) {
        calculate_window(idx_array, current_allocation, phase_allocation, limits, charger_state, charger_count, cfg, ca_state);
    }
}

// A charger can be activated if
// - wnd_max.pv is less than the min PV limit (if CHECK_IMPROVEMENT or CHECK_IMPROVEMENT_ALL_PHASE is set for the PV phase)
// - wnd_max is less than the min limit of
//      - at least one phase that the charger is active on (if CHECK_IMPROVEMENT is set for that phase)
//      - or all phases that the charger is active on (if CHECK_IMPROVEMENT_ALL_PHASE is set for that phase)
//   and activating the charger moves wnd_max closer to the limit (i.e. its cost on this phase is positive)
// - the
//      - enable cost (if CHECK_MIN_WINDOW_ENABLE is set for that phase)
//      - cost  (if CHECK_MIN_WINDOW_MIN is set for that phase)
//   does not exceed the min on any phase or PV that the charger is active on
static constexpr int CHECK_MIN_WINDOW_MIN = 1;
static constexpr int CHECK_MIN_WINDOW_ENABLE = 2;
static constexpr int CHECK_IMPROVEMENT = 4;
static constexpr int CHECK_IMPROVEMENT_ALL_PHASE = 8;
static bool can_activate(const Cost check_phase, const Cost new_cost, const Cost new_enable_cost, const Cost wnd_min, const Cost wnd_max, const CurrentLimits *limits, const CurrentAllocatorConfig *cfg, bool is_unknown_rotated_1p_3p_switch=false) {
    bool improves_pv = (check_phase.pv & (CHECK_IMPROVEMENT | CHECK_IMPROVEMENT_ALL_PHASE)) == 0 || (new_cost.pv > 0 && wnd_max.pv < limits->min.pv);
    if (!improves_pv)
        return false;

    bool check_all_phases = ((check_phase.l1 | check_phase.l2 | check_phase.l3) & CHECK_IMPROVEMENT_ALL_PHASE) != 0;
    bool improves_any_phase = false;
    bool improves_all_phases = true;

    for (size_t p = 1; p < 4; ++p) {
        if ((check_phase[p] & (CHECK_IMPROVEMENT | CHECK_IMPROVEMENT_ALL_PHASE)) == 0 || new_cost[p] <= 0)
            continue;

        // More efficient than |= or &= on the Xtensa.
        if (wnd_max[p] < limits->min[p]) {
            improves_any_phase = true;
        } else {
            improves_all_phases = false;
        }
    }
    if ((check_all_phases && !improves_all_phases) || (!check_all_phases && !improves_any_phase)) {
        // PV is already checked above.
        bool enable = is_unknown_rotated_1p_3p_switch;
        if (!enable)
            return false;
    }

    for (size_t p = 0; p < 4; ++p) {
        if ((check_phase[p] & (CHECK_MIN_WINDOW_MIN | CHECK_MIN_WINDOW_ENABLE)) == 0 || new_cost[p] <= 0)
            continue;

        auto required = 0;
        if ((check_phase[p] & CHECK_MIN_WINDOW_ENABLE) != 0)
            required = wnd_min[p] * cfg->enable_current_factor + new_enable_cost[p];
        else if ((check_phase[p] & CHECK_MIN_WINDOW_MIN) != 0)
            required = wnd_min[p] + new_cost[p];

        if (limits->min[p] < required)
            return false;
    }

    return true;
}

static int get_enable_cost(const ChargerState *state, bool activate_3p, Cost *minimum, Cost *enable, const CurrentAllocatorConfig *cfg) {
    auto min_1p = cfg->minimum_current_1p;
    auto min_3p = cfg->minimum_current_3p;

    // TODO: Add to cfg? Will not change while the firmware is running.
    int ena_1p = min_1p * cfg->enable_current_factor;
    int ena_3p = min_3p * cfg->enable_current_factor;

    Cost new_cost;
    Cost new_enable_cost;
    int enable_current;

    if (activate_3p) {
        enable_current = ena_3p;
        new_cost = Cost{3 * min_3p, min_3p, min_3p, min_3p};
        new_enable_cost = Cost{3 * ena_3p, ena_3p, ena_3p, ena_3p};
    } else if (state->phase_rotation == PhaseRotation::Unknown) {
        enable_current = ena_1p;
        new_cost = Cost{min_1p, min_1p, min_1p, min_1p};
        new_enable_cost = Cost{ena_1p, ena_1p, ena_1p, ena_1p};
    } else {
        enable_current = ena_1p;

        auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);

        new_cost.pv += min_1p;
        new_cost[phase] += min_1p;

        new_enable_cost.pv += ena_1p;
        new_enable_cost[phase] += ena_1p;
    }

    if (minimum != nullptr)
        *minimum = new_cost;
    if (enable != nullptr)
        *enable = new_enable_cost;

    return enable_current;
}

static bool try_activate(const ChargerState *state, bool activate_3p, bool have_active_chargers, Cost *spent, const CurrentLimits *limits, const CurrentAllocatorConfig *cfg,const CurrentAllocatorState *ca_state) {
    Cost wnd_min = ca_state->control_window_min;
    Cost wnd_max = ca_state->control_window_max;

    Cost new_cost;
    Cost new_enable_cost;

    get_enable_cost(state, activate_3p, &new_cost, &new_enable_cost, cfg);

    // If there are no chargers active, don't require the enable cost.
    auto check_min = have_active_chargers ? CHECK_MIN_WINDOW_ENABLE : CHECK_MIN_WINDOW_MIN;

    Cost check_phase{
        CHECK_IMPROVEMENT | check_min,
        CHECK_IMPROVEMENT | check_min,
        CHECK_IMPROVEMENT | check_min,
        CHECK_IMPROVEMENT | check_min
    };

    bool result = can_activate(check_phase, new_cost, new_enable_cost, wnd_min, wnd_max, limits, cfg);
    if (result && spent != nullptr)
        *spent = new_enable_cost;
    return result;
}

// Stage 4: Activate chargers if current is available
// - Only work with chargers that don't already have one or more phase allocated
// - Sort chargers by allocated energy (overall) ascending.
//   We want to enable chargers that have received the least amount of energy first for fairness.
// - Only activate chargers if there is at least one phase limit that is more than the corresponding window maximum.
//   I.e. there is at least one phase were current is available that can not be allocated to already active chargers.
// - Also only activate chargers if the limit is "enable cost" above the window minimum.
//   Enabling a charger costs its minimum current per to-be-active phase * the configured enable factor.
// - Prefer to activate chargers with one phase.
//   Only immediately activate all three phases if this is a non-phase-switchable three-phase charger or one with an unknown phase rotation.
// - Recalculate the window each time a charger is activated
void stage_4(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    if (!ca_state->global_hysteresis_elapsed)
        return;

    int matched = 0;

    bool have_active_chargers = ca_state->control_window_min.pv != 0;

    // A charger that was rotated has 0 allocated phases but is still charging.
    filter(allocated_phases == 0 && (state->wants_to_charge || state->is_charging));

    sort(0,
        left.state->allocated_energy < right.state->allocated_energy
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        bool is_fixed_3p = state->phases == 3 && !state->phase_switch_supported;
        bool is_unknown_rot_switchable = state->phase_rotation == PhaseRotation::Unknown && state->phase_switch_supported;
        bool activate_3p = is_fixed_3p || is_unknown_rot_switchable;

        if (!try_activate(state, activate_3p, have_active_chargers, nullptr, limits, cfg, ca_state)) {
            if (!is_unknown_rot_switchable)
                continue;
            // Retry enabling unknown_rot_switchable charger with one phase only
            activate_3p = false;
            if (!try_activate(state, activate_3p, have_active_chargers, nullptr, limits, cfg, ca_state))
                continue;
        }

        phase_allocation[idx_array[i]] = activate_3p ? 3 : 1;
        calculate_window(idx_array, current_allocation, phase_allocation, limits, charger_state, charger_count, cfg, ca_state);
    }
}

// Stage 5: Switch 1p -> 3p if possible
// - This is conceptionally similar to stage 3 except that chargers already have one phase allocated
// - If there are still phases with a limit greater than the window maximum,
//   switch chargers that have 1 phase allocated to 3 phases if possible
// - Only chargers that _currently_ support the phase switch are considered
// - Sort by allocated energy ascending as in stage 3
// - Enable conditions are similar to stage 3:
//   The enable cost is the cost to enable a charger with three phases minus the cost (that already was subtracted in stage 3) to enable it with one phase.
void stage_5(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    Cost wnd_min = ca_state->control_window_min;
    Cost wnd_max = ca_state->control_window_max;

    auto min_1p = cfg->minimum_current_1p;
    auto min_3p = cfg->minimum_current_3p;

    int ena_1p = min_1p * cfg->enable_current_factor;
    int ena_3p = min_3p * cfg->enable_current_factor;

    int matched = 0;

    // If there is exactly one charger active, the window minimum should be the charger's 1p minimum current,
    // or it is already active with three phases in which case this stage will do nothing.
    bool have_active_chargers = ca_state->control_window_min.pv != min_1p;
    auto check_min = have_active_chargers ? CHECK_MIN_WINDOW_ENABLE : CHECK_MIN_WINDOW_MIN;

    filter(allocated_phases == 1 && state->phase_switch_supported);

    sort(0,
        left.state->allocated_energy < right.state->allocated_energy
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        // If the hysteresis is not elapsed yet, still allow 1p->3p switch for chargers
        // that were activated in this iteration. Those ignored the hysteresis anyway
        // and we can skip the phase switch by immediately allocating three phases.
        // Note that this is **not** the same check as the one in stage 9 (waking up chargers)
        if (!ca_state->global_hysteresis_elapsed && state->allowed_current != 0)
            continue;

        Cost new_cost = Cost{3 * min_3p - min_1p, min_3p, min_3p, min_3p};
        Cost new_enable_cost = Cost{3 * ena_3p - ena_1p, ena_3p, ena_3p, ena_3p};

        // P1 is already active
        auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);

        if (state->phase_rotation == PhaseRotation::Unknown) {
            for (size_t p = 1; p < 4; ++p) {
                new_cost[p] -= min_1p;
                new_enable_cost[p] -= min_1p;
            }
        } else {
            new_cost[phase] -= min_1p;
            new_enable_cost[phase] -= ena_1p;
        }

        // Only switch from one to three phase if there is still current available on **all** phases.
        Cost check_phase{
            CHECK_IMPROVEMENT_ALL_PHASE | check_min,
            (state->phase_rotation == PhaseRotation::Unknown || phase == GridPhase::L1 ? 0 : CHECK_IMPROVEMENT_ALL_PHASE) | check_min,
            (state->phase_rotation == PhaseRotation::Unknown || phase == GridPhase::L2 ? 0 : CHECK_IMPROVEMENT_ALL_PHASE) | check_min,
            (state->phase_rotation == PhaseRotation::Unknown || phase == GridPhase::L3 ? 0 : CHECK_IMPROVEMENT_ALL_PHASE) | check_min
        };

        if (!can_activate(check_phase, new_cost, new_enable_cost, wnd_min, wnd_max, limits, cfg))
            continue;

        phase_allocation[idx_array[i]] = 3;
        calculate_window(idx_array, current_allocation, phase_allocation, limits, charger_state, charger_count, cfg, ca_state);
        wnd_min = ca_state->control_window_min;
        wnd_max = ca_state->control_window_max;
    }
}

// Stage 6: Allocate minimum current to chargers with at least one allocated phase
void stage_6(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_phases > 0);

    // No need to sort here: We know that we have enough current to give each charger its minimum current.
    // A charger that can't be activated has 0 phases allocated.

    auto min_1p = cfg->minimum_current_1p;
    auto min_3p = cfg->minimum_current_3p;

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto allocated_phases = phase_allocation[idx_array[i]];
        Cost cost{0, 0, 0, 0};

        if (allocated_phases == 3) {
            cost = Cost{3 * min_3p, min_3p, min_3p, min_3p};
        } else {
            cost.pv = min_1p;
            if (state->phase_rotation == PhaseRotation::Unknown) {
                for (size_t p = 1; p < 4; ++p)
                    cost[p] += min_1p;
            } else {
                auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
                cost[phase] += min_1p;
            }
        }

        current_allocation[idx_array[i]] = allocated_phases == 3 ? min_3p : min_1p;
        apply_cost(cost, limits);
    }
}

// The current capacity of a charger is the maximum amount of current that can be allocated to the charger additionally to the already allocated current on the allocated phases.
static int32_t current_capacity(const CurrentLimits *limits, const ChargerState *state, int32_t allocated_current, uint8_t allocated_phases) {
    auto requested_current = state->requested_current;

    // TODO: add margin again if exactly one charger is active and requested_current > 6000. Also add in calculate_window?

    if (allocated_phases == 3 || state->phase_rotation == PhaseRotation::Unknown) {
        return std::min({std::max(requested_current - allocated_current, 0), limits->raw.l1, limits->raw.l2, limits->raw.l3});
    }

    auto capacity = std::max(requested_current - allocated_current, 0);
    for (size_t i = (size_t)ChargerPhase::P1; i < (size_t)ChargerPhase::P1 + allocated_phases; ++i) {
        auto phase = get_phase(state->phase_rotation, (ChargerPhase)i);
        capacity = std::min(capacity, limits->raw[phase]);
    }

    return allocated_phases * capacity;
}

// Stage 7: Allocate fair current to chargers with at least one allocated phase
// - All those chargers already have their minimum current allocated
void stage_7(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_current > 0);

    if (matched == 0)
        return;

    // No need to sort here: Each charger gets the same current

    Cost active_on_phase{0, 0, 0, 0};
    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];
        active_on_phase.pv += allocated_phases;
        if (allocated_phases == 3 || state->phase_rotation == PhaseRotation::Unknown) {
            active_on_phase.l1 += 1;
            active_on_phase.l2 += 1;
            active_on_phase.l3 += 1;
        } else {
            for (size_t p = 1; p < 1 + allocated_phases; ++p) {
                auto phase = get_phase(state->phase_rotation, (ChargerPhase)p);
                active_on_phase[phase] += 1;
            }
        }
    }

    // TODO: calculate fair current per phase. Use min fair current of phases used per charger?
    auto fair_current = std::max(0, limits->raw.pv / active_on_phase.pv);
    for (size_t p = 1; p < 4; ++p) {
        if (active_on_phase[p] == 0)
            continue;
        fair_current = std::min(fair_current, limits->raw[p] / active_on_phase[p]);
    }

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto allocated_current = current_allocation[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];

        auto current = fair_current;

        // Don't allocate more than the enable current to a charger that does not charge.
        if (!state->is_charging) {
            Cost enable_cost;
            auto enable_current = get_enable_cost(state, allocated_phases == 3, nullptr, &enable_cost, cfg);
            current = std::min(current, std::max(0, enable_current - allocated_current));
        }

        current = std::min(current, current_capacity(limits, state, allocated_current, allocated_phases));
        current += allocated_current;

        auto cost = get_cost(current, (ChargerPhase)allocated_phases, state->phase_rotation, allocated_current, (ChargerPhase)allocated_phases);

        // This should never happen.
        if (cost_exceeds_limits(cost, limits, 7)) {
            logger.printfln("stage 7: Cost exceeded limits!");
            print_alloc(7, limits, current_allocation, phase_allocation, charger_count, charger_state);
            PRINT_COST(cost);
            PRINT_COST(ca_state->control_window_min);
            PRINT_COST(ca_state->control_window_max);
            continue;
        }

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;
    }
}

// Stage 8: Allocate left-over current.
// - Group by phases. Makes sure that we allocate current to three phase chargers first.
//   If there is current left after this, we've probably hit one of the phase limits.
//   One phase chargers on other phases will take the rest if possible.
// - Sort by current_capacity ascending. This makes sure that one pass is enough to allocate the possible maximum.
void stage_8(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    // Chargers that are currently not charging already have the enable current allocated (if available) by stage 6.
    filter(allocated_current > 0 && state->is_charging);

    sort(
        3 - allocated_phases,
        current_capacity(_limits, left.state, left.allocated_current, left.allocated_phases) < current_capacity(_limits, right.state, right.allocated_current, right.allocated_phases)
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto allocated_current = current_allocation[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];

        auto current = std::min(std::max(0, limits->raw.pv / allocated_phases), current_capacity(limits, state, allocated_current, allocated_phases));

        if (state->phase_rotation == PhaseRotation::Unknown) {
            // Phase rotation unknown. We have to assume that each phase could be used
            current = std::min({current, limits->raw[GridPhase::L1], limits->raw[GridPhase::L2], limits->raw[GridPhase::L3]});
        } else {
            for (int p = 1; p <= (int)allocated_phases; ++p) {
                current = std::min(current, limits->raw[get_phase(state->phase_rotation, (ChargerPhase)p)]);
            }
        }

        current += allocated_current;

        auto cost = get_cost(current, (ChargerPhase)allocated_phases, state->phase_rotation, allocated_current, (ChargerPhase)allocated_phases);

        // This should never happen.
        if (cost_exceeds_limits(cost, limits, 8)) {
            logger.printfln("stage 8: Cost exceeded limits! Charger %d Current %u", idx_array[i], current);
            print_alloc(8, limits, current_allocation, phase_allocation, charger_count, charger_state);
            PRINT_COST(cost);
            PRINT_COST(ca_state->control_window_min);
            PRINT_COST(ca_state->control_window_max);
            continue;
        }

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;
    }
}

static constexpr int CURRENTLY_WAKING_UP_CAR = 0;
static constexpr int NEVER_ATTEMPTED_TO_WAKE_UP = 1;
static constexpr int CAR_DID_NOT_WAKE_UP = 2;

static int stage_9_group(const ChargerState *state) {
    if (state->last_wakeup != 0_usec) {
        if (deadline_elapsed(state->last_wakeup + WAKEUP_TIME))
            return CAR_DID_NOT_WAKE_UP;

        return CURRENTLY_WAKING_UP_CAR;
    }

    return NEVER_ATTEMPTED_TO_WAKE_UP;
}

static bool stage_9_sort(const ChargerState *left_state, const ChargerState *right_state) {
    switch(stage_9_group(left_state)) {
        case CURRENTLY_WAKING_UP_CAR:
            // Prefer newer timestamps, i.e. cars that we have just now allocated current to.
            return left_state->last_wakeup >= right_state->last_wakeup;
        case NEVER_ATTEMPTED_TO_WAKE_UP:
            // Prefer older timestamps, i.e. cars that have been switched off longer.
            return left_state->last_switch < right_state->last_switch;
        case CAR_DID_NOT_WAKE_UP:
            // Prefer older timestamps, i.e. cars that we longer did not attempt to wake up.
            return left_state->last_wakeup < right_state->last_wakeup;
    }
    return false;
}

// Stage 9: Wake up chargers.
// If there is still current left, attempt to wake up one charger with a probably fully charged car.
// Activating the charger will automatically change it from low to normal priority for at least 3 minutes,
// giving the car time to request current.
void stage_9(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_phases == 0 && (state->wants_to_charge_low_priority || (state->wants_to_charge && state->last_wakeup != 0_usec)));

    if (matched == 0)
        return;

    sort(
        stage_9_group(state),
        stage_9_sort(left.state, right.state)
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        // If the hysteresis is not elapsed yet, still allow waking up
        // chargers that had current allocated in the last iteration.
        // A charger that is being woken up is not regarded as active
        // and its current/phases are deallocated in stage 1 to
        // free up the current in case it is needed for "normal" charging.
        // Only if the current is still left over in this iteration, the
        // charger that is being woken up is reallocated the current.
        // Note that this is **not** the same check as the one in stage 5 (1p->3p switch)
        if (!ca_state->global_hysteresis_elapsed && state->allowed_current == 0)
            continue;

        bool is_fixed_3p = state->phases == 3 && !state->phase_switch_supported;
        bool is_unknown_rot_switchable = state->phase_rotation == PhaseRotation::Unknown && state->phase_switch_supported;
        bool activate_3p = is_fixed_3p || is_unknown_rot_switchable;

        Cost enable_cost;
        auto enable_current = get_enable_cost(state, activate_3p, nullptr, &enable_cost, cfg);

        if (cost_exceeds_limits(enable_cost, limits, 9)) {
            if (!is_unknown_rot_switchable)
                continue;
            // Retry enabling unknown_rot_switchable charger with one phase only
            activate_3p = false;
            enable_current = get_enable_cost(state, activate_3p, nullptr, &enable_cost, cfg);
            if (cost_exceeds_limits(enable_cost, limits, 9))
                continue;
        }

        apply_cost(enable_cost, limits);

        phase_allocation[idx_array[i]] = activate_3p ? 3 : 1;
        current_allocation[idx_array[i]] = enable_current;
        calculate_window(idx_array, current_allocation, phase_allocation, limits, charger_state, charger_count, cfg, ca_state);
    }
}

int allocate_current(
    const CurrentAllocatorConfig *cfg,
    const bool seen_all_chargers,
    CurrentLimits *limits,
    const bool cp_disconnect_requested,
    /*const TODO: move allocated_energy into charger allocation state so that this can be const once again*/ ChargerState *charger_state,
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

    if (!seen_all_chargers) {
        limits->raw = Cost{0, 0, 0, 0};
        limits->min = Cost{0, 0, 0, 0};
        limits->max = Cost{0, 0, 0, 0};
    }

    bool print_local_log = false;
    char *local_log = cfg->distribution_log.get();
    if (local_log)
        local_log += snprintf_u(local_log, cfg->distribution_log_len - (local_log - cfg->distribution_log.get()), "Redistributing current%c", '\0');

    bool vehicle_connected = false;

    assert(cfg->charger_count > 0 && cfg->charger_count <= MAX_CONTROLLED_CHARGERS);
    int32_t current_array[MAX_CONTROLLED_CHARGERS] = {0};
    uint8_t phases_array[MAX_CONTROLLED_CHARGERS] = {0};
    int idx_array[MAX_CONTROLLED_CHARGERS] = {0};
    for(int i = 0; i < cfg->charger_count; ++i) {
        idx_array[i] = i;
        phases_array[i] = charger_allocation_state[i].allocated_phases;
    }


    ca_state->global_hysteresis_elapsed = ca_state->last_hysteresis_reset == 0_usec || deadline_elapsed(ca_state->last_hysteresis_reset + GLOBAL_HYSTERESIS);

    //logger.printfln("Hysteresis %selapsed. Last hyst reset %lld. Now %lld", ca_state->global_hysteresis_elapsed ? "" : "not ", (int64_t)(ca_state->last_hysteresis_reset / 1000000_usec), (int64_t)(now_us() / 1000000_usec));

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
            if (charger_alloc.allocated_current < charger.allowed_current && deadline_elapsed(charger_alloc.last_sent_config + 1000_usec * (micros_t)TIMEOUT_MS)) {
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
            limits->raw = Cost{0, 0, 0, 0};
            limits->min = Cost{0, 0, 0, 0};
            limits->max = Cost{0, 0, 0, 0};
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

    //auto start = micros();
    //print_alloc(0, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_1(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(1, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_2(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(2, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_3(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(3, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_4(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(4, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_5(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(5, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_6(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(6, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_7(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(7, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_8(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(8, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_9(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(9, limits, current_array, phases_array, cfg->charger_count, charger_state);
    //logger.printfln("\n");
    //auto end = micros();
    //logger.printfln("Took %u µs", end - start);

    // Apply current limits.
    {
        for (int i = 0; i < cfg->charger_count; ++i) {
            auto &charger = charger_state[i];
            auto &charger_alloc = charger_allocation_state[i];

            uint16_t current_to_set = current_array[i];
            int8_t phases_to_set = phases_array[i];

            LOCAL_LOG("stage 1: Throttled %s (%s) to %d mA.",
                      get_charger_name(i),
                      hosts[i],
                      current_to_set);

            // Don't reset hysteresis if a charger is shut down. Re-activating a charger is (always?) fine.
            if (charger_alloc.allocated_phases != phases_to_set && phases_to_set != 0) {
                // TODO use same timestamp everywhere
                charger.last_switch = now_us();
                ca_state->last_hysteresis_reset = now_us();
            }

            if (charger.wants_to_charge_low_priority && phases_to_set != 0) {
                charger.last_wakeup = now_us();
            }

            bool change = charger_alloc.allocated_current != current_to_set || charger_alloc.allocated_phases != phases_to_set;
            charger_alloc.allocated_current = current_to_set;
            charger_alloc.allocated_phases = phases_to_set;

            if (phases_to_set == 0) {
                charger.allocated_energy_this_rotation = 0;
            } else {
                charger.last_plug_in = 0_usec;

                auto allocated_energy = (float)current_to_set / 1000.0f * phases_to_set * ALLOCATION_TIMEOUT_S / 3600.0f * 230.0f / 1000.0f;
                charger.allocated_energy_this_rotation += allocated_energy;
                charger.allocated_energy += allocated_energy;
            }

            if (change) {
                print_local_log = true;
                if (charger_alloc.error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                    charger_alloc.last_sent_config = now_us();
            }
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
    cm_state_v3 *v3,
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
    if (v1->charger_state != 1 && v1->charger_state != 2)
        target.last_wakeup = 0;

    target.allowed_current = v1->allowed_charging_current;
    target.supported_current = v1->supported_current;
    target.cp_disconnect_supported = CM_FEATURE_FLAGS_CP_DISCONNECT_IS_SET(v1->feature_flags);
    target.cp_disconnect_state = CM_STATE_FLAGS_CP_DISCONNECTED_IS_SET(v1->state_flags);

    // Wait for A -> non-A transitions, but ignore chargers that are already in a non-A state in their first packet.
    if (target.last_update != 0 && target.charger_state == 0 && v1->charger_state != 0)
        target.last_plug_in = now_us();

    target.charger_state = v1->charger_state;
    target.last_update = millis();

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

    if (v3 != nullptr) {
        target.phases = CM_STATE_V3_PHASES_CONNECTED_GET(v3->phases);
        target.phase_switch_supported = CM_FEATURE_FLAGS_PHASE_SWITCH_IS_SET(v1->feature_flags) && CM_STATE_V3_CAN_PHASE_SWITCH_IS_SET(v3->phases);
    } else {
        target.phases = 3;
        target.phase_switch_supported = false;
    }

    return true;
}
