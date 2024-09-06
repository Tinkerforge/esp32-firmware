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

#define ENABLE_CA_TRACE 1
#if ENABLE_CA_TRACE
#define trace(...) logger.tracefln(__VA_ARGS__)
#else
#define trace(...)
#endif

static constexpr int32_t UNLIMITED = 10 * 1000 * 1000; /* mA */

static void print_alloc(int stage, CurrentLimits *limits, int32_t *current_array, uint8_t *phases_array, size_t charger_count, const ChargerState *charger_state) {
    char buf[768] = {};
    logger.printfln("%d LIMITS raw(%6.3f,%6.3f,%6.3f,%6.3f) min(%6.3f,%6.3f,%6.3f,%6.3f) spread(%6.3f,%6.3f,%6.3f,%6.3f) max_pv %6.3f",
           stage,
           limits->raw[0] / 1000.0f,
           limits->raw[1] / 1000.0f,
           limits->raw[2] / 1000.0f,
           limits->raw[3] / 1000.0f,

           limits->min[0] / 1000.0f,
           limits->min[1] / 1000.0f,
           limits->min[2] / 1000.0f,
           limits->min[3] / 1000.0f,

           limits->spread[0] / 1000.0f,
           limits->spread[1] / 1000.0f,
           limits->spread[2] / 1000.0f,
           limits->spread[3] / 1000.0f,

           limits->max_pv / 1000.0f);

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

static void trace_alloc(int stage, CurrentLimits *limits, int32_t *current_array, uint8_t *phases_array, size_t charger_count, const ChargerState *charger_state) {
    #if ENABLE_CA_TRACE
    char buf[768] = {};
    trace("%d: raw(%d %d %d %d) min(%d %d %d %d) spread(%d %d %d %d) max_pv %d",
           stage,
           limits->raw[0],
           limits->raw[1],
           limits->raw[2],
           limits->raw[3],

           limits->min[0],
           limits->min[1],
           limits->min[2],
           limits->min[3],

           limits->spread[0],
           limits->spread[1],
           limits->spread[2],
           limits->spread[3],

           limits->max_pv);

    char *ptr = buf;
    ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "%d: ", stage);
    for(size_t i = 0; i < charger_count; ++i) {
        if (phases_array[i] == 0 && current_array[i] == 0 && stage != 0)
            ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "[        %2zu        ]", i);
        else
            ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "[%2zu %5d@%dp;%4dWh]", i, current_array[i], phases_array[i], (int32_t)(charger_state[i].allocated_energy_this_rotation * 1000.0f));
    }
    trace(buf);
    #endif
}

static void trace_sort_fn(int stage, int matched, int *idx_array, size_t charger_count) {
    #if ENABLE_CA_TRACE
    char buf[200];
    memset(buf, 0, sizeof(buf));
    char *ptr = buf;
    ptr += snprintf(ptr, ARRAY_SIZE(buf) - (ptr - buf), "%d: filtered %d to %d, sorted to ", stage, charger_count, matched);
    if (matched == 0)
        ptr += snprintf(ptr, ARRAY_SIZE(buf) - (ptr - buf), "| ");

    for(int i = 0; i < charger_count; ++i)
        ptr += snprintf(ptr, ARRAY_SIZE(buf) - (ptr - buf), "%d %s", idx_array[i], i == (matched - 1) ? "| " : "");
    trace("%s", buf);
    #endif
}

#define trace_sort(x) trace_sort_fn(x, matched, idx_array, charger_count)

// Sorts the indices of chargers that match the filter to the front of idx_array and returns the number of matches.
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

