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

#pragma once

#include <functional>

#include "charge_manager_private.h"

struct cm_state_v1;
struct cm_state_v2;

int allocate_current(
        const CurrentAllocatorConfig *cfg,
        const bool seen_all_chargers,
        const uint32_t pv_excess_current,
        const int phases_available,
        const bool cp_disconnect_requested,
        const ChargerState *charger_state,
        const char * const *hosts,
        const std::function<const char *(uint8_t)> get_charger_name,
        const std::function<void (uint8_t)> clear_dns_cache_entry,

        CurrentAllocatorState *ca_state,
        ChargerAllocationState *charger_allocation_state,
        uint32_t *allocated_current);

bool update_from_client_packet(
    uint8_t client_id,
    cm_state_v1 *v1,
    cm_state_v2 *v2,
    const CurrentAllocatorConfig *cfg,
    ChargerState *charger_state,
    ChargerAllocationState *charger_allocation_state,
    const char * const *hosts,
    const std::function<const char *(uint8_t)> get_charger_name);

typedef bool(*filter_fn)(uint32_t /*allocated_current*/, uint8_t /*allocated_phases*/, const ChargerState */*state*/);

int filter_chargers(filter_fn filter, int *idx_array, const uint32_t *current_allocation, const uint8_t *phase_allocation, const ChargerState *charger_state, size_t charger_count);

typedef int(*group_fn)(uint32_t /*allocated_current*/, uint8_t /*allocated_phases*/, const ChargerState */*state*/);

struct CompareInfo {
    uint32_t allocated_current;
    uint8_t allocated_phases;
    const ChargerState *state;
};
typedef bool(*compare_fn)(CompareInfo /*left*/, CompareInfo /*right*/);

void sort_chargers(group_fn group, compare_fn compare, int *idx_array, const uint32_t *current_allocation, const uint8_t *phase_allocation, const ChargerState *charger_state, size_t charger_count);

struct Cost {
    int pv;
    int l1;
    int l2;
    int l3;

    int& operator[](size_t idx) { return *(&pv + idx); }
    const int& operator[](size_t idx) const { return *(&pv + idx); }

    int& operator[](GridPhase p) { return *(&pv + (int)p); }
    const int& operator[](GridPhase p) const { return *(&pv + (int)p); }
};
static_assert(sizeof(Cost) == 4 * sizeof(int), "Unexpected size of Cost");

GridPhase get_phase(PhaseRotation rot, ChargerPhase phase);

Cost get_cost(uint32_t current_to_allocate,
              ChargerPhase phases_to_allocate,
              PhaseRotation rot,
              uint32_t allocated_current,
              ChargerPhase allocated_phases);

struct CurrentLimits {
    uint32_t grid_l1;
    uint32_t grid_l2;
    uint32_t grid_l3;

    uint32_t grid_l1_filtered;
    uint32_t grid_l2_filtered;
    uint32_t grid_l3_filtered;

    uint32_t pv_excess;
    uint32_t pv_excess_filtered;

    uint32_t supply_cable_l1;
    uint32_t supply_cable_l2;
    uint32_t supply_cable_l3;
};

void stage_1(int *idx_array, uint32_t *current_allocation, uint8_t *phase_allocation, CurrentLimits *limits, const ChargerState *charger_state, size_t charger_count, const CurrentAllocatorConfig *cfg, const CurrentAllocatorState *ca_state);
