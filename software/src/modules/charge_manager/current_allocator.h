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