// Sorts the indices of chargers by first grouping them with the group function and then comparing in groups with the sort function.
void sort_chargers(group_fn group, compare_fn compare, int *idx_array, const int32_t *current_allocation, const uint8_t *phase_allocation, const ChargerState *charger_state, size_t charger_count, CurrentLimits *limits, const CurrentAllocatorConfig *cfg) {
    int groups[MAX_CONTROLLED_CHARGERS] = {};

    for(int i = 0; i < charger_count; ++i)
        groups[idx_array[i]] = group(current_allocation[idx_array[i]], phase_allocation[idx_array[i]], &charger_state[idx_array[i]], cfg);

    std::stable_sort(
        idx_array,
        idx_array + charger_count,
        [&groups, &compare, &current_allocation, &phase_allocation, &charger_state, &limits, &cfg] (int left, int right) {
            if (groups[left] != groups[right])
                return groups[left] < groups[right];

            return compare({current_allocation[left], phase_allocation[left], &charger_state[left]},
                           {current_allocation[right], phase_allocation[right], &charger_state[right]},
                           limits, cfg);
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
        phases_exceeded |= cost[i] > 0 && limits->raw[i] < cost[i];
    }

    bool pv_excess_exceeded = cost.pv > 0 && limits->raw.pv < cost.pv;

    switch(stage) {
        case 6:
            // Intentionally ignore the phase limit in stage 6:
            // If we currently don't have the minimum current available
            // on the PV "phase" we don't want to shut down the charger immediately.
            return phases_exceeded;
        case 7:
        case 8:
            return phases_exceeded || pv_excess_exceeded;
        case 9: {
            bool phases_min_exceeded = false;
            for (size_t i = (size_t)GridPhase::L1; i <= (size_t)GridPhase::L3; ++i) {
                phases_min_exceeded |= cost[i] > 0 && limits->min[i] < cost[i];
            }

            bool pv_excess_min_exceeded = cost.pv > 0 && limits->min.pv < cost.pv;
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
        // No need to apply the cost to the spread limit:
        // It is only used for the window calculation, i.e. before costs are applied.
    }
    limits->max_pv -= cost.pv;
}

// A charger is active if
// - we have allocated current to it or will in the future (thus checking the allocated phases)
// - it wants to charge (i.e. a vehicle is plugged in and no other slot blocks) or is charging (i.e. is in state C)
// - we are not currently attempting to wake up a "full" vehicle
static bool is_active(uint8_t allocated_phases, const ChargerState *state) {
    return allocated_phases > 0 && (state->wants_to_charge || state->is_charging) && state->last_wakeup == 0_us;
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
    // This charger also has to be plugged in for at least one allocation iteration, to make sure
    // we don't rotate chargers out immediately if a new charger is plugged in. Maybe there is
    // enough current available to activate both of them.
    //
    // First get the phase(s) that we have to make some room for the B1 charger(s) on.
    // We don't have to handle the PV "phase" because an active charger will draw the PV current on some of the phases.
    bool have_b1 = false;
    Cost b1_on_phase= {0, 0, 0, 0};
    for (int i = 0; i < charger_count; ++i) {
        const auto *state = &charger_state[i];
        bool is_b1 = state->wants_to_charge && phase_allocation[i] == 0 && deadline_elapsed(state->last_plug_in + cfg->allocation_interval);

        if (is_b1) {
            have_b1 = true;
            if (state->phases == 3 || state->phase_rotation == PhaseRotation::Unknown) {
                // We only care about the phases that are blocked, not about how many chargers are waiting.
                b1_on_phase.l1 = 1;
                b1_on_phase.l2 = 1;
                b1_on_phase.l3 = 1;
            } else {
                // Not 2p safe!
                auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
                b1_on_phase[phase] = 1;
            }
        }
    }
    trace(have_b1 ? "1: have B1" : "1: don't have B1");

    // Deallocate a charger if it should be rotated or if it reports that it is not active anymore.
    // Note that a charger that was activated in stage 9 (to wake up a full vehicle)
    // it will always be deactivated here to make sure we only wake up vehicles if there is current left over.
    for (int i = 0; i < charger_count; ++i) {
        if (phase_allocation[i] == 0)
            continue;

        const auto *state = &charger_state[i];

        bool alloc_energy_over_thres = state->allocated_energy_this_rotation >= cfg->allocated_energy_rotation_threshold;
        bool min_active_elapsed = deadline_elapsed(state->last_switch + cfg->minimum_active_time);

        bool rotate = have_b1 && ca_state->global_hysteresis_elapsed && alloc_energy_over_thres && min_active_elapsed;

        // A 3p or unknown rotated charger is active on all phases -> can be rotated immediately.
        if (rotate && (state->phases != 3 && state->phase_rotation == PhaseRotation::Unknown)) {
            // Not 2p safe!
            auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
            rotate &= b1_on_phase[phase] == 1;
        }

        bool keep_active = is_active(phase_allocation[i], state) && !rotate;

        trace("1: %d: alloc_ge_thres %c min_active %c rot %c keep_active %c can p-switch %c",
                        i,
                        alloc_energy_over_thres ? '1' : '0',
                        min_active_elapsed ? '1' : '0',
                        rotate ? '1' : '0',
                        keep_active ? '1' : '0',
                        state->phase_switch_supported ? '1' : '0');

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

// Only regard a charger as just plugged in if it wants to charge.
// If another current limit blocks this charger (for example the user/NFC limit)
// there is not need to allocate current to it.
static bool was_just_plugged_in(const ChargerState *state) {
    return state->last_plug_in != 0_us && state->wants_to_charge;
}

// Stage 2: Immediately activate chargers were a vehicle was just plugged in.
// Do this before calculating the initial control window and ignore limits.
// If we exceed limits by activating those chargers, the next stage will
// shut down other chargers if necessary.
// Both this stage and stage 3 sort by the last_plug_in timestamp,
// so if we have more chargers with a timestamp than can be activated at once,
// we will activate some, give them current for some time and then activate the next group.
// Because a charger's last_plug_in timestamp is only cleared once it has been charging for the plug_in_time,
// we won't toggle the contactors too fast.
// This feature is completely deactivated if cfg->plug_in_time is set to 0.
void stage_2(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(was_just_plugged_in(state));

    // Charger that is plugged in for the longest time first.
    sort(0,
        left.state->last_plug_in < right.state->last_plug_in
    );

    trace_sort(2);

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        bool is_fixed_3p = state->phases == 3 && !state->phase_switch_supported;
        bool activate_3p = is_fixed_3p;

        // Don't override existing allocation.
        // This charger could be switched to 3p.
        if (phase_allocation[idx_array[i]] == 0)
            phase_allocation[idx_array[i]] = activate_3p ? 3 : 1;

        trace("2: %d: plugged in. alloc %dp", idx_array[i], phase_allocation[idx_array[i]]);
    }
}

// Use the supported current in case the last allocation was able to fulfill the requested current.
// In that case we want a fast ramp-up until we know the new limit of the charger (or don't have any current left)
static int32_t get_requested_current(const ChargerState *state, const CurrentAllocatorConfig *cfg) {
    if (state->last_alloc_fulfilled_reqd && !deadline_elapsed(state->ignore_phase_currents + micros_t{cfg->requested_current_threshold} * 1_s))
        return state->supported_current;

    return state->requested_current;
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
    trace("  Calc Wnd");
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

        trace("    %d wnd_min (%d %d %d %d)", idx_array[i], wnd_min.pv, wnd_min.l1, wnd_min.l2, wnd_min.l3);
    }

    // Calculate left over current for 3p chargers after allocating the 1p minimum to 1p chargers
    auto current_avail_for_3p = UNLIMITED;

    for (size_t i = 0; i < 4; ++i) {
        auto avail_on_phase = limits->raw[i] - wnd_min_1p[i];
        current_avail_for_3p = std::min(current_avail_for_3p, avail_on_phase);
    }

    trace("    current_avail_for_3p %d", current_avail_for_3p);

    // Add maximum window of 3p chargers.
    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases != 3)
            continue;

        auto requested_current = get_requested_current(state, cfg);

        wnd_max += Cost{0,
                        requested_current,
                        requested_current,
                        requested_current};

        // It is sufficient to check one phase here, wnd_max should have the same value on every phase because only three phase chargers are included yet
        if (wnd_max.l1 > current_avail_for_3p) {
            wnd_max = Cost{wnd_max.l1 + (current_avail_for_3p - wnd_max.l1) * alloc_phases,
                           current_avail_for_3p,
                           current_avail_for_3p,
                           current_avail_for_3p};
            trace("    %d (3p) wnd_max (%d %d %d %d).l1 == current_avail_for_3p.", idx_array[i], wnd_max.pv, wnd_max.l1, wnd_max.l2, wnd_max.l3);
            break;
        }

        wnd_max.pv += requested_current * alloc_phases;

        trace("    %d (3p) wnd_max (%d %d %d %d)", idx_array[i], wnd_max.pv, wnd_max.l1, wnd_max.l2, wnd_max.l3);
    }

    // Add maximum window of 1p chargers with unknown rotation.
    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases == 3 || state->phase_rotation != PhaseRotation::Unknown)
            continue;

        auto current = get_requested_current(state, cfg);
        for (size_t p = 1; p < 4; ++p) {
            auto avail_on_phase = limits->raw[p] - wnd_max[p];
            current = std::min(current, avail_on_phase);
        }

        wnd_max += Cost{current,
                        current,
                        current,
                        current};

        trace("    %d (1p unknown rot) wnd_max (%d %d %d %d)", idx_array[i], wnd_max.pv, wnd_max.l1, wnd_max.l2, wnd_max.l3);
    }

    // Add maximum window of 1p chargers with known rotation.
    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases == 3 || state->phase_rotation == PhaseRotation::Unknown)
            continue;

        const auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
        const auto current = std::min(limits->raw[phase] - wnd_max[phase], get_requested_current(state, cfg));

        wnd_max[phase] += current;
        wnd_max.pv += current;

    trace("    %d (1p known rot) wnd_max (%d %d %d %d)", idx_array[i], wnd_max.pv, wnd_max.l1, wnd_max.l2, wnd_max.l3);
    }

    ca_state->control_window_min = wnd_min;
    ca_state->control_window_max = wnd_max;

    trace("  Wnd (%d %d %d %d)->(%d %d %d %d)", wnd_min.pv, wnd_min.l1, wnd_min.l2, wnd_min.l3, wnd_max.pv, wnd_max.l1, wnd_max.l2, wnd_max.l3);

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
    // Group chargers that were activated in stage 2 (because a vehicle was just plugged in)
    // behind others and sort those by the plug in timestamp to make sure the same chargers are shut down
    // if phases are overloaded in every iteration.
    sort(
        was_just_plugged_in(state) ? 1 : 0,
        was_just_plugged_in(left.state) ? (left.state->last_plug_in < right.state->last_plug_in) : (left.state->last_switch < right.state->last_switch)
    );

    trace_sort(3);

    bool any_charger_shut_down = false;

    // If any phase is overloaded, shut down chargers using this phase until it is not.
    // Maybe try to be more clever here:
    // - Shut down 1p chargers first if only one or two phase limits are below wnd_min
    // - Shut down 1p unknown rotated chargers last
    for (size_t p = 1; p < 4; ++p) {
        for (int i = 0; i < matched; ++i) {
            if (wnd_min[p] <= limits->raw[p]) {
                // Window minimum less than raw phase limit -> phase not overloaded
                trace("3: wnd_min %d <= p%d raw %d", wnd_min[p], p, limits->raw[p]);
                break;
            }

            const auto *state = &charger_state[idx_array[i]];
            const auto alloc_phases = phase_allocation[idx_array[i]];

            if (alloc_phases == 0)
                continue;

            trace("3: wnd_min %d > p%d raw %d", wnd_min[p], p, limits->raw[p]);

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

            trace("3: shut down %d", idx_array[i]);

            any_charger_shut_down = true;
        }
    }

    // If the max PV excess over the last x minutes is less than the PV minimum window,
    // we want to shut down chargers and this decision was stable for x minutes.
    // Also check the hysteresis to make sure the last switch on/off decisions
    // did propagate to the calculated limits.
    for (int i = 0; i < matched; ++i) {
        if (wnd_min.pv <= limits->max_pv || !ca_state->global_hysteresis_elapsed) {
            // Window minimum less than max pv limit -> PV not permanently overloaded
            // or hysteresis is not elapsed yet.
            if (wnd_min.pv <= limits->max_pv) {
                trace("3: wnd_min %d <= max_pv %d", wnd_min.pv, limits->max_pv);
            } else {
                trace("3: wnd_min %d > max_pv %d hyst not elapsed", wnd_min.pv, limits->max_pv);
            }

            break;
        }

        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        // Don't shut down chargers that were just plugged in.
        // We want those to be active until plug_in_time is elapsed to make sure the vehicle does not disable its EVCC.
        if (alloc_phases == 0 || was_just_plugged_in(state))
            continue;

        trace("3: wnd_min %d > max_pv %d", wnd_min.pv, limits->max_pv);

        // We don't have to recalculate the window but instead can just change the minimum.
        // The window minimum does not have dependencies between chargers.
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

        trace("3: shut down %d", idx_array[i]);

        any_charger_shut_down = true;
    }

    // Recalculate the window once. wnd_min is already correct, but wnd_max needs to be fixed.
    if (any_charger_shut_down) {
        calculate_window(idx_array, current_allocation, phase_allocation, limits, charger_state, charger_count, cfg, ca_state);
    }
}

