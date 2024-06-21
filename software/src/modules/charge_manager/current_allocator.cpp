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

#define PRINT_COST(x) logger.tf_dbg("%s %d %d %d %d", #x, x[0], x[1], x[2], x[3])

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

bool cost_exceeds_limits(Cost cost, const CurrentLimits* limits, int stage)
{
    bool phases_exceeded = false;
    //bool phases_filtered_exceeded = false;
    for (size_t i = (size_t)GridPhase::L1; i <= (size_t)GridPhase::L3; ++i) {
        phases_exceeded |= limits->raw[i] < cost[i];
        //phases_filtered_exceeded |= limits->filtered[i] < cost[i];
    }

    bool pv_excess_exceeded = limits->raw.pv < cost.pv;
    //bool pv_excess_filtered_exceeded = limits->filtered.pv < cost.pv;

    switch(stage) {
        case 6:
        case 7:
            return phases_exceeded || pv_excess_exceeded;
        default:
            assert(false && "Unknown charge management stage!");
    }
}

void apply_cost(Cost cost, CurrentLimits* limits) {
    for (size_t i = (size_t)GridPhase::PV; i <= (size_t)GridPhase::L3; ++i) {
        limits->raw[i] -= cost[i];
        limits->filtered[i] -= cost[i];
    }
}

static bool is_active(uint8_t allocated_phases, const ChargerState *state) {
    return allocated_phases > 0 or state->is_charging;
    // TODO: implement other checks such as "Einschaltzeit > 0 < 1 min und State == B1 oder B2" here
    // Maybe also handle global hysteresis here? (ignore ALLOCATED_ENERGY_ROTATION_THRESHOLD if it is not elapsed)
}

#define GLOBAL_HYSTERESIS 1
#define ALLOCATION_TIMEOUT_S 1
#define ALLOCATED_ENERGY_ROTATION_THRESHOLD 5 /*kWh*/
static constexpr int32_t UNLIMITED = 1000 * 1000 * 10;

// Stage 1: Rotate chargers
// If there is any charger that wants to charge but doesn't have current allocated,
// temporarily disable chargers that are currently active (see is_active) but have been allocated
// ALLOCATED_ENERGY_ROTATION_THRESHOLD energy since being activated.
void stage_1(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    bool have_b1 = false;
    for (int i = 0; i < charger_count; ++i) {
        have_b1 |= charger_state[i].wants_to_charge;
    }

    for (int i = 0; i < charger_count; ++i) {
        const auto *state = &charger_state[i];
        if (is_active(phase_allocation[i], state) && (!have_b1 || state->allocated_energy_this_rotation < ALLOCATED_ENERGY_ROTATION_THRESHOLD))
            phase_allocation[i] = state->phases;
    }
}

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

    auto current_avail_for_3p = UNLIMITED;

    for (size_t i = 0; i < 4; ++i) {
        auto avail_on_phase = std::min(limits->raw[i], limits->filtered[i]) - wnd_min_1p[i];
        current_avail_for_3p = std::min(current_avail_for_3p, avail_on_phase);
    }

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases != 3)
            continue;

        wnd_max += Cost{0,
                        state->supported_current,
                        state->supported_current,
                        state->supported_current};

        // It is sufficient to check one phase here, wnd_max should have the same value on every phase because only three phase chargers are included yet
        if (wnd_max.l1 > current_avail_for_3p) {
            wnd_max = Cost{wnd_max.l1 + (current_avail_for_3p - wnd_max.l1) * alloc_phases,
                           current_avail_for_3p,
                           current_avail_for_3p,
                           current_avail_for_3p};
            //PRINT_COST(wnd_max);
            break;
        }

        wnd_max.pv += state->supported_current * alloc_phases;
        //PRINT_COST(wnd_max);
    }

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases == 3 || state->phase_rotation != PhaseRotation::Unknown)
            continue;

        // 1p unknown rotated chargers

        auto current = (int32_t)state->supported_current;
        for (size_t p = 1; p < 4; ++p) {
            auto avail_on_phase = std::min(limits->raw[p], limits->filtered[p]) - wnd_max[p];
            current = std::min(current, avail_on_phase);
        }

        wnd_max += Cost{current,
                        current,
                        current,
                        current};
        //PRINT_COST(wnd_max);
    }

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        const auto alloc_phases = phase_allocation[idx_array[i]];

        if (alloc_phases == 3 || state->phase_rotation == PhaseRotation::Unknown)
            continue;

        const auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
        const auto l = std::min(limits->raw[phase], limits->filtered[phase]);
        const auto current = std::min(l - wnd_max[phase], (int)state->supported_current);

        wnd_max[phase] += current;
        wnd_max.pv += current;
        //PRINT_COST(wnd_max);
    }

    ca_state->control_window_min = wnd_min;
    ca_state->control_window_max = wnd_max;
}

