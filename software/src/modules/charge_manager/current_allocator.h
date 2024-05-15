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