// A charger can be activated if
// - (1) the enable cost of all active and the new charger is less than the spread limit on all relevant phases (if CHECK_SPREAD is set for at least one phase)
//   OR (2) (
//      - wnd_max.pv is less than the min PV limit (if CHECK_IMPROVEMENT or CHECK_IMPROVEMENT_ALL_PHASE is set for the PV phase)
//      - wnd_max is less than the min limit of
//          - at least one phase that the charger is active on (if CHECK_IMPROVEMENT is set for that phase)
//          - or all phases that the charger is active on (if CHECK_IMPROVEMENT_ALL_PHASE is set for that phase)
//        and activating the charger moves wnd_max closer to the limit (i.e. its cost on this phase is positive)
//   )
// - AND (3) the
//      - enable cost (if CHECK_MIN_WINDOW_ENABLE is set for that phase)
//      - cost  (if CHECK_MIN_WINDOW_MIN is set for that phase)
//   does not exceed the min on any phase or PV that the charger is active on
// tl;dr: A charger can be activated if (3) wouldn't have to shut it down in the last few minutes (minimum limit)
//        and (2) there's current available on any/all relevant phase(s) even if all already active chargers are at their maximum (maximum limit)
//        or (1) we wouldn't have to shut down the charger in the last few hours (spread limit)
static constexpr int CHECK_MIN_WINDOW_MIN = 1;
static constexpr int CHECK_MIN_WINDOW_ENABLE = 2;
static constexpr int CHECK_IMPROVEMENT = 4;
static constexpr int CHECK_IMPROVEMENT_ALL_PHASE = 8;
static constexpr int CHECK_SPREAD = 16;
static bool can_activate(const Cost check_phase, const Cost new_cost, const Cost new_enable_cost, const Cost wnd_min, const Cost wnd_max, const CurrentLimits *limits, const CurrentAllocatorConfig *cfg, bool is_unknown_rotated_1p_3p_switch) {
    // Spread
    bool check_spread = ((check_phase.pv | check_phase.l1 | check_phase.l2 | check_phase.l3) & CHECK_SPREAD) != 0;
    bool improves_all_spread = true;
    for (size_t p = 0; p < 4; ++p) {
        if ((check_phase[p] & CHECK_SPREAD) == 0 || new_cost[p] <= 0)
            continue;

        auto required = wnd_min[p] * cfg->enable_current_factor + new_enable_cost[p];

        if (limits->spread[p] < required) {
            improves_all_spread = false;
        }
    }

    // Improvement - Check only if spread was not checked or was checked but new charger did not fit.
    if (!check_spread || !improves_all_spread) {
        if (!improves_all_spread)
            trace("    Does not improve spread");

        bool improves_pv = (check_phase.pv & (CHECK_IMPROVEMENT | CHECK_IMPROVEMENT_ALL_PHASE)) == 0 || (new_cost.pv > 0 && wnd_max.pv < limits->min.pv);
        if (!improves_pv) {
            trace("    Can't activate: does not improve PV");
            return false;
        }

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
            trace("    Can't activate: check_all %c improves_all %c improves_any %c is_unknown_rotated_1p_3p_switch %c",
                            check_all_phases ? '1' : '0',
                            improves_all_phases ? '1' : '0',
                            improves_any_phase ? '1' : '0',
                            is_unknown_rotated_1p_3p_switch ? '1' : '0');

            // PV is already checked above.
            bool enable = is_unknown_rotated_1p_3p_switch;
            if (!enable)
                return false;
        }
    } else {
        trace("    Improves spread");
    }

    // Check minimum
    for (size_t p = 0; p < 4; ++p) {
        if ((check_phase[p] & (CHECK_MIN_WINDOW_MIN | CHECK_MIN_WINDOW_ENABLE)) == 0 || new_cost[p] <= 0)
            continue;

        auto required = 0;
        if ((check_phase[p] & CHECK_MIN_WINDOW_ENABLE) != 0)
            required = wnd_min[p] * cfg->enable_current_factor + new_enable_cost[p];
        else if ((check_phase[p] & CHECK_MIN_WINDOW_MIN) != 0)
            required = wnd_min[p] + new_cost[p];

        if (limits->min[p] < required) {
            trace("    Can't activate: p%d min %d < required %d", p, limits->min[p], required);
            return false;
        }
    }

    return true;
}

