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

#define TRACE_LOG_PREFIX "charge_manager"

#include <assert.h>
#include <string.h> // For strlen
#include <math.h> // For isnan
#include <algorithm>
#include <stdio.h> // For snprintf

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "modules/cm_networking/cm_networking_defs.h"
#include "current_allocator_private.h"
#include "string_builder.h"

//#include "gcc_warnings.h"

#define LOCAL_LOG_FULL(indent, fmt, ...) \
    do { \
        if(local_log) { \
            local_log += snprintf_u(local_log, cfg->distribution_log_len - (local_log - cfg->distribution_log.get()), indent fmt "%c" __VA_OPT__(,) __VA_ARGS__, '\0'); \
        } \
    } while (0)

#define LOCAL_LOG(fmt, ...) LOCAL_LOG_FULL("    ", fmt __VA_OPT__(,) __VA_ARGS__)

static constexpr micros_t TIMEOUT = 32_s;

#define PRINT_COST(x) logger.printfln("%s %d %d %d %d", #x, x[0], x[1], x[2], x[3])

#define trace(fmt, ...) logger.tracefln_plain(charge_manager.trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__)

static constexpr int UNLIMITED = 10 * 1000 * 1000; /* mA */
static constexpr micros_t KEEP_ACTIVE_AFTER_PHASE_SWITCH_TIME = 1_min;

static void print_alloc(int stage, const StageContext &sc) {
    char buf[768] = {};
    logger.printfln("%d LIMITS raw(%6.3f,%6.3f,%6.3f,%6.3f) min(%6.3f,%6.3f,%6.3f,%6.3f) spread(%6.3f,%6.3f,%6.3f,%6.3f) max_pv %6.3f",
           stage,
           sc.limits->raw[0] / 1000.0f,
           sc.limits->raw[1] / 1000.0f,
           sc.limits->raw[2] / 1000.0f,
           sc.limits->raw[3] / 1000.0f,

           sc.limits->min[0] / 1000.0f,
           sc.limits->min[1] / 1000.0f,
           sc.limits->min[2] / 1000.0f,
           sc.limits->min[3] / 1000.0f,

           sc.limits->spread[0] / 1000.0f,
           sc.limits->spread[1] / 1000.0f,
           sc.limits->spread[2] / 1000.0f,
           sc.limits->spread[3] / 1000.0f,

           sc.limits->max_pv / 1000.0f);

    char *ptr = buf;
    ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "  ALLOC");
    for(size_t i = 0; i < sc.charger_count; ++i) {
        if (sc.phase_allocation[i] == 0 && sc.current_allocation[i] == 0 && stage != 0)
            ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "|        %zu        |", i);
        else
            ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), " %6.3f@%dp", sc.current_allocation[i] / 1000.0f, sc.phase_allocation[i]);
        if (i % 8 == 7 && i != sc.charger_count - 1) {
            logger.printfln("%s", buf);
            ptr = buf;
            ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "   ");
        }
    }
    logger.printfln("%s", buf);
}

static void trace_alloc(int stage, const StageContext &sc) {
#if defined(BOARD_HAS_PSRAM)
    char buf[768] = {};
    trace("%d: raw(%d %d %d %d) min(%d %d %d %d) spread(%d %d %d %d) max_pv %d",
           stage,
           sc.limits->raw[0],
           sc.limits->raw[1],
           sc.limits->raw[2],
           sc.limits->raw[3],

           sc.limits->min[0],
           sc.limits->min[1],
           sc.limits->min[2],
           sc.limits->min[3],

           sc.limits->spread[0],
           sc.limits->spread[1],
           sc.limits->spread[2],
           sc.limits->spread[3],

           sc.limits->max_pv);

    char *ptr = buf;
    ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "%d: ", stage);
    for(size_t i = 0; i < sc.charger_count; ++i) {
        if (stage == 0) {
            ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "[%2zu %dp %11s ]", i, sc.phase_allocation[i], ChargeMode::Strings[sc.charger_state[i].charge_mode]);
        } else {
            if (sc.phase_allocation[i] == 0 && sc.current_allocation[i] == 0)
                ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "[        %2zu        ]", i);
            else
                ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "[%2zu %5d@%dp       ]", i, sc.current_allocation[i], sc.phase_allocation[i]);
        }

        if (i % 8 == 7 && i != sc.charger_count - 1) {
            trace("%s", buf);
            ptr = buf;
            ptr += snprintf(ptr, sizeof(buf) - (ptr - buf), "   ");
        }
    }
    trace("%s", buf);
#endif
}

static void trace_sort_fn(int stage, int matched, const StageContext &sc) {
#if defined(BOARD_HAS_PSRAM)
    char buf[200];
    memset(buf, 0, sizeof(buf));
    char *ptr = buf;
    ptr += snprintf(ptr, ARRAY_SIZE(buf) - (ptr - buf), "%d: filtered %d to %d%s", stage, sc.charger_count, matched, matched > 0 ? ", sorted to " : ".");
    if (matched > 0) {
        for(int i = 0; i < matched; ++i)
            ptr += snprintf(ptr, ARRAY_SIZE(buf) - (ptr - buf), "%d %s", sc.idx_array[i], (matched < sc.charger_count) && (i == matched - 1) ? "|..." : "");
    }
    trace("%s", buf);
#endif
}

#define trace_sort(x) trace_sort_fn(x, matched, sc)

// Sorts the indices of chargers that match the filter to the front of idx_array and returns the number of matches.
int filter_chargers_impl(filter_fn filter_, StageContext &sc) {
    int matches = 0;
    for(int i = 0; i < sc.charger_count; ++i) {
        if (!filter_({sc.current_allocation[sc.idx_array[i]], sc.phase_allocation[sc.idx_array[i]], sc.cfg, &sc.charger_state[sc.idx_array[i]], sc.charge_mode_filter}))
            continue;

        int tmp = sc.idx_array[matches];
        sc.idx_array[matches] = sc.idx_array[i];
        sc.idx_array[i] = tmp;
        ++matches;
    }
    return matches;
}

// Sorts the indices of chargers by first grouping them with the group function and then comparing in groups with the sort function.
void sort_chargers_impl(group_fn group, compare_fn compare, StageContext &sc, int matched) {
    int groups[MAX_CONTROLLED_CHARGERS] = {};

    for(int i = 0; i < matched; ++i)
        groups[sc.idx_array[i]] = group({sc.current_allocation[sc.idx_array[i]], sc.phase_allocation[sc.idx_array[i]], &sc.charger_state[sc.idx_array[i]], sc.cfg, &sc.charger_allocation_state[sc.idx_array[i]]});

    std::stable_sort(
        sc.idx_array,
        sc.idx_array + matched,
        [&groups, &compare, sc] (int left, int right) {
            if (groups[left] != groups[right])
                return groups[left] < groups[right];

            return compare({
                            {sc.current_allocation[left], sc.phase_allocation[left], &sc.charger_state[left]},
                            {sc.current_allocation[right], sc.phase_allocation[right], &sc.charger_state[right]},
                           sc.limits,
                           sc.cfg});
        }
    );
}

GridPhase get_phase(PhaseRotation rot, ChargerPhase phase) {
    return (GridPhase)(((int)rot >> (6 - 2 * (int)phase)) & 0x3);
}

static inline Cost get_phase_factors(uint8_t allocated_phases, PhaseRotation rotation) {
    if (allocated_phases == 3) {
        return Cost{3, 1, 1, 1};
    }
    else if (rotation == PhaseRotation::Unknown) {
        // unknown rot 1p
        return Cost{1, 1, 1, 1};
    }

    // known rot 1p
    Cost result {1, 0, 0, 0};
    result[get_phase(rotation, ChargerPhase::P1)] = 1;
    return result;
}

// Returns how much current would be used
// if the allocation of a charger was changed from
// [allocated_current]@[allocated_phases]
// to [current_to_allocate]@{phases_to_allocate].
Cost get_cost(int current_to_allocate,
              ChargerPhase phases_to_allocate,
              PhaseRotation rot,
              int allocated_current,
              ChargerPhase allocated_phases)
{
    auto allocated = allocated_current * get_phase_factors((uint8_t)allocated_phases, rot);
    auto to_allocate = current_to_allocate * get_phase_factors((uint8_t)phases_to_allocate, rot);

    return to_allocate - allocated;
}