// Stage 2: Shut down chargers if over limit
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
void stage_2(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    calculate_window(idx_array, current_allocation, phase_allocation, limits, charger_state, cfg->charger_count, cfg, ca_state);

    Cost wnd_min = ca_state->control_window_min;
    auto min_1p = cfg->minimum_current_1p;
    auto min_3p = cfg->minimum_current_3p;

    int matched = 0;

    filter(allocated_phases > 0);

    // Reverse sort. Charger with most energy allocated first.
    // TODO: Don't sort descending, loop backwards instead
    // so that stage 4 and 5 don't have to sort.
    sort(0,
        left.state->allocated_energy >= right.state->allocated_energy
    );

    bool any_charger_shut_down = false;

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

    // TODO: charger min active time
    for (int i = 0; i < matched; ++i) {
        if (limits->raw.pv >= wnd_min.pv || limits->filtered.pv >= wnd_min.pv || !ca_state->global_hysteresis_elapsed)
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
// - wnd_max is less than the raw and filtered limit of one phase and activating the charger moves wnd_max closer to the limit (i.e. its cost on this phase is positive)
// - the enable cost does not exceed the raw or filtered limits on any phase or PV

static bool can_activate(const Cost new_cost, const Cost new_enable_cost, const Cost wnd_min, const Cost wnd_max, const CurrentLimits *limits, const CurrentAllocatorConfig *cfg, bool is_unknown_rotated_1p_3p_switch=false) {
    bool improves_any_phase = false;

    for (size_t p = 1; p < 4; ++p) {
        improves_any_phase |= new_cost[p] > 0 && wnd_max[p] < limits->raw[p] && wnd_max[p] < limits->filtered[p];
    }
    if (!improves_any_phase) {
        bool enable = is_unknown_rotated_1p_3p_switch && new_cost.pv >= 0 && wnd_max.pv < limits->raw.pv && wnd_max.pv < limits->filtered.pv;
        if (!enable)
            return false;
    }

    for (size_t p = 0; p < 4; ++p) {
        if (new_cost[p] <= 0)
            continue;
        auto required = wnd_min[p] * cfg->enable_current_factor + new_enable_cost[p];

        if (limits->raw[p] < required || limits->filtered[p] < required)
            return false;
    }

    return true;
}

// Stage 3: Activate chargers if current is available
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
void stage_3(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    Cost wnd_min = ca_state->control_window_min;
    Cost wnd_max = ca_state->control_window_max;

    auto min_1p = cfg->minimum_current_1p;
    auto min_3p = cfg->minimum_current_3p;

    int ena_1p = min_1p * cfg->enable_current_factor;
    int ena_3p = min_3p * cfg->enable_current_factor;

    int matched = 0;

    filter(allocated_phases == 0 && (state->wants_to_charge || is_active(allocated_phases, state)));

    sort(0,
        left.state->allocated_energy < right.state->allocated_energy
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        bool activate_3p = state->phases == 3 && !state->phase_switch_supported;

        Cost new_cost;
        Cost new_enable_cost;

        if (activate_3p) {
            new_cost = Cost{3 * min_3p, min_3p, min_3p, min_3p};
            new_enable_cost = Cost{3 * ena_3p, ena_3p, ena_3p, ena_3p};
        } else if (state->phase_rotation == PhaseRotation::Unknown) {
            // Try to enable switchable unknown rotated chargers with three phases first.
            if (state->phase_switch_supported) {
                new_cost = Cost{3 * min_3p, min_3p, min_3p, min_3p};
                new_enable_cost = Cost{3 * ena_3p, ena_3p, ena_3p, ena_3p};
                activate_3p = true;
            }
            if (!state->phase_switch_supported || !can_activate(new_cost, new_enable_cost, wnd_min, wnd_max, limits, cfg)) {
                new_cost = Cost{min_1p, min_1p, min_1p, min_1p};
                new_enable_cost = Cost{ena_1p, ena_1p, ena_1p, ena_1p};
                activate_3p = false;
            }
        } else {
            auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);

            new_cost.pv += min_1p;
            new_cost[phase] += min_1p;

            new_enable_cost.pv += ena_1p;
            new_enable_cost[phase] += ena_1p;
        }

        if (!can_activate(new_cost, new_enable_cost, wnd_min, wnd_max, limits, cfg)) {
            /*logger.tf_dbg("skip %d", idx_array[i]);
            PRINT_COST(new_cost);
            PRINT_COST(new_enable_cost);
            PRINT_COST(wnd_min);
            PRINT_COST(wnd_max);
            PRINT_COST(limits->raw);
            PRINT_COST(limits->filtered);*/
            continue;
        }

        phase_allocation[idx_array[i]] = activate_3p ? 3 : 1;
        calculate_window(idx_array, current_allocation, phase_allocation, limits, charger_state, charger_count, cfg, ca_state);
        wnd_min = ca_state->control_window_min;
        wnd_max = ca_state->control_window_max;
    }
}

// Stage 4: Switch 1p -> 3p if possible
// - This is conceptionally similar to stage 3 except that chargers already have one phase allocated
// - If there are still phases with a limit greater than the window maximum,
//   switch chargers that have 1 phase allocated to 3 phases if possible
// - Only chargers that _currently_ support the phase switch are considered
// - Sort by allocated energy ascending as in stage 3
// - Enable conditions are similar to stage 3:
//   The enable cost is the cost to enable a charger with three phases minus the cost (that already was subtracted in stage 3) to enable it with one phase.
void stage_4(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    Cost wnd_min = ca_state->control_window_min;
    Cost wnd_max = ca_state->control_window_max;

    auto min_1p = cfg->minimum_current_1p;
    auto min_3p = cfg->minimum_current_3p;

    int ena_1p = min_1p * cfg->enable_current_factor;
    int ena_3p = min_3p * cfg->enable_current_factor;

    int matched = 0;

    filter(allocated_phases == 1 && state->phase_switch_supported);

    sort(0,
        left.state->allocated_energy < right.state->allocated_energy
    );

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];

        Cost new_cost = Cost{3 * min_3p - min_1p, min_3p, min_3p, min_3p};
        Cost new_enable_cost = Cost{3 * ena_3p - ena_1p, ena_3p, ena_3p, ena_3p};

        if (state->phase_rotation == PhaseRotation::Unknown) {
            for (size_t p = 1; p < 4; ++p) {
                new_cost[p] -= min_1p;
                new_enable_cost[p] -= min_1p;
            }
        } else {
            auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
            // P1 is already active
            new_cost[phase] -= min_1p;
            new_enable_cost[phase] -= ena_1p;
        }

        if (!can_activate(new_cost, new_enable_cost, wnd_min, wnd_max, limits, cfg))
            continue;

        phase_allocation[idx_array[i]] = 3;
        calculate_window(idx_array, current_allocation, phase_allocation, limits, charger_state, charger_count, cfg, ca_state);
        wnd_min = ca_state->control_window_min;
        wnd_max = ca_state->control_window_max;
    }
}