// Returns
// - (in *minimum) the minimum cost to keep this charger active
// - (in *enable)  the cost to enable this charger
// - (as return value) the enable current of each of the phases that the charger would be active on
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

// Activates the charger if possible.
// Returns false if the charger can't be activated (see can_activate).
// If true is returned, *spent is the cost that was spent to enable the charger.
static bool try_activate(const ChargerState *state, bool activate_3p, bool have_active_chargers, Cost *spent, const CurrentLimits *limits, const CurrentAllocatorConfig *cfg,const CurrentAllocatorState *ca_state) {
    Cost wnd_min = ca_state->control_window_min;
    Cost wnd_max = ca_state->control_window_max;

    Cost new_cost;
    Cost new_enable_cost;

    get_enable_cost(state, activate_3p, &new_cost, &new_enable_cost, cfg);

    // If there are no chargers active, don't require the enable cost on PV.
    // Still require the enable cost on the phases:
    // Phase limits are hard limits. PV can be exceeded for some time.
    Cost check_phase{
        CHECK_SPREAD | CHECK_IMPROVEMENT | (have_active_chargers ? CHECK_MIN_WINDOW_ENABLE : CHECK_MIN_WINDOW_MIN),
        CHECK_SPREAD | CHECK_IMPROVEMENT | CHECK_MIN_WINDOW_ENABLE,
        CHECK_SPREAD | CHECK_IMPROVEMENT | CHECK_MIN_WINDOW_ENABLE,
        CHECK_SPREAD | CHECK_IMPROVEMENT | CHECK_MIN_WINDOW_ENABLE
    };

    bool result = can_activate(check_phase, new_cost, new_enable_cost, wnd_min, wnd_max, limits, cfg, false);
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

    trace(have_active_chargers ? "4: have active chargers." : "4: don't have active chargers.");

    // A charger that was rotated has 0 allocated phases but is still charging.
    filter(allocated_phases == 0 && (state->wants_to_charge || state->is_charging));

    sort(0,
        left.state->allocated_energy < right.state->allocated_energy
    );

    trace_sort(4);

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        bool is_fixed_3p = state->phases == 3 && !state->phase_switch_supported;
        bool is_unknown_rot_switchable = state->phase_rotation == PhaseRotation::Unknown && state->phase_switch_supported;
        // Prefer unknown rotated switchable chargers to be active on all three phases.
        // In that case we know what the charger is doing.
        bool activate_3p = is_fixed_3p || is_unknown_rot_switchable;

        trace("4: Can activate %d?", idx_array[i]);
        if (!try_activate(state, activate_3p, have_active_chargers, nullptr, limits, cfg, ca_state)) {
            trace("4: No");
            if (!is_unknown_rot_switchable)
                continue;
            trace("4: %d retrying 1p", idx_array[i]);
            // Retry enabling unknown_rot_switchable charger with one phase only
            activate_3p = false;
            if (!try_activate(state, activate_3p, have_active_chargers, nullptr, limits, cfg, ca_state)) {
                trace("4: No");
                continue;
            }
        }

        phase_allocation[idx_array[i]] = activate_3p ? 3 : 1;

        trace("4: Yes (%dp)", phase_allocation[idx_array[i]]);

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

    trace(have_active_chargers ? "5: have active chargers." : "5: don't have active chargers.");

    filter(allocated_phases == 1 && state->phase_switch_supported);

    sort(0,
        left.state->allocated_energy < right.state->allocated_energy
    );

    trace_sort(5);

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
            (state->phase_rotation == PhaseRotation::Unknown || phase == GridPhase::L1 ? 0 : CHECK_IMPROVEMENT_ALL_PHASE) | CHECK_MIN_WINDOW_ENABLE,
            (state->phase_rotation == PhaseRotation::Unknown || phase == GridPhase::L2 ? 0 : CHECK_IMPROVEMENT_ALL_PHASE) | CHECK_MIN_WINDOW_ENABLE,
            (state->phase_rotation == PhaseRotation::Unknown || phase == GridPhase::L3 ? 0 : CHECK_IMPROVEMENT_ALL_PHASE) | CHECK_MIN_WINDOW_ENABLE
        };

        trace("5: Can %d 1p->3p switched?", idx_array[i]);
        if (!can_activate(check_phase, new_cost, new_enable_cost, wnd_min, wnd_max, limits, cfg, state->phase_rotation == PhaseRotation::Unknown)) {
            trace("5: No");
            continue;
        }

        trace("5: Yes");

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

    trace_sort(6);

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

        // This should never happen:
        // We should never allocate more current than the raw phase limits,
        // not even to allocate the minimum current to a charger. If there
        // is a charger that was allocated phases to before stage 6, but we
        // don't have the minimum current available, that is a bug in the
        // previous stages.
        if (cost_exceeds_limits(cost, limits, 6)) {
            logger.printfln("stage 6: Cost exceeded limits!");
            print_alloc(6, limits, current_allocation, phase_allocation, charger_count, charger_state);
            PRINT_COST(cost);
            PRINT_COST(ca_state->control_window_min);
            PRINT_COST(ca_state->control_window_max);
            continue;
        }

        current_allocation[idx_array[i]] = allocated_phases == 3 ? min_3p : min_1p;
        trace("6: %d: %d@%dp", idx_array[i], current_allocation[idx_array[i]], allocated_phases);
        apply_cost(cost, limits);
    }
}