// Checks stage-specific limits.
bool cost_exceeds_limits(const Cost &cost, const CurrentLimits* limits, int stage, bool observe_pv_limit, uint32_t guaranteed_pv_current)
{
    bool phases_exceeded = false;
    for (size_t i = (size_t)GridPhase::L1; i <= (size_t)GridPhase::L3; ++i) {
        phases_exceeded |= cost[i] > 0 && limits->raw[i] < cost[i];
    }

    bool pv_excess_exceeded = observe_pv_limit && cost.pv > 0 && limits->raw.pv < cost.pv && cost.pv > guaranteed_pv_current;

    switch(stage) {
        case 6:
            // Intentionally ignore the PV limit in stage 6:
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

            bool pv_excess_min_exceeded =  observe_pv_limit && cost.pv > 0 && limits->min.pv < cost.pv && cost.pv > guaranteed_pv_current;
            return phases_exceeded || pv_excess_exceeded || phases_min_exceeded || pv_excess_min_exceeded;
        }
        default:
            assert(false && "Unknown charge management stage!");
    }
}

void apply_cost(const Cost &cost, CurrentLimits* limits) {
    limits->raw -= cost;
    limits->min -= cost;
    limits->spread -= cost;

    limits->max_pv -= cost.pv;
}

// A charger is active if
// - we have allocated current to it or will in the future (thus checking the allocated phases)
// - it wants to charge (i.e. a vehicle is plugged in and no other slot blocks) or is charging (i.e. is in state C) or the car blocks and we've just activated or phase-switched the charger
// - we are not currently attempting to wake up a "full" vehicle:
//   We don't consider this as active because the wake-up stage will allocate to this charger anyway, but this has the lowest priority.
//   If this was considered active, a wake-up could steal current from the following states
static bool is_active(uint8_t allocated_phases, const ChargerState *state) {
    return allocated_phases > 0 && (state->wants_to_charge || state->is_charging) && state->last_wakeup == 0_us;
}

static int get_highest_charge_mode_bit(const ChargerState *state) {
    for (uint8_t i = ChargeMode::_max; i >= ChargeMode::_min; i = ((uint8_t)i >> 1)) {
        switch((ChargeMode::Type)(state->charge_mode & i)) {
            case ChargeMode::Fast:
                return (int)ChargeMode::Fast;
            case ChargeMode::Eco:
                if (state->eco_fast)
                    return (int)ChargeMode::Eco;
                break;
            case ChargeMode::Min:
                return (int)ChargeMode::Min;
            case ChargeMode::PV:
                return (int)ChargeMode::PV;
        }
    }
    return 0;
}

// Stage 1: Rotate chargers
// If there is any charger that wants to charge but doesn't have current allocated,
// temporarily disable chargers that are currently active (see is_active) but have been allocated
// ALLOCATED_ENERGY_ROTATION_THRESHOLD energy since being activated.
//
// Also deallocate phases if the charger is not active anymore.
// We want to dealloc a charger if the connected vehicle is currently being woken up.
// If there is current left over at the end, we will reallocate some to this charger.
static void stage_1(StageContext &sc) {
    // Only rotate if there is at least one charger that does want to charge but doesn't have current allocated.
    // This charger also has to be plugged in for at least one allocation iteration, to make sure
    // we don't rotate chargers out immediately if a new charger is plugged in. Maybe there is
    // enough current available to activate both of them.
    //
    // First get the phase(s) that we have to make some room for the B1 charger(s) on.
    // We don't have to handle the PV "phase" because an active charger will draw the PV current on some of the phases.
    bool have_b1 = false;
    Cost b1_on_phase= {0, 0, 0, 0};
    int highest_charge_mode_bit_seen = 0;

    for (int i = 0; i < sc.charger_count; ++i) {
        const auto *state = &sc.charger_state[i];
        if (state->off)
            continue;

        if (state->wants_to_charge || state->is_charging) {
            auto highest_bit = get_highest_charge_mode_bit(state);
            // Don't rotate PV chargers out for Min chargers. Both groups are allocated at the same time.
            if (highest_bit == ChargeMode::Min)
                highest_bit = ChargeMode::PV;
            highest_charge_mode_bit_seen = std::max(highest_charge_mode_bit_seen, highest_bit);
        }

        // Don't trigger a rotation if the new charger was plugged in less than one alloc ago.
        // Using last_plug_in is fine here, we don't need the more complicated logic of just_plugged_in.
        bool is_b1 = state->wants_to_charge && sc.phase_allocation[i] == 0 && deadline_elapsed(state->last_plug_in + sc.cfg->allocation_interval);

        if (is_b1) {
            have_b1 = true;
            if (state->phases == 3 || state->phase_rotation == PhaseRotation::Unknown) {
                // We only care about the phases that are blocked, not about how many chargers are waiting.
                b1_on_phase += Cost{0, 1, 1, 1};
            } else {
                // Not 2p safe!
                auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
                b1_on_phase[phase] = 1;
            }
        }
    }

    // Only rotate once per cfg->rotation_interval.
    // Setting the rotation interval to 0 seconds disables the rotation feature completely.
    bool rotation_allowed = sc.cfg->rotation_interval > 0_s && deadline_elapsed(sc.ca_state->next_rotation);

    trace("1: have B1 %d; rotation allowed %d",have_b1, rotation_allowed);

    if (rotation_allowed) {
        seconds_t interval = sc.cfg->rotation_interval;
        sc.ca_state->next_rotation = now_us() + interval;

        // Align rotations to wall-clock time if it is known.
        timeval tv;
        if (rtc.clock_synced(&tv)) {
            seconds_t now = seconds_t{tv.tv_sec};
            seconds_t midnight = seconds_t{get_localtime_midnight_in_utc(tv.tv_sec)};

            auto offset_after_interval = (now - midnight) % interval;
            sc.ca_state->next_rotation -= offset_after_interval;

            if (offset_after_interval >= (2_us * sc.cfg->allocation_interval))
                sc.ca_state->next_rotation += interval;
        }
    }

    // Shut down a charger if it should be rotated or if it reports that it is not active anymore.
    // Note that a charger that was activated in stage 9 (to wake up a full vehicle)
    // it will always be deactivated here to make sure we only wake up vehicles if there is current left over.
    for (int i = 0; i < sc.charger_count; ++i) {
        if (sc.phase_allocation[i] == 0)
            continue;

        const auto *state = &sc.charger_state[i];

        // We need both the clock-aligned rotation interval and the minimum active time to be elapsed before rotating a charger:
        // A charger that was just plugged in may not be rotated.
        bool rotate_for_waiting_b1  = rotation_allowed && sc.ca_state->global_hysteresis_elapsed && have_b1;
        bool rotate_for_higher_prio = rotation_allowed && sc.ca_state->global_hysteresis_elapsed && get_highest_charge_mode_bit(state) < highest_charge_mode_bit_seen;

        // 3p and 1p unknown rotated chargers are considered active on any phase.
        // They can be shut down if there is a charger waiting on any phase.
        // 1p known rotated chargers are only active on a specific phase.
        // Don't shut a 1p known rotated charger down if it is active on a phase where there is no charger waiting.
        if (rotate_for_waiting_b1 && (state->phases != 3 && state->phase_rotation != PhaseRotation::Unknown)) {
            // Not 2p safe!
            auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
            rotate_for_waiting_b1 &= b1_on_phase[phase] == 1;
        }

        bool keep_active = is_active(sc.phase_allocation[i], state) && !rotate_for_waiting_b1 && !rotate_for_higher_prio && !state->off;

        trace("1: %d: rot_b1 %c rot_prio %c keep_active %c can p-switch %c",
                        i,
                        rotate_for_waiting_b1 ? '1' : '0',
                        rotate_for_higher_prio ? '1' : '0',
                        keep_active ? '1' : '0',
                        state->phase_switch_supported ? '1' : '0');

        if (!keep_active) {
            sc.phase_allocation[i] = 0;
            continue;
        }

        // If a charger does not support phase switching (anymore),
        // the connected number of phases wins against the allocated number of phases.
        if (!state->phase_switch_supported) {
            sc.phase_allocation[i] = state->phases;
            continue;
        }
    }
}

// Only regard a charger as just plugged in if it wants to charge.
// If another current limit blocks this charger (for example the user/NFC limit)
// there is not need to allocate current to it.
static bool was_just_plugged_in(const ChargerState *state) {
    return state->just_plugged_in_timestamp != 0_us && state->wants_to_charge;
}