// Stage 5: Allocate minimum current to chargers with at least one allocated phase
void stage_5(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
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
int32_t current_capacity(const CurrentLimits *limits, const ChargerState *state, int32_t allocated_current, uint8_t allocated_phases) {
    if (allocated_phases == 3 || state->phase_rotation == PhaseRotation::Unknown) {
        return std::min({std::max(state->supported_current - allocated_current, 0), limits->raw.l1, limits->raw.l2, limits->raw.l3});
    }

    auto capacity = std::max(state->supported_current - allocated_current, 0);
    for (size_t i = (size_t)ChargerPhase::P1; i < (size_t)ChargerPhase::P1 + allocated_phases; ++i) {
        auto phase = get_phase(state->phase_rotation, (ChargerPhase)((size_t)ChargerPhase::P1 + i));
        capacity = std::min(capacity, limits->raw[phase]);
    }

    return allocated_phases * capacity;
}

// Stage 6: Allocate fair current to chargers with at least one allocated phase
// - All those chargers already have their minimum current allocated
void stage_6(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
    int matched = 0;

    filter(allocated_current > 0);

    if (matched == 0)
        return;

    // No need to sort here: Each charger gets the same current

    Cost active_on_phase{matched, 0, 0, 0};
    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];
        if (allocated_phases == 3 or state->phase_rotation == PhaseRotation::Unknown) {
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

    auto fair_current = UNLIMITED;
    for (size_t p = 1; p < 4; ++p) {
        if (active_on_phase[p] == 0)
            continue;
        fair_current = std::min(fair_current, limits->raw[p] / active_on_phase[p]);
    }

    for (int i = 0; i < matched; ++i) {
        const auto *state = &charger_state[idx_array[i]];

        auto allocated_current = current_allocation[idx_array[i]];
        auto allocated_phases = phase_allocation[idx_array[i]];

        auto current = std::min(fair_current, current_capacity(limits, state, allocated_current, allocated_phases));
        current += allocated_current;

        auto cost = get_cost(current, (ChargerPhase)allocated_phases, state->phase_rotation, allocated_current, (ChargerPhase)allocated_phases);

        // TODO: This should never happen?
        if (cost_exceeds_limits(cost, limits, 6))
            continue;

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;
    }
}

// Stage 7: Allocate left-over current.
// - Group by phases. Makes sure that we allocate current to three phase chargers first.
//   If there is current left after this, we've probably hit one of the phase limits.
//   One phase chargers on other phases will take the rest if possible.
// - Sort by current_capacity ascending. This makes sure that one pass is enough to allocate the possible maximum.
void stage_7(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state) {
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

        auto cost = get_cost(current, (ChargerPhase)allocated_phases, state->phase_rotation, allocated_current, (ChargerPhase)allocated_phases);

        // TODO: This should never happen?
        if (cost_exceeds_limits(cost, limits, 7))
            continue;

        apply_cost(cost, limits);
        current_allocation[idx_array[i]] = current;
    }
}

