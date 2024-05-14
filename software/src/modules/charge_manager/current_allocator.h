#pragma once

#include "charge_manager.h"

int allocate_current(
        const CurrentAllocatorConfig *cfg,
        const bool seen_all_chargers,
        const uint32_t pv_excess_current,
        const int phases_available,
        const bool cp_disconnect_requested,
        const ChargeManager::ChargerState *charger_state,
        const char * const *hosts,
        const std::function<const char *(uint8_t)> get_charger_name,

        CurrentAllocatorState *ca_state,
        ChargeManager::ChargerAllocationState *charger_allocation_state,
        uint32_t *allocated_current);