// The current capacity of a charger is the maximum amount of current that can be allocated to the charger additionally to the already allocated current on the allocated phases.
static int32_t current_capacity(const CurrentLimits *limits, const ChargerState *state, int32_t allocated_current, uint8_t allocated_phases, const CurrentAllocatorConfig *cfg) {
    auto requested_current = get_requested_current(state, cfg);

    // TODO: add margin again if exactly one charger is active and requested_current > 6000. Also add in calculate_window? -> Maybe not necessary any more?

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

static Cost get_fair_current(int matched, int start, int *idx_array, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state) {
    Cost active_on_phase{0, 0, 0, 0};
    for (int i = start; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];
        ++active_on_phase.pv;
        if (allocated_phases == 3 || state->phase_rotation == PhaseRotation::Unknown) {
            ++active_on_phase.l1;
            ++active_on_phase.l2;
            ++active_on_phase.l3;
        } else {
            for (size_t p = 1; p < 1 + allocated_phases; ++p) {
                auto phase = get_phase(state->phase_rotation, (ChargerPhase)p);
                ++active_on_phase[phase];
            }
        }
    }

    Cost fair = {0, 0, 0, 0};

    for (size_t p = 0; p < 4; ++p) {
        if (active_on_phase[p] == 0)
            continue;
        // Make sure the fair current is never negative to not steal a charger's minimum current.
        fair[p] = std::max(0, limits->raw[p] / active_on_phase[p]);
    }

    trace("7: active (%d %d %d %d) fair (%d %d %d %d)", active_on_phase.pv, active_on_phase.l1, active_on_phase.l2, active_on_phase.l3, fair.pv, fair.l1, fair.l2, fair.l3);

    return fair;
}