/*

Accumulate (allocation cycle time * allocated current * allocated phases * 230V) per charger -> allocated charge

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

#include <stdio.h>

static void print_alloc(int stage, CurrentLimits *limits, int32_t *current_array, uint8_t *phases_array, size_t charger_count) {
    printf("%d LIMITS (%6.3f,%6.3f,%6.3f,%6.3f)/(%6.3f,%6.3f,%6.3f,%6.3f) ALLOC",
           stage,
           limits->raw[0] / 1000.0f,
           limits->raw[1] / 1000.0f,
           limits->raw[2] / 1000.0f,
           limits->raw[3] / 1000.0f,

           limits->filtered[0] / 1000.0f,
           limits->filtered[1] / 1000.0f,
           limits->filtered[2] / 1000.0f,
           limits->filtered[3] / 1000.0f);
    for(size_t i = 0; i < charger_count; ++i) {
        printf(" %6.3f@%dp", current_array[i] / 1000.0f, phases_array[i]);
    }
    printf("\n");
}

int allocate_current(
    const CurrentAllocatorConfig *cfg,
    const bool seen_all_chargers,
    CurrentLimits *limits,
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

    if (!seen_all_chargers) {
        limits->raw = Cost{0, 0, 0, 0};
        limits->filtered = Cost{0, 0, 0, 0};
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
            limits->raw = Cost{0, 0, 0, 0};
            limits->filtered = Cost{0, 0, 0, 0};
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

    //print_alloc(0, limits, current_array, phases_array, cfg->charger_count);
    stage_1(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(1, limits, current_array, phases_array, cfg->charger_count);
    stage_2(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(2, limits, current_array, phases_array, cfg->charger_count);
    stage_3(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(3, limits, current_array, phases_array, cfg->charger_count);
    stage_4(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(4, limits, current_array, phases_array, cfg->charger_count);
    stage_5(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(5, limits, current_array, phases_array, cfg->charger_count);
    stage_6(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(6, limits, current_array, phases_array, cfg->charger_count);
    stage_7(idx_array, current_array, phases_array, limits, charger_state, cfg->charger_count, cfg, ca_state);
    //print_alloc(7, limits, current_array, phases_array, cfg->charger_count);

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

            bool change = charger_alloc.allocated_current != current_to_set || charger_alloc.allocated_phases != phases_to_set;
            charger_alloc.allocated_current = current_to_set;
            charger_alloc.allocated_phases = phases_to_set;
            if (change) {
                print_local_log = true;
                if (charger_alloc.error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                    charger_alloc.last_sent_config = millis();
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

    if (v3 != nullptr) {
        target.phases = CM_STATE_V3_PHASES_CONNECTED_GET(v3->phases);
        target.phase_switch_supported = CM_FEATURE_FLAGS_PHASE_SWITCH_IS_SET(v1->feature_flags) && CM_STATE_V3_CAN_PHASE_SWITCH_IS_SET(v3->phases);
    }

    return true;
}
