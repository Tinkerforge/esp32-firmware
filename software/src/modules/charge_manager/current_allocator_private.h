/* esp32-firmware
 * Copyright (C) 2024 Erik Fleckstein <erik@tinkerforge.com>
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

#pragma once

#include "current_allocator.h"

struct StageContext {
    int *idx_array;
    int *current_allocation;
    uint8_t *phase_allocation;
    ChargerDecision *charger_decisions;
    GlobalDecision *global_decision;
    CurrentLimits *limits;
    const ChargerState *charger_state;
    size_t charger_count;
    const CurrentAllocatorConfig *cfg;
    CurrentAllocatorState *ca_state;
    const ChargerAllocationState *charger_allocation_state;
    uint32_t charge_mode_filter;
};


struct FilterContext {
    int allocated_current;
    uint8_t allocated_phases;
    const CurrentAllocatorConfig *cfg;
    const ChargerState *state;
    uint32_t charge_mode_filter;
};

struct GroupContext {
    int allocated_current;
    uint8_t allocated_phases;
    const ChargerState *state;
    const CurrentAllocatorConfig *cfg;
    const ChargerAllocationState *alloc_state;
};

struct CompareInfo {
    int allocated_current;
    uint8_t allocated_phases;
    const ChargerState *state;
};

struct CompareContext {
    CompareInfo left;
    CompareInfo right;
    CurrentLimits *limits;
    const CurrentAllocatorConfig *cfg;
};

typedef bool(*filter_fn)(const FilterContext &ctx);

int filter_chargers_impl(filter_fn filter, StageContext &sc);

typedef int(*group_fn)(const GroupContext &ctx);

typedef bool(*compare_fn)(const CompareContext &ctx);

void sort_chargers_impl(group_fn group, compare_fn compare, StageContext &sc);

#define filter_chargers(x) filter_chargers_impl([](const FilterContext &ctx) { \
            return (x); \
        }, \
        sc)

#define sort_chargers(group, filter) do {\
    sort_chargers_impl( \
        [](const GroupContext &ctx) { \
            return (group); \
        }, \
        [](const CompareContext &ctx) { \
            return (filter); \
        }, \
        sc, \
        matched); \
    } while (0)

GridPhase get_phase(PhaseRotation rot, ChargerPhase phase);

Cost get_cost(int current_to_allocate,
              ChargerPhase phases_to_allocate,
              PhaseRotation rot,
              int allocated_current,
              ChargerPhase allocated_phases);

bool cost_exceeds_limits(Cost cost, const CurrentLimits* limits, int stage, bool observe_pv_limit);
