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

typedef bool(*filter_fn)(int32_t /*allocated_current*/, uint8_t /*allocated_phases*/, const CurrentAllocatorConfig */*cfg*/, const ChargerState */*state*/);

int filter_chargers_impl(filter_fn filter, int *idx_array, const int32_t *current_allocation, const uint8_t *phase_allocation, const ChargerState *charger_state, size_t charger_count);

typedef int(*group_fn)(int32_t /*allocated_current*/, uint8_t /*allocated_phases*/, const ChargerState */*state*/, const CurrentAllocatorConfig * /*cfg*/);

struct CompareInfo {
    int32_t allocated_current;
    uint8_t allocated_phases;
    const ChargerState *state;
};
typedef bool(*compare_fn)(CompareInfo /*left*/, CompareInfo /*right*/, CurrentLimits * /*limits*/, const CurrentAllocatorConfig * /*cfg*/);

void sort_chargers_impl(group_fn group, compare_fn compare, int *idx_array, const int32_t *current_allocation, const uint8_t *phase_allocation, const ChargerState *charger_state, size_t charger_count, CurrentLimits *limits, const CurrentAllocatorConfig *cfg);

GridPhase get_phase(PhaseRotation rot, ChargerPhase phase);

Cost get_cost(int32_t current_to_allocate,
              ChargerPhase phases_to_allocate,
              PhaseRotation rot,
              int32_t allocated_current,
              ChargerPhase allocated_phases);

bool cost_exceeds_limits(Cost cost, const CurrentLimits* limits, int stage);

void stage_1(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state);
void stage_2(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state);
void stage_3(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state);
void stage_4(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state);
void stage_5(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state);
void stage_6(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state);
void stage_7(int *idx_array, int32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, CurrentAllocatorState *ca_state);

#define filter_chargers(x) do { \
    matched = filter_chargers_impl([](int32_t allocated_current, uint8_t allocated_phases, const CurrentAllocatorConfig *_cfg, const ChargerState *state) { \
            return (x); \
        }, \
        idx_array, \
        current_allocation, \
        phase_allocation, \
        cfg, \
        charger_state, \
        charger_count); \
    } while(0)

#define sort_chargers(group, filter) do {\
    sort_chargers_impl( \
        [](int32_t allocated_current, uint8_t allocated_phases, const ChargerState *state, const CurrentAllocatorConfig *_cfg) { \
            return (group); \
        }, \
        [](CompareInfo left, CompareInfo right, CurrentLimits *_limits, const CurrentAllocatorConfig *_cfg) { \
            return (filter); \
        }, \
        idx_array, \
        current_allocation, \
        phase_allocation, \
        charger_state, \
        matched, \
        limits, \
        cfg); \
    } while (0)