// Stage 7: Allocate fair current to chargers with at least one allocated phase
// - All those chargers already have their minimum current allocated
// - The fair current is the current left divided by the number of chargers active on that phase.
//   On the PV "phase" include a charger n times were n is the number of phases this charger uses.
//   A three-phase charger will use 18 A of PV current if it is allocated 6 A to each phase.
void stage_7(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_current > 0);

    if (matched == 0)
        return;

    sort(
        3 - allocated_phases,
        current_capacity(_limits, left.state, left.allocated_current, left.allocated_phases, _cfg) < current_capacity(_limits, right.state, right.allocated_current, right.allocated_phases, _cfg)
    );

    trace_sort(7);

    for (int i = 0; i < matched; ++i) {
        Cost fair = get_fair_current(matched, i, idx_array, phase_allocation, limits, charger_state);

        const auto *state = &charger_state[idx_array[i]];

        auto allocated_current = current_allocation[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];

        auto current = fair.pv / allocated_phases;

        if (state->phase_rotation == PhaseRotation::Unknown) {
            current = std::min({current, fair.l1, fair.l2, fair.l3});
        } else {
            for (size_t p = 0; p < allocated_phases; ++p) {
                auto phase = get_phase(state->phase_rotation, (ChargerPhase)((size_t)ChargerPhase::P1 + p));
                current = std::min(current, fair[phase]);
            }
        }

        // Don't allocate more than the enable current to a charger that does not charge.
        if (!state->is_charging) {
            trace("7: %d not charging", idx_array[i]);
            Cost enable_cost;
            auto enable_current = get_enable_cost(state, allocated_phases == 3, nullptr, &enable_cost, cfg);
            current = std::min(current, std::max(0, enable_current - allocated_current));
        }

        current = std::min(current, current_capacity(limits, state, allocated_current, allocated_phases, cfg));
        current += allocated_current;

        auto cost = get_cost(current, (ChargerPhase)allocated_phases, state->phase_rotation, allocated_current, (ChargerPhase)allocated_phases);

        // This should never happen:
        // We've just calculated how much current is still available.
        // If this cost exceeds the limits, stage_7 is bugged.
        if (cost_exceeds_limits(cost, limits, 7)) {
            logger.printfln("stage 7: Cost exceeded limits!");
            print_alloc(7, limits, current_allocation, phase_allocation, charger_count, charger_state);
            PRINT_COST(cost);
            PRINT_COST(ca_state->control_window_min);
            PRINT_COST(ca_state->control_window_max);
            continue;
        }

        current_allocation[idx_array[i]] = current;
        apply_cost(cost, limits);
        trace("7: %d: %d@%dp", idx_array[i], current_allocation[idx_array[i]], allocated_phases);
    }
}

// Stage 8: Allocate left-over current.
// - Group by phases. Makes sure that we allocate current to three phase chargers first.
//   If there is current left after this, we've probably hit one of the phase limits.
//   One phase chargers on other phases will take the rest if possible.
// - Sort by current_capacity ascending. This makes sure that one pass is enough to allocate the possible maximum.
void stage_8(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    // Chargers that are currently not charging already have the enable current allocated (if available) by stage 7.
    filter(allocated_current > 0 && state->is_charging);

    sort(
        3 - allocated_phases,
        current_capacity(_limits, left.state, left.allocated_current, left.allocated_phases, _cfg) < current_capacity(_limits, right.state, right.allocated_current, right.allocated_phases, _cfg)
    );

    trace_sort(8);

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto allocated_current = current_allocation[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];

        auto current = std::min(std::max(0, limits->raw.pv / allocated_phases), current_capacity(limits, state, allocated_current, allocated_phases, cfg));

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

        // This should never happen:
        // We've just calculated how much current is still available.
        // If this cost exceeds the limits, stage_8 is bugged.
        if (cost_exceeds_limits(cost, limits, 8)) {
            logger.printfln("stage 8: Cost exceeded limits! Charger %d Current %u", idx_array[i], current);
            print_alloc(8, limits, current_allocation, phase_allocation, charger_count, charger_state);
            PRINT_COST(cost);
            PRINT_COST(ca_state->control_window_min);
            PRINT_COST(ca_state->control_window_max);
            continue;
        }

        current_allocation[idx_array[i]] = current;
        apply_cost(cost, limits);
        trace("8: %d: %d@%dp", idx_array[i], current_allocation[idx_array[i]], allocated_phases);
    }
}

static constexpr int CURRENTLY_WAKING_UP_CAR = 0;
static constexpr int NEVER_ATTEMPTED_TO_WAKE_UP = 1;
static constexpr int CAR_DID_NOT_WAKE_UP = 2;

static int stage_9_group(const ChargerState *state, const CurrentAllocatorConfig *cfg) {
    if (state->last_wakeup != 0_us) {
        if (deadline_elapsed(state->last_wakeup + cfg->wakeup_time))
            return CAR_DID_NOT_WAKE_UP;

        return CURRENTLY_WAKING_UP_CAR;
    }

    return NEVER_ATTEMPTED_TO_WAKE_UP;
}