// Stage 2: Immediately activate chargers were a vehicle was just plugged in.
// Do this before calculating the initial control window and ignore limits.
// If we exceed limits by activating those chargers, the next stage will
// shut down other chargers if necessary.
// Both this stage and stage 3 sort by the just_plugged_in_timestamp timestamp,
// so if we have more chargers with a timestamp than can be activated at once,
// we will activate some, give them current for some time and then activate the next group.
// Because a charger's just_plugged_in_timestamp timestamp is only cleared once it has been charging for the plug_in_time,
// we won't toggle the contactors too fast.
// This feature is completely deactivated if cfg->plug_in_time is set to 0.
static void stage_2(StageContext &sc) {
    int matched = filter_chargers(was_just_plugged_in(ctx.state) && (ctx.state->charge_mode & ctx.charge_mode_filter) != 0 && !ctx.state->off);

    // Charger that is plugged in for the longest time first.
    sort_chargers(0,
        ctx.left.state->just_plugged_in_timestamp < ctx.right.state->just_plugged_in_timestamp
    );

    trace_sort(2);

    for (int i = 0; i < matched; ++i) {
        const auto *state = &sc.charger_state[sc.idx_array[i]];

        bool is_fixed_3p = state->phases == 3 && !state->phase_switch_supported;
        bool activate_3p = is_fixed_3p;

        // Don't override existing allocation.
        // This charger could be switched to 3p.
        if (sc.phase_allocation[sc.idx_array[i]] == 0)
            sc.phase_allocation[sc.idx_array[i]] = activate_3p ? 3 : 1;

        trace("2: %d: plugged in. alloc %dp", sc.idx_array[i], sc.phase_allocation[sc.idx_array[i]]);
    }
}

