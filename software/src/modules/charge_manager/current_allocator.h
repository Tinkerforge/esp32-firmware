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