static bool stage_9_sort(const ChargerState *left_state, const ChargerState *right_state, const CurrentAllocatorConfig *cfg) {
    switch(stage_9_group(left_state, cfg)) {
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
// If there is still current left, attempt to wake up chargers with a probably fully charged car.
// Activating a charger will automatically change it from low to normal priority for at least 3 minutes,
// giving the car time to request current.
void stage_9(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_phases == 0 && (state->wants_to_charge_low_priority || (state->wants_to_charge && state->last_wakeup != 0_us)));

    if (matched == 0)
        return;

    sort(
        stage_9_group(state, _cfg),
        stage_9_sort(left.state, right.state, _cfg)
    );

    trace_sort(9);

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
        trace("9: %d: %d@%dp", idx_array[i], current_allocation[idx_array[i]], phase_allocation[idx_array[i]]);

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
#if ENABLE_CA_TRACE
    logger.trace_timestamp();
#endif

    // TODO use enum for this. See charge_manager.cpp state definition for constants.
    int result = 1;

    if (!seen_all_chargers) {
        trace("Did not see all chargers yet!");
        limits->raw = Cost{0, 0, 0, 0};
        limits->min = Cost{0, 0, 0, 0};
        limits->max_pv = 0;
    }

    bool print_local_log = false;
    char *local_log = cfg->distribution_log.get();
    if (local_log)
        local_log += snprintf_u(local_log, cfg->distribution_log_len - (local_log - cfg->distribution_log.get()), "Allocating current%c", '\0');

    bool vehicle_connected = false;

    assert(cfg->charger_count > 0 && cfg->charger_count <= MAX_CONTROLLED_CHARGERS);
    int32_t current_array[MAX_CONTROLLED_CHARGERS] = {0};
    uint8_t phases_array[MAX_CONTROLLED_CHARGERS] = {0};
    int idx_array[MAX_CONTROLLED_CHARGERS] = {0};
    for(int i = 0; i < cfg->charger_count; ++i) {
        idx_array[i] = i;
        phases_array[i] = charger_allocation_state[i].allocated_phases;
    }

    ca_state->global_hysteresis_elapsed = ca_state->last_hysteresis_reset == 0_us || deadline_elapsed(ca_state->last_hysteresis_reset + cfg->global_hysteresis);

    trace("Hysteresis %lld", (int64_t)(now_us() - ca_state->last_hysteresis_reset - cfg->global_hysteresis) / 1000000);

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
                LOCAL_LOG("%s (%s) reports error %u.", get_charger_name(i), hosts[i], charger_error);

                print_local_log = !ca_state->last_print_local_log_was_error;
                ca_state->last_print_local_log_was_error = true;
            }

            // Charger does not respond anymore
            if (deadline_elapsed(charger.last_update + TIMEOUT_MS)) {
                unreachable_evse_found = true;
                LOCAL_LOG("Can't reach EVSE of %s (%s): last_update too old.", get_charger_name(i), hosts[i]);

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

            // Charger did not update the charging current or phases in time.
            // It is not an error if the charger reports != 0 phases but 0 are allocated,
            // because the charger reports the number of phases that would be active if
            // it was charging. Check is_charging instead.
            if ((charger_alloc.allocated_current < charger.allowed_current
                || (charger_alloc.allocated_phases != 0 && charger_alloc.allocated_phases < charger.phases)
                || (charger_alloc.allocated_phases == 0 && charger.is_charging))
               && deadline_elapsed(charger_alloc.last_sent_config + 1_ms * (micros_t)TIMEOUT_MS)) {
                unreachable_evse_found = true;
                LOCAL_LOG("EVSE of %s (%s) did not react in time. Expected %d mA @ %dp but is %d mA @ %dp",
                          get_charger_name(i),
                          hosts[i],
                          charger_alloc.allocated_current,
                          charger_alloc.allocated_phases,
                          charger.allowed_current,
                          charger.phases);

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
            // TODO: Don't block energy manager firmware updates if a charger can't be reached or is in an error state.
            // Only block if state is in [1.4] or 6
            if (charger_alloc.state != 0)
                vehicle_connected = true;
        }

        if (unreachable_evse_found) {
            // Shut down everything.
            limits->raw = Cost{0, 0, 0, 0};
            limits->min = Cost{0, 0, 0, 0};
            limits->max_pv = 0;
            LOCAL_LOG("%s", "Unreachable, unreactive or misconfigured EVSE(s) found. Setting available current to 0 mA.");
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
    trace_alloc(0, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_1(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //trace_alloc(1, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_2(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //trace_alloc(2, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_3(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //trace_alloc(3, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_4(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //trace_alloc(4, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_5(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //trace_alloc(5, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_6(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //trace_alloc(6, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_7(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //trace_alloc(7, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_8(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //trace_alloc(8, limits, current_array, phases_array, cfg->charger_count, charger_state);
    stage_9(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    trace_alloc(9, limits, current_array, phases_array, cfg->charger_count, charger_state);
    //auto end = micros();
    //logger.printfln("Took %u µs", end - start);

    auto now = now_us();

    // Apply current limits.
    {
        for (int i = 0; i < cfg->charger_count; ++i) {
            auto &charger = charger_state[i];
            auto &charger_alloc = charger_allocation_state[i];

            uint16_t current_to_set = current_array[i];
            int8_t phases_to_set = phases_array[i];

            // Don't reset hysteresis if a charger is shut down. Re-activating a charger is (always?) fine.
            if (charger_alloc.allocated_phases != phases_to_set && phases_to_set != 0) {
                charger.last_switch = now;
                ca_state->last_hysteresis_reset = now;
            }

            if (charger.wants_to_charge_low_priority && phases_to_set != 0) {
                LOCAL_LOG("Waking up vehicle at %s (%s).",
                          get_charger_name(i),
                          hosts[i]);

                trace("charger %d: waking up", i);
                charger.last_wakeup = now;
            }

            // If we could not allocate the charger its requested current the last time
            // but we can do this now (note that the requested current now does not have to be the same it was the last time!)
            // and we've just increased the allocation (as opposed to the requested current decreased),
            // we probably have more current available.
            // Ignore the phase currents for some time for a faster ramp up.
            if (!charger.last_alloc_fulfilled_reqd && current_to_set >= charger.requested_current && current_to_set > charger_alloc.allocated_current) {
                trace("charger %d: requested current fulfilled. Will use supported current for 1 min.", i);
                charger.ignore_phase_currents = now;
            }
            charger.last_alloc_fulfilled_reqd = current_to_set >= charger.requested_current;

            // The charger was just plugged in. If we've allocated phases to it for PLUG_IN_TIME, clear the timestamp
            // to reduce its priority.
            if (charger.last_plug_in != 0_us && phases_to_set > 0 && deadline_elapsed(charger.last_switch + cfg->plug_in_time)) {
                trace("charger %d: clearing last_plug_in after deadline elapsed", i);
                charger.last_plug_in = 0_us;
            }

            // The charger was just plugged in, we've allocated phases to it in the last iteration but no phases to it in this iteration.
            // As stage 3 (switching chargers off if phases are overloaded) sorts chargers by last_plug_in ascending if it is not 0,
            // the sort order is stable, so we've just hit a phase limit that was not as restrictive in the last iteration.
            // Clear the timestamp to make sure
            if (charger.last_plug_in != 0_us && charger_alloc.allocated_phases > 0 && phases_to_set == 0) {
                trace("charger %d: clearing last_plug_in; phases overloaded?", i);
                charger.last_plug_in = 0_us;
            }

            bool change = charger_alloc.allocated_current != current_to_set || charger_alloc.allocated_phases != phases_to_set;
            charger_alloc.allocated_current = current_to_set;
            charger_alloc.allocated_phases = phases_to_set;

            if (phases_to_set == 0) {
                charger.allocated_energy_this_rotation = 0;
            } else {
                auto amps = (float)current_to_set / 1000.0f * phases_to_set;
                auto amp_hours = amps * ((float)cfg->allocation_interval) / ((float)1_h);
                auto watt_hours = amp_hours * 230.0f;
                auto allocated_energy = watt_hours / 1000;
                charger.allocated_energy_this_rotation += allocated_energy;
                charger.allocated_energy += allocated_energy;
            }

            if (change) {
                LOCAL_LOG("Allocated %d mA @ %dp to %s (%s).",
                      current_to_set,
                      phases_to_set,
                      get_charger_name(i),
                      hosts[i]);

                print_local_log = true;
                if (charger_alloc.error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                    charger_alloc.last_sent_config = now;
            }
        }
    }

    if (print_local_log) {
        local_log = cfg->distribution_log.get();
        if (local_log) {
            size_t len = strlen(local_log);
            while (len > 0) {
                logger.printfln("%.*s", len, local_log);
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

#if MODULE_FIRMWARE_UPDATE_AVAILABLE() && MODULE_EM_V1_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
    firmware_update.vehicle_connected = vehicle_connected;
#else
    (void)vehicle_connected;
#endif

    trace("\n\n");

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

#if MODULE_FIRMWARE_UPDATE_AVAILABLE() && MODULE_EM_V1_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
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

    // Reset allocated energy if no car is connected
    if (v1->charger_state == 0) {
        target.allocated_energy = 0;
        target.allocated_energy_this_rotation = 0;
    }

    target.allowed_current = v1->allowed_charging_current;

    if (target.supported_current != v1->supported_current)
        trace("RECV %d: supported %u -> %umA", client_id, target.supported_current, v1->supported_current);
    target.supported_current = v1->supported_current;
    target.cp_disconnect_supported = CM_FEATURE_FLAGS_CP_DISCONNECT_IS_SET(v1->feature_flags);
    target.cp_disconnect_state = CM_STATE_FLAGS_CP_DISCONNECTED_IS_SET(v1->state_flags);

    // Wait for A -> non-A transitions, but ignore chargers that are already in a non-A state in their first packet.
    // Only set the timestamp if plug_in_time is != 0: This feature is deactivated if the time is set to 0.
    if (target.last_update != 0 && target.charger_state == 0 && v1->charger_state != 0 && cfg->plug_in_time != 0_us)
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
    if (abs((int)target.requested_current - (int)requested_current) > 1500) {
        trace("RECV %d: requested %u -> %u mA (lines %.3f %.3f %.3f)", client_id, target.requested_current, requested_current, v1->line_currents[0], v1->line_currents[1], v1->line_currents[2]);
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
        uint8_t new_phases = CM_STATE_V3_PHASES_CONNECTED_GET(v3->phases);
        uint8_t new_pss =  CM_STATE_V3_CAN_PHASE_SWITCH_IS_SET(v3->phases);
        if (new_phases != target.phases)
            trace("RECV %d: phases %u -> %u", client_id, target.phases, new_phases);
        if (new_pss != target.phase_switch_supported)
            trace("RECV %d: phase_switch_supported %u -> %u", client_id, target.phase_switch_supported, new_pss);
        target.phases = new_phases;
        target.phase_switch_supported = new_pss;
    } else {
        target.phases = 3;
        target.phase_switch_supported = false;
    }

    return true;
}