// Use the supported current in case the last allocation was able to fulfill the requested current.
// In that case we want a fast ramp-up until we know the new limit of the charger (or don't have any current left)
static int get_requested_current(const ChargerState *state, const CurrentAllocatorConfig *cfg, uint8_t allocated_phases) {
    int reqd = state->requested_current;

    if (!deadline_elapsed(state->use_supported_current + seconds_t{cfg->requested_current_threshold})) {
        reqd = state->supported_current;
    }

    if (get_highest_charge_mode_bit(state) == ChargeMode::Min && ((state->charge_mode & ChargeMode::PV) == 0)) {
        auto guaranteed_current = state->guaranteed_pv_current / allocated_phases;
        reqd = std::min(guaranteed_current, reqd);
    }
    return reqd;
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
static void calculate_window(bool trace_short, StageContext &sc) {
    if (!trace_short) trace("     Calc Wnd");
    auto min_1p = sc.cfg->minimum_current_1p;
    auto min_3p = sc.cfg->minimum_current_3p;

    Cost wnd_min{0, 0, 0, 0};
    Cost wnd_min_1p{0, 0, 0, 0};
    Cost wnd_max{0, 0, 0, 0};

    // Work on copy of idx_array so that this function can be called in stages without destroying their sort order.
    int idx_array[MAX_CONTROLLED_CHARGERS];
    memcpy(idx_array, sc.idx_array, sizeof(idx_array));

    int *old_idx_array = sc.idx_array;
    sc.idx_array = idx_array;

    int matched = filter_chargers(ctx.allocated_phases > 0);
    sc.idx_array = old_idx_array;

    // Calculate minimum window
    for (int i = 0; i < matched; ++i) {
        // Never remove current that was allocated in this iteration.
        if (sc.current_allocation[idx_array[i]] > 0)
            continue;

        const auto *state = &sc.charger_state[idx_array[i]];
        const auto alloc_phases = sc.phase_allocation[idx_array[i]];

        const auto factors = get_phase_factors(alloc_phases, state->phase_rotation);

        if (alloc_phases == 3) {
            wnd_min += min_3p * factors;
        }
        else {
            wnd_min += min_1p * factors;
            wnd_min_1p += min_1p * factors.without_pv();
        }

        if (!trace_short) trace("       %d wnd_min (%d %d %d %d)", idx_array[i], wnd_min.pv, wnd_min.l1, wnd_min.l2, wnd_min.l3);
    }

    // Calculate left over current for 3p chargers after allocating the 1p minimum to 1p chargers.
    // The PV phase is not blocked by other phases when allocating current to a 3p charger, so it can be excluded here.
    auto current_avail_for_3p = (sc.limits->raw - wnd_min_1p).min_phase();

    if (!trace_short) trace("       current_avail_for_3p %d", current_avail_for_3p);

    // Add maximum window of 3p chargers.
    for (int i = 0; i < matched; ++i) {
        const auto *state = &sc.charger_state[idx_array[i]];
        const auto alloc_phases = sc.phase_allocation[idx_array[i]];

        if (alloc_phases != 3)
            continue;

        auto factors = get_phase_factors(alloc_phases, state->phase_rotation);
        auto requested = factors * get_requested_current(state, sc.cfg, alloc_phases);
        auto already_allocated = factors * sc.current_allocation[idx_array[i]];

        wnd_max += requested - already_allocated;

        // It is sufficient to check one phase here, wnd_max should have the same value on every phase because only three phase chargers are included yet
        if (wnd_max.l1 > current_avail_for_3p) {
            wnd_max = factors * current_avail_for_3p;

            if (!trace_short) trace("       %d (3p) wnd_max (%d %d %d %d).l1 == current_avail_for_3p.", idx_array[i], wnd_max.pv, wnd_max.l1, wnd_max.l2, wnd_max.l3);

            break;
        }

        if (!trace_short) trace("       %d (3p) wnd_max (%d %d %d %d)", idx_array[i], wnd_max.pv, wnd_max.l1, wnd_max.l2, wnd_max.l3);
    }

    // Add maximum window of 1p chargers with unknown rotation.
    for (int i = 0; i < matched; ++i) {
        const auto *state = &sc.charger_state[idx_array[i]];
        const auto alloc_phases = sc.phase_allocation[idx_array[i]];

        if (alloc_phases == 3 || state->phase_rotation != PhaseRotation::Unknown)
            continue;

        auto factors = get_phase_factors(alloc_phases, state->phase_rotation);

        auto already_allocated = sc.current_allocation[idx_array[i]];
        auto current = get_requested_current(state, sc.cfg, alloc_phases) - already_allocated;

        auto available_current = (sc.limits->raw - wnd_max).min_phase();
        current = std::min(available_current, current);

        wnd_max += current * factors;

        if (!trace_short) trace("       %d (1p unknown rot) wnd_max (%d %d %d %d)", idx_array[i], wnd_max.pv, wnd_max.l1, wnd_max.l2, wnd_max.l3);
    }

    // Add maximum window of 1p chargers with known rotation.
    for (int i = 0; i < matched; ++i) {
        const auto *state = &sc.charger_state[idx_array[i]];
        const auto alloc_phases = sc.phase_allocation[idx_array[i]];

        if (alloc_phases == 3 || state->phase_rotation == PhaseRotation::Unknown)
            continue;

        auto already_allocated = sc.current_allocation[idx_array[i]];
        auto current = get_requested_current(state, sc.cfg, alloc_phases) - already_allocated;

        const auto phase = get_phase(state->phase_rotation, ChargerPhase::P1);
        current = std::min(sc.limits->raw[phase] - wnd_max[phase], current);

        wnd_max[phase] += current;
        wnd_max.pv += current;

        if (!trace_short) trace("       %d (1p known rot) wnd_max (%d %d %d %d)", idx_array[i], wnd_max.pv, wnd_max.l1, wnd_max.l2, wnd_max.l3);
    }

    sc.ca_state->control_window_min = wnd_min;
    sc.ca_state->control_window_max = wnd_max;

    trace("     Wnd (%d %d %d %d)->(%d %d %d %d)", wnd_min.pv, wnd_min.l1, wnd_min.l2, wnd_min.l3, wnd_max.pv, wnd_max.l1, wnd_max.l2, wnd_max.l3);

}

static inline Cost get_minimum_cost(uint8_t allocated_phases, PhaseRotation rotation, const CurrentAllocatorConfig *cfg) {
    return (allocated_phases == 3 ? cfg->minimum_current_3p : cfg->minimum_current_1p) * get_phase_factors(allocated_phases, rotation);
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
static void stage_3(StageContext &sc) {
    calculate_window(false, sc);

    Cost wnd_min = sc.ca_state->control_window_min;

    int matched = filter_chargers(ctx.allocated_phases > 0);

    // Reverse sort. Charger that was active for the longest time first.
    // Group chargers that were activated in stage 2 (because a vehicle was just plugged in)
    // behind others and sort those by the plug in timestamp to make sure the same chargers are shut down
    // if phases are overloaded in every iteration.
    // Sort PV chargers before fast chargers (symmetric to stage 4)
    sort_chargers(
        was_just_plugged_in(ctx.state) ? (ChargeMode::_max + 1) : get_highest_charge_mode_bit(ctx.state),
        // We only compare in groups, so was_just_plugged_in(right.state) is true iff it is for left.
        was_just_plugged_in(ctx.left.state) ? (ctx.left.state->just_plugged_in_timestamp < ctx.right.state->just_plugged_in_timestamp) : (ctx.left.state->last_switch_on < ctx.right.state->last_switch_on)
    );

    trace_sort(3);

    // If any phase is overloaded, shut down chargers using this phase until it is not.
    // Maybe try to be more clever here:
    // - Shut down 1p chargers first if only one or two phase limits are below wnd_min
    // - Shut down 1p unknown rotated chargers last
    for (size_t p = 1; p < 4; ++p) {
        for (int i = 0; i < matched; ++i) {
            if (wnd_min[p] <= sc.limits->raw[p]) {
                // Window minimum less than raw phase limit -> phase not overloaded
                trace("3: wnd_min %d <= p%d raw %d", wnd_min[p], p, sc.limits->raw[p]);
                break;
            }

            const auto *state = &sc.charger_state[sc.idx_array[i]];
            const auto alloc_phases = sc.phase_allocation[sc.idx_array[i]];

            if (alloc_phases == 0)
                continue;

            if (alloc_phases == 1 && state->phase_rotation != PhaseRotation::Unknown && (GridPhase)p != get_phase(state->phase_rotation, ChargerPhase::P1)) {
                // This is a 1p charger with known rotation that is not active on this phase.
                // Disabling it will not improve the situation.
                continue;
            }

            trace("3: wnd_min %d > p%d raw %d", wnd_min[p], p, sc.limits->raw[p]);

            // We don't have to recalculate the window but instead can just change the minimum.
            // The window minimum does not have dependencies between chargers.
            wnd_min -= get_minimum_cost(alloc_phases, state->phase_rotation, sc.cfg);
            sc.phase_allocation[sc.idx_array[i]] = 0;

            trace("3: shut down %d", sc.idx_array[i]);
        }
    }

    // If the max PV excess over the last x minutes is less than the PV minimum window,
    // we want to shut down chargers and this decision was stable for x minutes.
    // Also check the hysteresis to make sure the last switch on/off decisions
    // did propagate to the calculated limits.
    for (int i = 0; i < matched; ++i) {
        if (wnd_min.pv <= sc.limits->max_pv || !sc.ca_state->global_hysteresis_elapsed) {
            // Window minimum less than max pv limit -> PV not permanently overloaded
            // or hysteresis is not elapsed yet.
            if (wnd_min.pv <= sc.limits->max_pv) {
                trace("3: wnd_min %d <= max_pv %d", wnd_min.pv, sc.limits->max_pv);
            } else {
                trace("3: wnd_min %d > max_pv %d hyst not elapsed", wnd_min.pv, sc.limits->max_pv);
            }

            break;
        }

        const auto *state = &sc.charger_state[sc.idx_array[i]];
        const auto alloc_phases = sc.phase_allocation[sc.idx_array[i]];

        // Don't shut down chargers that were just plugged in.
        // We want those to be active until plug_in_time is elapsed to make sure the vehicle does not disable its EVCC.
        // Also ignore exceeded PV limits if this charger is not PV charging.
        if (alloc_phases == 0 || was_just_plugged_in(state) || !state->observe_pv_limit)
            continue;

        auto min_cost = get_minimum_cost(alloc_phases, state->phase_rotation, sc.cfg);

        // This is a Min(+PV?) charger that is guaranteed enough PV current to continue charging.
        if (state->observe_pv_limit && min_cost.pv <= state->guaranteed_pv_current)
            continue;

        trace("3: wnd_min %d > max_pv %d", wnd_min.pv, sc.limits->max_pv);

        // We don't have to recalculate the window but instead can just change the minimum.
        // The window minimum does not have dependencies between chargers.
        wnd_min -= min_cost;
        sc.phase_allocation[sc.idx_array[i]] = 0;

        trace("3: shut down %d", sc.idx_array[i]);
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

static bool can_activate(StringWriter &sw, const Cost &check_phase, const Cost &new_cost, const Cost &new_enable_cost, const Cost &wnd_min, const Cost &wnd_max, const CurrentLimits *limits, const CurrentAllocatorConfig *cfg, bool is_unknown_rotated_1p_3p_switch, uint16_t guaranteed_pv_current) {
    // Spread
    bool check_spread = ((check_phase.pv | check_phase.l1 | check_phase.l2 | check_phase.l3) & CHECK_SPREAD) != 0;
    bool check_improvement = ((check_phase.pv | check_phase.l1 | check_phase.l2 | check_phase.l3) & (CHECK_IMPROVEMENT | CHECK_IMPROVEMENT_ALL_PHASE)) != 0;
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
    if (check_improvement && (!check_spread || !improves_all_spread)) {
        bool improves_pv = (check_phase.pv & (CHECK_IMPROVEMENT | CHECK_IMPROVEMENT_ALL_PHASE)) == 0 || (new_cost.pv > 0 && (wnd_max.pv < limits->min.pv || new_cost.pv <= guaranteed_pv_current));
        if (!improves_pv) {
            sw.printf(" No: !impr_pv");
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
            sw.printf(" No: %d %d %d %d",
                            check_all_phases,
                            improves_all_phases,
                            improves_any_phase,
                            is_unknown_rotated_1p_3p_switch);

            // PV is already checked above.
            bool enable = is_unknown_rotated_1p_3p_switch;
            if (!enable)
                return false;
        }
    } else if (check_improvement) {
        sw.printf(" impr_spread ");
    }

    // Check minimum
    for (size_t p = 0; p < 4; ++p) {
        if ((check_phase[p] & (CHECK_MIN_WINDOW_MIN | CHECK_MIN_WINDOW_ENABLE)) == 0 || new_cost[p] <= 0)
            continue;

        // If the guaranteed current is sufficient to activate this charger
        // we don't have to check the PV minimum. It is allowed to activate
        // this charger even if the PV limit will be exceeded.
        if (p == (size_t)GridPhase::PV && new_cost.pv <= guaranteed_pv_current) {
            continue;
        }

        auto required = 0;
        if ((check_phase[p] & CHECK_MIN_WINDOW_ENABLE) != 0)
            required = wnd_min[p] * cfg->enable_current_factor + new_enable_cost[p];
        else if ((check_phase[p] & CHECK_MIN_WINDOW_MIN) != 0)
            required = wnd_min[p] + new_cost[p];

        if (limits->min[p] < required) {
            sw.printf(" No: p%d min %d < req %d ", p, limits->min[p], required);
            return false;
        }
    }

    sw.printf(" Yes");

    return true;
}

// Returns
// - (in *minimum) the minimum cost to keep this charger active
// - (in *enable)  the cost to enable this charger
// - (as return value) the enable current of each of the phases that the charger would be active on
static int get_enable_cost(const ChargerState *state, bool activate_3p, bool have_active_chargers, Cost *minimum, Cost *enable, const CurrentAllocatorConfig *cfg) {
    Cost new_cost;
    Cost new_enable_cost;
    int enable_current;

    new_cost = get_minimum_cost(activate_3p ? 3 : 1, state->phase_rotation, cfg);
    new_enable_cost = (have_active_chargers ? cfg->enable_current_factor : 1.0f) * new_cost;

    if (activate_3p || state->phase_rotation == PhaseRotation::Unknown) {
        enable_current = new_enable_cost.l1;
    } else {
        enable_current = new_enable_cost[get_phase(state->phase_rotation, ChargerPhase::P1)];
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
static bool try_activate(StringWriter &sw, const ChargerState *state, bool activate_3p, bool have_active_chargers, Cost *spent, const CurrentLimits *limits, const CurrentAllocatorConfig *cfg, const CurrentAllocatorState *ca_state) {
    Cost wnd_min = ca_state->control_window_min;
    Cost wnd_max = ca_state->control_window_max;

    Cost new_cost;
    Cost new_enable_cost;

    get_enable_cost(state, activate_3p, have_active_chargers, &new_cost, &new_enable_cost, cfg);

    // If this charger's mode is at least Min, it has a guaranteed PV current > 0.
    // If this current is sufficient to enable the charger, we don't have to check
    // for any improvement: If there's enough current available to enable a Min
    // charger, enable it.
    int improve_check = state->guaranteed_pv_current >= new_cost.pv ? 0 : CHECK_SPREAD | CHECK_IMPROVEMENT;

    // If there are no chargers active, don't require the enable cost on PV.
    // Still require the enable cost on the phases:
    // Phase limits are hard limits. PV can be exceeded for some time.
    // Ignore PV limit completely if this charger is not PV charging.
    Cost check_phase{
        (!state->observe_pv_limit ? 0 :
            improve_check | (have_active_chargers
                                                ? CHECK_MIN_WINDOW_ENABLE
                                                : CHECK_MIN_WINDOW_MIN)),
        improve_check | CHECK_MIN_WINDOW_ENABLE,
        improve_check | CHECK_MIN_WINDOW_ENABLE,
        improve_check | CHECK_MIN_WINDOW_ENABLE
    };

    bool result = can_activate(sw, check_phase, new_cost, new_enable_cost, wnd_min, wnd_max, limits, cfg, false, state->guaranteed_pv_current);
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
static void stage_4(StageContext &sc) {
    // Recalculate window here so that stages 4 - 8 can be called multiple times with different priority groups.
    calculate_window(true, sc);

    if (!sc.ca_state->global_hysteresis_elapsed)
        return;


    // If there is no charger active right now, only require the minimum current
    // instead of the enable current. This is especially relevant for a setup
    // with only one charger.
    bool have_active_chargers = false;
    for (size_t i = 0; i < sc.cfg->charger_count; ++i) {
        if (sc.phase_allocation[i] > 0) {
            have_active_chargers = true;
            break;
        }
    }

    trace(have_active_chargers ? "4: have active chargers." : "4: don't have active chargers.");

    // A charger that was rotated has 0 allocated phases but is still charging.
    int matched = filter_chargers(ctx.allocated_phases == 0 && (ctx.state->wants_to_charge || ctx.state->is_charging) && (ctx.state->charge_mode & ctx.charge_mode_filter) != 0 && !ctx.state->off);

    sort_chargers(
        -get_highest_charge_mode_bit(ctx.state),
        ctx.left.state->allocated_average_power < ctx.right.state->allocated_average_power
    );

    trace_sort(4);

    for (int i = 0; i < matched; ++i) {
        // If we can allocate all available current to the currently active set of chargers
        // and the spread limit is exceeded, short-circuit. We won't enable more chargers.
        if (   sc.ca_state->control_window_max.l1 >= sc.limits->raw.l1
            && sc.ca_state->control_window_max.l2 >= sc.limits->raw.l2
            && sc.ca_state->control_window_max.l3 >= sc.limits->raw.l3
            && sc.ca_state->control_window_min.l1 * sc.cfg->enable_current_factor >= sc.limits->spread.l1
            && sc.ca_state->control_window_min.l2 * sc.cfg->enable_current_factor >= sc.limits->spread.l2
            && sc.ca_state->control_window_min.l3 * sc.cfg->enable_current_factor >= sc.limits->spread.l3) {
            trace("4: wnd_max >= raw, wnd_ena >= spread");
            break;
        }

        // If the window minimum is closer than the smaller minimum current
        // to the min limit on any phase, short-circuit. We won't enable more chargers.
        if (   sc.limits->min.l1 - sc.ca_state->control_window_min.l1 < std::min(sc.cfg->minimum_current_1p, sc.cfg->minimum_current_3p)
            && sc.limits->min.l2 - sc.ca_state->control_window_min.l2 < std::min(sc.cfg->minimum_current_1p, sc.cfg->minimum_current_3p)
            && sc.limits->min.l3 - sc.ca_state->control_window_min.l3 < std::min(sc.cfg->minimum_current_1p, sc.cfg->minimum_current_3p)) {
            trace("4: min - wnd_min < min_1p");
            break;
        }

        const auto *state = &sc.charger_state[sc.idx_array[i]];

        bool force_3p = !deadline_elapsed(state->last_phase_switch + sc.cfg->global_hysteresis) && state->phases == 3;
        bool force_1p = !deadline_elapsed(state->last_phase_switch + sc.cfg->global_hysteresis) && state->phases == 1;

        bool is_fixed_3p = state->phases == 3 && !state->phase_switch_supported;

        // Prefer unknown rotated switchable chargers to be active on all three phases.
        // In that case we know what the charger is doing.
        bool is_unknown_rot_switchable = state->phase_rotation == PhaseRotation::Unknown && state->phase_switch_supported;

        bool try_3p = !force_1p && (force_3p || is_fixed_3p || is_unknown_rot_switchable);
        bool try_1p = !force_3p && !is_fixed_3p;

        char buf[256];
        StringWriter sw{buf, ARRAY_SIZE(buf)};

        sw.printf("4: %d:", sc.idx_array[i]);

        uint8_t phase_alloc = 0;

        if (try_3p && try_activate(sw, state, true, have_active_chargers, nullptr, sc.limits, sc.cfg, sc.ca_state)) {
            try_1p = false;
            phase_alloc = 3;
        }

        if (try_1p && try_activate(sw, state, false, have_active_chargers, nullptr, sc.limits, sc.cfg, sc.ca_state)) {
            phase_alloc = 1;
        }

        sc.phase_allocation[sc.idx_array[i]] = phase_alloc;

        sw.printf(" (%dp)", sc.phase_allocation[sc.idx_array[i]]);
        trace("%s", buf);

        if (phase_alloc > 0) {
            // Only allow the first charger to be activated with the minimum current. Require the enable current now.
            have_active_chargers = true;
            calculate_window(true, sc);
        }
    }
}

// Stage 5: Switch 1p -> 3p if possible
// - This is conceptionally similar to stage 4 except that chargers already have one phase allocated
// - If there are still phases with a limit greater than the window maximum,
//   switch chargers that have 1 phase allocated to 3 phases if possible
// - Only chargers that _currently_ support the phase switch are considered
// - Sort by allocated energy ascending as in stage 4
// - Enable conditions are similar to stage 4:
//   The enable cost is the cost to enable a charger with three phases minus the cost (that already was subtracted in stage 3) to enable it with one phase.
static void stage_5(StageContext &sc) {
    Cost wnd_min = sc.ca_state->control_window_min;
    Cost wnd_max = sc.ca_state->control_window_max;

    auto min_3p = sc.cfg->minimum_current_3p;

    int active_chargers = 0;
    for (size_t i = 0; i < sc.cfg->charger_count; ++i) {
        if (sc.phase_allocation[i] != 0) {
            ++active_chargers;
        }
    }

    // - If there are no chargers active this stage will do nothing.
    // - If there are more than one charger active, require the enable current on the PV phase.
    // - If there is only one charger active, this could be the one charger that is about to be switched to 3p.
    //   In that case require only the minimum current on the PV phase.
    // check_pv_min does not have to be set to false after switching one charger to 3p:
    // either it was already false, or there is no chargers active (then the loop does nothing)
    // or there is exactly one charger active, then the loop only runs once.
    auto check_pv_min = active_chargers < 2 ? CHECK_MIN_WINDOW_MIN : CHECK_MIN_WINDOW_ENABLE;

    trace(check_pv_min ? "5: <2 active. pv reqs min" : "5: >1 active. pv reqs ena");

    int matched = filter_chargers(ctx.allocated_phases == 1 && ctx.state->phase_switch_supported && deadline_elapsed(ctx.state->last_phase_switch + ctx.cfg->global_hysteresis) && (ctx.state->charge_mode & ctx.charge_mode_filter) != 0);

    sort_chargers(
        -get_highest_charge_mode_bit(ctx.state),
        ctx.left.state->allocated_average_power < ctx.right.state->allocated_average_power
    );

    trace_sort(5);

    for (int i = 0; i < matched; ++i) {
        const auto *state = &sc.charger_state[sc.idx_array[i]];

        // If the hysteresis is not elapsed yet, still allow 1p->3p switch for chargers
        // that were activated in this iteration. Those ignored the hysteresis anyway
        // and we can skip the phase switch by immediately allocating three phases.
        // Note that this is **not** the same check as the one in stage 9 (waking up chargers)
        if (!sc.ca_state->global_hysteresis_elapsed && sc.charger_allocation_state[sc.idx_array[i]].allocated_current != 0)
            continue;

        Cost new_cost = min_3p * Cost{3, 1, 1, 1};
        Cost new_enable_cost = min_3p * sc.cfg->enable_current_factor * Cost{3, 1, 1, 1};

        auto already_allocated = get_minimum_cost(1, state->phase_rotation, sc.cfg);

        auto old_factors = get_phase_factors(1, state->phase_rotation);
        // TODO use already allocated current here
        new_cost -= already_allocated;
        // TODO should we subtract the 1p enable cost here?
        new_enable_cost -= already_allocated;

        // If this charger's mode is at least Min, it has a guaranteed PV current > 0.
        // If this current is sufficient to enable the charger, we don't have to check
        // for any improvement: If there's enough current available to enable a Min
        // charger, enable it.
        // This is the same check as the one in try_activate, except that we already have some
        // current allocated to the charger for it to run 1p.
        int improve_check = (state->guaranteed_pv_current - already_allocated.pv >= new_cost.pv) ? 0 : CHECK_IMPROVEMENT_ALL_PHASE;

        // Only switch from one to three phase if there is still current available on all phases except the P1 phase of this charger: P1 is used by this charger anyway.
        Cost check_phase{
            !state->observe_pv_limit ? 0 : (improve_check | check_pv_min),
            (old_factors.l1 > 0 ? 0 : improve_check) | CHECK_MIN_WINDOW_ENABLE,
            (old_factors.l2 > 0 ? 0 : improve_check) | CHECK_MIN_WINDOW_ENABLE,
            (old_factors.l3 > 0 ? 0 : improve_check) | CHECK_MIN_WINDOW_ENABLE
        };

        char buf[256];
        StringWriter sw{buf, ARRAY_SIZE(buf)};

        sw.printf("5: %d:", sc.idx_array[i]);
        if (!can_activate(sw, check_phase, new_cost, new_enable_cost, wnd_min, wnd_max, sc.limits, sc.cfg, state->phase_rotation == PhaseRotation::Unknown, state->guaranteed_pv_current)) {
            trace("%s", buf);
            continue;
        }

        trace("%s", buf);

        sc.phase_allocation[sc.idx_array[i]] = 3;
        calculate_window(true, sc);
        wnd_min = sc.ca_state->control_window_min;
        wnd_max = sc.ca_state->control_window_max;
    }
}

// Stage 6: Allocate minimum current to chargers with at least one allocated phase
static void stage_6(StageContext &sc) {
    int matched = filter_chargers(ctx.allocated_phases > 0 && ctx.allocated_current == 0 && (ctx.state->charge_mode & ctx.charge_mode_filter) != 0);

    // No need to sort here: We know that we have enough current to give each charger its minimum current.
    // A charger that can't be activated has 0 phases allocated.

    trace_sort(6);

    for (int i = 0; i < matched; ++i) {
        const auto *state = &sc.charger_state[sc.idx_array[i]];

        auto allocated_phases = sc.phase_allocation[sc.idx_array[i]];
        auto cost = get_minimum_cost(allocated_phases, state->phase_rotation, sc.cfg);

        // This should never happen:
        // We should never allocate more current than the raw phase limits,
        // not even to allocate the minimum current to a charger. If there
        // is a charger that was allocated phases to before stage 6, but we
        // don't have the minimum current available, that is a bug in the
        // previous stages.
        if (cost_exceeds_limits(cost, sc.limits, 6, state->observe_pv_limit, state->guaranteed_pv_current)) {
            logger.printfln("stage 6: Cost exceeded limits!");
            print_alloc(6, sc);
            PRINT_COST(cost);
            PRINT_COST(sc.ca_state->control_window_min);
            PRINT_COST(sc.ca_state->control_window_max);
            continue;
        }

        sc.current_allocation[sc.idx_array[i]] = cost.max_phase();
        trace("6: %d: %d@%dp", sc.idx_array[i], sc.current_allocation[sc.idx_array[i]], allocated_phases);
        apply_cost(cost, sc.limits);
    }
}

// The current capacity of a charger is the maximum amount of current that can be allocated to the charger additionally to the already allocated current on the allocated phases.
static int current_capacity(const CurrentLimits *limits, const ChargerState *state, int allocated_current, uint8_t allocated_phases, const CurrentAllocatorConfig *cfg) {
    auto requested_current = get_requested_current(state, cfg, allocated_phases);

    // TODO: add margin again if exactly one charger is active and requested_current > 6000. Also add in calculate_window? -> Maybe not necessary any more?

    auto capacity = std::max(requested_current - allocated_current, 0);

    if (allocated_phases == 3 || state->phase_rotation == PhaseRotation::Unknown) {
        return std::min(capacity, limits->raw.min_phase());
    }

    for (size_t i = (size_t)ChargerPhase::P1; i < (size_t)ChargerPhase::P1 + allocated_phases; ++i) {
        auto phase = get_phase(state->phase_rotation, (ChargerPhase)i);
        capacity = std::min(capacity, limits->raw[phase]);
    }

    return allocated_phases * capacity;
}

static Cost get_fair_current(int matched, int start, int *idx_array, uint8_t *phase_allocation, const CurrentLimits *limits, const ChargerState *charger_state) {
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
static void stage_7(StageContext &sc) {
    int matched = filter_chargers(ctx.allocated_current > 0 && (ctx.state->charge_mode & ctx.charge_mode_filter) != 0);

    if (matched == 0)
        return;

    sort_chargers(
        3 - ctx.allocated_phases,
        current_capacity(ctx.limits, ctx.left.state, ctx.left.allocated_current, ctx.left.allocated_phases, ctx.cfg) < current_capacity(ctx.limits, ctx.right.state, ctx.right.allocated_current, ctx.right.allocated_phases, ctx.cfg)
    );

    trace_sort(7);

    for (int i = 0; i < matched; ++i) {
        Cost fair = get_fair_current(matched, i, sc.idx_array, sc.phase_allocation, sc.limits, sc.charger_state);

        const auto *state = &sc.charger_state[sc.idx_array[i]];

        auto allocated_current = sc.current_allocation[sc.idx_array[i]];
        auto allocated_phases = sc.phase_allocation[sc.idx_array[i]];

        auto current = state->observe_pv_limit ? std::max(state->guaranteed_pv_current / allocated_phases - allocated_current, fair.pv / allocated_phases) : 32000;

        if (state->phase_rotation == PhaseRotation::Unknown) {
            current = std::min(current, fair.min_phase());
        } else {
            for (size_t p = 0; p < allocated_phases; ++p) {
                auto phase = get_phase(state->phase_rotation, (ChargerPhase)((size_t)ChargerPhase::P1 + p));
                current = std::min(current, fair[phase]);
            }
        }

        // Don't allocate more than the enable current to a charger that does not charge.
        if (!state->is_charging) {
            Cost enable_cost;
            auto enable_current = get_enable_cost(state, allocated_phases == 3, true, nullptr, &enable_cost, sc.cfg);
            current = std::min(current, std::max(0, enable_current - allocated_current));
        }

        current = std::min(current, current_capacity(sc.limits, state, allocated_current, allocated_phases, sc.cfg));
        current += allocated_current;

        auto cost = get_cost(current, (ChargerPhase)allocated_phases, state->phase_rotation, allocated_current, (ChargerPhase)allocated_phases);

        // This should never happen:
        // We've just calculated how much current is still available.
        // If this cost exceeds the limits, stage_7 is bugged.
        if (cost_exceeds_limits(cost, sc.limits, 7, state->observe_pv_limit, state->guaranteed_pv_current)) {
            logger.printfln("stage 7: Cost exceeded limits!");
            print_alloc(7, sc);
            PRINT_COST(cost);
            PRINT_COST(sc.ca_state->control_window_min);
            PRINT_COST(sc.ca_state->control_window_max);
            continue;
        }

        sc.current_allocation[sc.idx_array[i]] = current;
        apply_cost(cost, sc.limits);
        trace("7: %d: %s%d@%dp", sc.idx_array[i], !state->is_charging ? "!chrg " : "", sc.current_allocation[sc.idx_array[i]], allocated_phases);
    }
}

// Stage 8: Allocate left-over current.
// - Group by phases. Makes sure that we allocate current to three phase chargers first.
//   If there is current left after this, we've probably hit one of the phase limits.
//   One phase chargers on other phases will take the rest if possible.
// - Sort by current_capacity ascending. This makes sure that one pass is enough to allocate the possible maximum.
static void stage_8(StageContext &sc) {
    // Chargers that are currently not charging already have the enable current allocated (if available) by stage 7.
    int matched = filter_chargers(ctx.allocated_current > 0 && ctx.state->is_charging && (ctx.state->charge_mode & ctx.charge_mode_filter) != 0);

    sort_chargers(
        3 - ctx.allocated_phases,
        current_capacity(ctx.limits, ctx.left.state, ctx.left.allocated_current, ctx.left.allocated_phases, ctx.cfg) < current_capacity(ctx.limits, ctx.right.state, ctx.right.allocated_current, ctx.right.allocated_phases, ctx.cfg)
    );

    trace_sort(8);

    for (int i = 0; i < matched; ++i) {
        const auto *state = &sc.charger_state[sc.idx_array[i]];

        auto allocated_current = sc.current_allocation[sc.idx_array[i]];
        auto allocated_phases = sc.phase_allocation[sc.idx_array[i]];

        auto current = std::min(
                        std::max(
                            0,
                            state->observe_pv_limit
                                ? std::max(state->guaranteed_pv_current / allocated_phases - allocated_current, sc.limits->raw.pv / allocated_phases)
                                : 32000),
                        current_capacity(sc.limits, state, allocated_current, allocated_phases, sc.cfg));

        if (state->phase_rotation == PhaseRotation::Unknown) {
            // Phase rotation unknown. We have to assume that each phase could be used
            current = std::min(current, sc.limits->raw.min_phase());
        } else {
            for (int p = 1; p <= (int)allocated_phases; ++p) {
                current = std::min(current, sc.limits->raw[get_phase(state->phase_rotation, (ChargerPhase)p)]);
            }
        }

        current += allocated_current;

        auto cost = get_cost(current, (ChargerPhase)allocated_phases, state->phase_rotation, allocated_current, (ChargerPhase)allocated_phases);

        // This should never happen:
        // We've just calculated how much current is still available.
        // If this cost exceeds the limits, stage_8 is bugged.
        if (cost_exceeds_limits(cost, sc.limits, 8, state->observe_pv_limit, state->guaranteed_pv_current)) {
            logger.printfln("stage 8: Cost exceeded limits! Charger %d Current %u", sc.idx_array[i], current);
            print_alloc(8, sc);
            PRINT_COST(cost);
            PRINT_COST(sc.ca_state->control_window_min);
            PRINT_COST(sc.ca_state->control_window_max);
            continue;
        }

        sc.current_allocation[sc.idx_array[i]] = current;
        apply_cost(cost, sc.limits);
        trace("8: %d: %d@%dp", sc.idx_array[i], sc.current_allocation[sc.idx_array[i]], allocated_phases);
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
            return left_state->last_switch_on < right_state->last_switch_on;
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
static void stage_9(StageContext &sc) {
    bool have_active_chargers = sc.ca_state->control_window_min.pv != 0;

    trace(have_active_chargers ? "9: have active chargers." : "9: don't have active chargers.");

    int matched = filter_chargers(ctx.allocated_phases == 0 && (ctx.state->wants_to_charge_low_priority || (ctx.state->wants_to_charge && ctx.state->last_wakeup != 0_us)));

    if (matched == 0)
        return;

    sort_chargers(
        stage_9_group(ctx.state, ctx.cfg),
        stage_9_sort(ctx.left.state, ctx.right.state, ctx.cfg)
    );

    trace_sort(9);

    for (int i = 0; i < matched; ++i) {
        const auto *state = &sc.charger_state[sc.idx_array[i]];

        // If the hysteresis is not elapsed yet, still allow waking up
        // chargers that had current allocated in the last iteration.
        // A charger that is being woken up is not regarded as active
        // and its current/phases are deallocated in stage 1 to
        // free up the current in case it is needed for "normal" charging.
        // Only if the current is still left over in this iteration, the
        // charger that is being woken up is reallocated the current.
        // Note that this is **not** the same check as the one in stage 5 (1p->3p switch)
        if (!sc.ca_state->global_hysteresis_elapsed && sc.charger_allocation_state[sc.idx_array[i]].allocated_current == 0)
            continue;

        bool force_3p = !deadline_elapsed(state->last_phase_switch + sc.cfg->global_hysteresis) && state->phases == 3;
        bool force_1p = !deadline_elapsed(state->last_phase_switch + sc.cfg->global_hysteresis) && state->phases == 1;

        bool is_fixed_3p = state->phases == 3 && !state->phase_switch_supported;

        // Prefer unknown rotated switchable chargers to be active on all three phases.
        // In that case we know what the charger is doing.
        bool is_unknown_rot_switchable = state->phase_rotation == PhaseRotation::Unknown && state->phase_switch_supported;

        bool try_3p = !force_1p && (force_3p || is_fixed_3p || is_unknown_rot_switchable);
        bool try_1p = !force_3p && !is_fixed_3p;

        Cost enable_cost;
        int enable_current;
        uint8_t phase_alloc = 0;

        if (try_3p) {
            enable_current = get_enable_cost(state, true, have_active_chargers, nullptr, &enable_cost, sc.cfg);
            if (!cost_exceeds_limits(enable_cost, sc.limits, 9, state->observe_pv_limit, state->guaranteed_pv_current)) {
                try_1p = false;
                phase_alloc = 3;
            }
        }

        if (try_1p) {
            enable_current = get_enable_cost(state, false, have_active_chargers, nullptr, &enable_cost, sc.cfg);
            if (!cost_exceeds_limits(enable_cost, sc.limits, 9, state->observe_pv_limit, state->guaranteed_pv_current)) {
                phase_alloc = 1;
            }
        }

        if (phase_alloc == 0)
            continue;

        apply_cost(enable_cost, sc.limits);

        sc.phase_allocation[sc.idx_array[i]] = phase_alloc;
        sc.current_allocation[sc.idx_array[i]] = enable_current;
        trace("9: %d: %d@%dp", sc.idx_array[i], sc.current_allocation[sc.idx_array[i]], sc.phase_allocation[sc.idx_array[i]]);

        calculate_window(true, sc);
    }
}

int allocate_current(
    const CurrentAllocatorConfig *cfg,
    CurrentLimits *limits,
    const bool cp_disconnect_requested,
    /*const TODO: move allocated_energy into charger allocation state so that this can be const once again*/ ChargerState *charger_state,
    const char * const *hosts,
    const std::function<const char *(uint8_t)> &get_charger_name,
    const std::function<void(uint8_t)> &clear_dns_cache_entry,

    CurrentAllocatorState *ca_state,
    ChargerAllocationState *charger_allocation_state,
    uint32_t *allocated_current
    )
{
    logger.trace_timestamp(charge_manager.trace_buffer_index);

    // TODO use enum for this. See charge_manager.cpp state definition for constants.
    int result = 1;

    bool print_local_log = false;
    char *local_log = cfg->distribution_log.get();
    bool vehicle_connected = false;

    LOCAL_LOG_FULL("", "Allocating current");

    assert(cfg->charger_count > 0 && cfg->charger_count <= MAX_CONTROLLED_CHARGERS);
    int current_array[MAX_CONTROLLED_CHARGERS] = {0};
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
            if (deadline_elapsed(charger.last_update + TIMEOUT)) {
                // Only shut down this charger.
                // If a charger does not receive manager packets anymore,
                // it will shut down after 30 seconds.
                charger.off = true;

                if (charger_alloc.error != CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE)
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
                LOCAL_LOG("EVSE of %s (%s) reachable again.", get_charger_name(i), hosts[i]);
            }

            // Charger did not update the charging current or phases in time.
            // It is not an error if the charger reports != 0 phases but 0 are allocated,
            // because the charger reports the number of phases that would be active if
            // it was charging. Check is_charging instead.
            if ((charger_alloc.allocated_current < charger.allowed_current
                || (charger_alloc.allocated_phases != 0 && charger_alloc.allocated_phases < charger.phases)
                || (charger_alloc.allocated_phases == 0 && charger.is_charging))
               && deadline_elapsed(charger_alloc.last_sent_config + TIMEOUT)) {
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
            for (int i = 0; i < cfg->charger_count; ++i) {
                charger_state[i].off = true;
            }
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

    StageContext sc{
        idx_array,
        current_array,
        phases_array,
        limits,
        charger_state,
        cfg->charger_count,
        cfg,
        ca_state,
        charger_allocation_state,
        0xFFFFFFFF
    };

    trace_alloc(0, sc);
    trace("__all__");
    stage_1(sc);
    stage_2(sc);
    stage_3(sc);
    stage_6(sc);

    for (ChargeMode::Type mode = ChargeMode::_max; mode >= ChargeMode::Min; mode = (ChargeMode::Type)((int)mode >> 1)) {
        sc.charge_mode_filter = mode;
        // Run Min and PV as one mode.
        if (mode == ChargeMode::Min) {
            sc.charge_mode_filter |= ChargeMode::PV;
            trace("__only [Min,PV,Min+PV]__");
        } else {
            trace("__only %s__", ChargeMode::Strings[(size_t)mode]);
        }

        stage_4(sc);
        stage_5(sc);
        stage_6(sc);
        stage_7(sc);
        stage_8(sc);
    }

    trace("__all__");
    stage_9(sc);
    trace_alloc(9, sc);
    //logger.printfln("Took %u µs", end - start);

    auto now = now_us();

    // Apply current limits.
    {
        for (int i = 0; i < cfg->charger_count; ++i) {
            auto &charger = charger_state[i];
            auto &charger_alloc = charger_allocation_state[i];

            uint16_t current_to_set = current_array[i];
            int8_t phases_to_set = phases_array[i];
            auto old_phases = charger_alloc.allocated_phases;

            if (old_phases != phases_to_set) {
                // Don't reset hysteresis if a charger is shut down. Re-activating a charger is (always?) fine.
                if(phases_to_set != 0) {
                    ca_state->last_hysteresis_reset = now;
                }

                if (old_phases == 0) {
                    charger.last_switch_on = now;
                }

            }

            // This detects a phase switch even if the charger is currently not activated:
            // charger.phases is 1 or 3, never 0.
            if (charger.phases != phases_to_set && phases_to_set != 0) {
                charger.last_phase_switch = now;
            }

            if (charger.wants_to_charge_low_priority && old_phases == 0 && phases_to_set != 0) {
                LOCAL_LOG("Waking up vehicle at %s (%s).",
                          get_charger_name(i),
                          hosts[i]);

                trace("charger %d: waking up", i);
                charger.last_wakeup = now;
            }

            // If we can't allocate the requested current,
            // ignore the requested current for some time.
            // If there is more current available in the next iteration,
            // use the supported current for a faster ramp up.
            // The requested current will be the last allocation + margin until we can fulfill it.
            if (current_to_set < charger.requested_current) {
                //trace("charger %d: requested current not fulfilled. Will use supported current for 1 min.", i);
                charger.use_supported_current = now;
            }

            // The charger was just plugged in. If we've allocated phases to it for PLUG_IN_TIME, clear the timestamp
            // to reduce its priority.
            if (charger.just_plugged_in_timestamp != 0_us && phases_to_set > 0 && deadline_elapsed(charger.last_switch_on + cfg->plug_in_time)) {
                trace("charger %d: clearing just_plugged_in_timestamp after deadline elapsed", i);
                charger.just_plugged_in_timestamp = 0_us;
            }

            // The charger was just plugged in, we've allocated phases to it in the last iteration but no phases to it in this iteration.
            // As stage 3 (switching chargers off if phases are overloaded) sorts chargers by just_plugged_in_timestamp ascending if it is not 0,
            // the sort order is stable, so we've just hit a phase limit that was not as restrictive in the last iteration.
            // Clear the timestamp to make sure
            if (charger.just_plugged_in_timestamp != 0_us && charger_alloc.allocated_phases > 0 && phases_to_set == 0) {
                trace("charger %d: clearing just_plugged_in_timestamp; phases overloaded?", i);
                charger.just_plugged_in_timestamp = 0_us;
            }

            bool change = charger_alloc.allocated_current != current_to_set || charger_alloc.allocated_phases != phases_to_set;
            charger_alloc.allocated_current = current_to_set;
            charger_alloc.allocated_phases = phases_to_set;

            auto charging_time = (now - charger.last_plug_in).as<float>();

            if (phases_to_set != 0) {
                auto amps = (float)current_to_set * phases_to_set / 1000.0f;

                auto alloc_time = !deadline_elapsed(charger.last_plug_in + cfg->allocation_interval) ? charging_time : cfg->allocation_interval.as<float>();

                auto amp_hours = amps * (alloc_time / ((micros_t)1_h).as<float>());
                auto watt_hours = amp_hours * 230.0f;
                auto allocated_energy = watt_hours / 1000.0f;
                charger.allocated_energy += allocated_energy;
            }

            charging_time /= 1000.0 * 1000.0 * 60.0 * 60.0;
            charger.allocated_average_power = charger.allocated_energy / (float)charging_time;
            if (phases_to_set != 0 && charger.is_charging) {
                // If this charger is in Eco+Min(+*),
                // and the eco mode check returned that charging is currently expensive
                // and we've allocated only the guaranteed current,
                // don't count this as time in state C:
                // We would always use the first X hours if the charge plan is "use the cheapest X out of Y hours".
                bool eco_min_pv_hack = (charger.charge_mode & (ChargeMode::Eco | ChargeMode::Min)) == (ChargeMode::Eco | ChargeMode::Min)
                                    && !charger.eco_fast
                                    && (current_to_set * phases_to_set) <= charger.guaranteed_pv_current;
                if (!eco_min_pv_hack) {
                    charger.time_in_state_c += cfg->allocation_interval;
                }
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

#if MODULE_POWER_MANAGER_AVAILABLE()
    power_manager.print_trace_header();
#endif

    return result;
}


static uint8_t get_charge_state(uint8_t charger_state, uint16_t supported_current, uint32_t car_stopped_charging, uint16_t target_allocated_current)
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
        if (car_stopped_charging == 0)
            return 2; // Not charged this session
        else
            return 6; // Charged at least once
    }
    if (charger_state == 2)
        return 3; // Waiting for the car to start charging

    logger.printfln("Unknown state! cs %u sc %u ct %lu tac %u", charger_state, supported_current, car_stopped_charging, target_allocated_current);
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
    const std::function<const char *(uint8_t)> &get_charger_name
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
        logger.printfln("Received stale charger state from %s (%s). Reported EVSE uptime (%lu) is the same as in the last state. Is the EVSE still reachable?",
            get_charger_name(client_id), hosts[client_id],
            v1->evse_uptime);
        if (deadline_elapsed(target.last_update + 10_s)) {
            target_alloc.state = 5;
            target_alloc.error = CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE;
        }

        return false;
    }

    target.uid = v1->esp32_uid;
    target.uptime = v1->evse_uptime;

    // If we've just resolved this charger but the charger did not reboot
    // we can receive a packet before successfully sending the first one
    // If the allocation algorithm runs between receiving the first packet
    // and sending the first packet, we wrongly assume that this charger's EVSE
    // does not react. (This only happens if resolving the charger takes just long enough, ~ 30 seconds)
    // To fix this, fake that we've sent a config just now.
    if (charger_allocation_state->last_sent_config == 0_us)
        charger_allocation_state->last_sent_config = now_us();

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
    bool wants_to_charge = (v1->car_stopped_charging == 0 && v1->supported_current != 0 && (v1->charger_state == 1 || v1->charger_state == 2)) || v1->charger_state == 3;
    target.wants_to_charge = wants_to_charge;

    // A charger wants to charge and has low priority if it has already charged this vehicle
    // AND only the charge manager slot (charger_state == 1, supported_current != 0) or no slot (charger_state == 2) blocks.
    bool low_prio = v1->car_stopped_charging != 0 && v1->supported_current != 0 && (v1->charger_state == 1 || v1->charger_state == 2);

    if (!target.wants_to_charge_low_priority && low_prio)
        target.last_wakeup = now_us() - cfg->wakeup_time;

    target.wants_to_charge_low_priority = low_prio;

    target.is_charging = v1->charger_state == 3;
    if (v1->charger_state != 1 && v1->charger_state != 2)
        target.last_wakeup = 0;

    // Reset allocated energy if no car is connected
    if (v1->charger_state == 0) {
        target.allocated_energy = 0;
        target.allocated_average_power = 0;
        target_alloc.allocated_current = 0;
        target_alloc.allocated_phases = 0;
    }

    target.allowed_current = v1->allowed_charging_current;

    if (target.supported_current != v1->supported_current)
        trace("RECV %d: supported %u -> %u mA", client_id, target.supported_current, v1->supported_current);
    target.supported_current = v1->supported_current;
    target.cp_disconnect_supported = CM_FEATURE_FLAGS_CP_DISCONNECT_IS_SET(v1->feature_flags);
    target.cp_disconnect_state = CM_STATE_FLAGS_CP_DISCONNECTED_IS_SET(v1->state_flags);

    if (target.charger_state == 0 && v1->charger_state != 0) {
        target.last_plug_in = now_us();

        // Wait for A -> non-A transitions, but ignore chargers that are already in a non-A state in their first packet.
        // Only set the timestamp if plug_in_time is != 0: This feature is deactivated if the time is set to 0.
        if (target.last_update != 0_us && target.charger_state == 0 && v1->charger_state != 0 && cfg->plug_in_time != 0_us)
            target.just_plugged_in_timestamp = now_us();
    }

    // If this charger just switched to state C (i.e. the contactor switched on)
    // set last_phase_switch to now to make sure we don't immediately switch again.
    // The delay between the phase switch and the car requesting current again
    // could be longer than the hysteresis. In that case we would be able to
    // immediately phase switch again after switching to C, if we don't prevent this here.
    if (target.charger_state != v1->charger_state && v1->charger_state == 3)
        target.last_phase_switch = now_us();

    if (v1->charger_state == 0) {
        target.last_phase_switch = -cfg->global_hysteresis;
        target.time_in_state_c = 0_us;
        target.last_plug_in = 0_us;
    }

    target.charger_state = v1->charger_state;
    target.last_update = now_us();

    uint16_t requested_current = v1->supported_current;

    if (v2 != nullptr && v1->charger_state == 3 && v2->time_since_state_change >= cfg->requested_current_threshold * 1000) {
        int max_phase_current = -1;

        for (int i = 0; i < 3; i++) {
            if (isnan(v1->line_currents[i])) {
                // Don't trust the line currents if one is missing.
                max_phase_current = 32000;
                break;
            }

            max_phase_current = std::max(max_phase_current, (int)(v1->line_currents[i] * 1000.0f));
        }
        // The CM protocol sends 0 instead of nan.
        if (max_phase_current == 0)
            max_phase_current = 32000;

        max_phase_current += cfg->requested_current_margin;

        max_phase_current = std::max(6000, std::min(32000, max_phase_current));
        requested_current = std::min(requested_current, (uint16_t)max_phase_current);
    }
    if (abs((int)target.requested_current - (int)requested_current) > 1500) {
        trace("RECV %d: requested %u -> %u mA (measured %.3fA %.3fA %.3fA)", client_id, target.requested_current, requested_current, v1->line_currents[0], v1->line_currents[1], v1->line_currents[2]);
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
                                              v1->car_stopped_charging,
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
