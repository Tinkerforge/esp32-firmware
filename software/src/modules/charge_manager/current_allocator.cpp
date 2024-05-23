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

//#include "gcc_warnings.h"

#define LOCAL_LOG(fmt, ...) if(local_log) local_log += snprintf_u(local_log, cfg->distribution_log_len - (local_log - cfg->distribution_log.get()), "    " fmt "%c", __VA_ARGS__, '\0');

#define TIMEOUT_MS 32000

int filter_chargers(filter_fn filter, int *idx_array, const uint32_t *current_allocation, const uint8_t *phase_allocation, const ChargerState *charger_state, size_t charger_count) {
    int matches = 0;
    for(int i = 0; i < charger_count; ++i) {
        if (!filter(current_allocation[idx_array[i]], phase_allocation[idx_array[i]], &charger_state[idx_array[i]]))
            continue;

        int tmp = idx_array[matches];
        idx_array[matches] = idx_array[i];
        idx_array[i] = tmp;
        ++matches;
    }
    return matches;
}

int allocate_current(
    const CurrentAllocatorConfig *cfg,
    const bool seen_all_chargers,
    const uint32_t pv_excess_current,
    const int phases_available,
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

    uint32_t available = seen_all_chargers ? pv_excess_current : 0;

    // TODO: will be replaced if we control the phases of each charger separately
    bool use_3phase_minimum_current = phases_available >= 3;
    uint32_t minimum_current = use_3phase_minimum_current ? cfg->minimum_current_3p :
                                                            cfg->minimum_current_1p;

    bool print_local_log = false;
    char *local_log = cfg->distribution_log.get();
    if (local_log)
        local_log += snprintf_u(local_log, cfg->distribution_log_len - (local_log - cfg->distribution_log.get()), "Redistributing current%c", '\0');

    bool vehicle_connected = false;

    assert(cfg->charger_count > 0 && cfg->charger_count <= MAX_CONTROLLED_CHARGERS);
    uint32_t current_array[MAX_CONTROLLED_CHARGERS] = {0};
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
            available = 0;
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

    // Sort chargers.
    {
        // Sort the chargers by their minimum supported current,
        // then sort chargers that are already charging before those that
        // want to charge but are not charging yet.
        // Sorting by the minimum current allows us to distribute the current "perfectly"
        // with a single pass over the chargers.
        int chargers_requesting_current = 0;
        int chargers_requesting_current_low_priority = 0;
        for (int i = 0; i < cfg->charger_count; ++i) {
            auto &charger = charger_state[i];
            if (charger.is_charging || charger.wants_to_charge) {
                ++chargers_requesting_current;
            } else if (charger.wants_to_charge_low_priority) {
                ++chargers_requesting_current_low_priority;
            }
        }

        LOCAL_LOG("%d charger%s request%s current. %u mA available.",
                  chargers_requesting_current,
                  chargers_requesting_current == 1 ? "" : "s",
                  chargers_requesting_current == 1 ? "s" : "",
                  available);

        std::stable_sort(idx_array, idx_array + cfg->charger_count, [charger_state](int left, int right) {
            return charger_state[left].requested_current < charger_state[right].requested_current;
        });

        std::stable_sort(idx_array, idx_array + cfg->charger_count, [charger_state](int left, int right) {
            bool left_charging = charger_state[left].is_charging;
            bool right_charging = charger_state[right].is_charging;
            return left_charging && !right_charging;
        });
    }

    // Allocate current to chargers.
    {
        // First allocate the minimum supported current to each charger.
        // Then distribute the rest of the available current to those
        // that received the minimum.
        int chargers_allocated_current_to = 0;

        uint16_t current_to_set = minimum_current;
        for (int i = 0; i < cfg->charger_count; ++i) {
            auto &charger = charger_state[idx_array[i]];

            if (!charger.is_charging && !charger.wants_to_charge) {
                continue;
            }

            uint16_t supported_current = charger.supported_current;
            if (supported_current < current_to_set) {
                LOCAL_LOG("stage 0: Can't unblock %s (%s): It only supports %u mA, but %u mA is the configured minimum current. Handling as low priority charger.",
                          get_charger_name(idx_array[i]),
                          hosts[idx_array[i]],
                          supported_current,
                          current_to_set);
                continue;
            }

            if (available < current_to_set) {
                LOCAL_LOG("stage 0: %u mA left, but %u mA required to unblock another charger. Blocking all following chargers.",available, current_to_set);
                current_to_set = 0;
            }

            if (current_to_set > 0) {
                ++chargers_allocated_current_to;
            }

            current_array[idx_array[i]] = current_to_set;
            available -= current_to_set;

            LOCAL_LOG("stage 0: Calculated target for %s (%s) of %u mA. %u mA left.",
                      get_charger_name(idx_array[i]),
                      hosts[idx_array[i]],
                      current_to_set,
                      available);
        }

        if (available > 0) {
            LOCAL_LOG("stage 0: %u mA still available. Recalculating targets.", available);

            int chargers_reallocated = 0;
            for (int i = 0; i < cfg->charger_count; ++i) {
                if (current_array[idx_array[i]] == 0)
                    continue;

                auto &charger = charger_state[idx_array[i]];
                uint16_t current_per_charger = std::min(32000u, available / (chargers_allocated_current_to - chargers_reallocated));

                uint16_t requested_current = charger.requested_current;

                // If exactly one charger is charging, double the current margin for faster power manager control.
                // If the requested current is exactly 6 A, the car limits itself to less than 6 A - requested_current_margin,
                // so we don't have to increase it.
                if (chargers_allocated_current_to == 1 && requested_current > 6000) {
                    requested_current += cfg->requested_current_margin;
                }

                // Protect against overflow.
                if (requested_current < current_array[idx_array[i]])
                    continue;

                uint16_t max_current = std::min((uint16_t)32000u, requested_current) - current_array[idx_array[i]];
                uint16_t current_to_add = std::min(max_current, current_per_charger);

                ++chargers_reallocated;

                current_array[idx_array[i]] += current_to_add;
                available -= current_to_add;

                LOCAL_LOG("stage 0: Recalculated target for %s (%s) of %u mA. %u mA left.",
                          get_charger_name(idx_array[i]),
                          hosts[idx_array[i]],
                          current_array[idx_array[i]],
                          available);
            }
        }
    }

    // Wake up chargers that already charged once.
    {
        if (available > 0) {
            LOCAL_LOG("stage 0: %u mA still available. Attempting to wake up chargers that already charged their vehicle once.", available);

            uint16_t current_to_set = minimum_current;
            for (int i = 0; i < cfg->charger_count; ++i) {
                auto &charger = charger_state[idx_array[i]];

                uint16_t supported_current = charger.supported_current;

                bool high_prio = charger.is_charging || charger.wants_to_charge;
                bool low_prio = charger.wants_to_charge_low_priority;

                if (!low_prio && !(high_prio && supported_current < current_to_set)) {
                    continue;
                }

                if (supported_current < current_to_set) {
                    LOCAL_LOG("stage 0: %s (%s) only supports %u mA, but %u mA is the configured minimum current. Allocating %u mA.",
                              get_charger_name(idx_array[i]),
                              hosts[idx_array[i]],
                              supported_current,
                              current_to_set,
                              current_to_set);
                }

                if (available < current_to_set) {
                    LOCAL_LOG("stage 0: %u mA left, but %u mA required to unblock another charger. Blocking all following chargers.",available, current_to_set);
                    current_to_set = 0;
                }

                /*if (current_to_set > 0) {
                    ++chargers_allocated_current_to;
                }*/

                current_array[idx_array[i]] = current_to_set;
                available -= current_to_set;

                LOCAL_LOG("stage 0: Calculated target for %s (%s) of %u mA. %u mA left.",
                          get_charger_name(idx_array[i]),
                          hosts[idx_array[i]],
                          current_to_set,
                          available);
            }
        }
    }

    // Apply current limits.
    {
        // First, throttle chargers that have a higher current limit than the calculated one.
        // If no charger has to be throttled, then also unthrottle other chargers. Skip this
        // stage if even one charger needs to be throttled to be sure that the available current
        // is never exceeded.
        bool skip_stage_2 = false;
        for (int i = 0; i < cfg->charger_count; ++i) {
            auto &charger = charger_state[i];
            auto &charger_alloc = charger_allocation_state[i];

            uint16_t current_to_set = current_array[i];

            bool will_throttle = current_to_set < charger_alloc.allocated_current || current_to_set < charger.allowed_current;

            if (!will_throttle) {
                continue;
            }

            LOCAL_LOG("stage 1: Throttled %s (%s) to %d mA.",
                      get_charger_name(i),
                      hosts[i],
                      current_to_set);

            bool current_changed = charger_alloc.allocated_current != current_to_set;
            charger_alloc.allocated_current = current_to_set;
            if (current_changed) {
                print_local_log = true;
                if (charger_alloc.error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                    charger_alloc.last_sent_config = millis();
            }

            // Skip stage 2 to wait for the charger to adapt to the now smaller limit.
            // Some cars are slow to adapt to a new limit. The standard requires them to
            // react in 5 seconds.
            // More correct would be to detect whether the throttled current limit
            // was accepted by the box more than 5 seconds ago (so that we can be sure the timing fits)
            // However this is complicated and waiting a complete cycle (i.e. 10 seconds)
            // works good enough.
            if (!skip_stage_2) {
                LOCAL_LOG("%s", "stage 1: Throttled a charger. Skipping stage 2");
                skip_stage_2 = true;
            }
        }

        if (!skip_stage_2) {
            for (int i = 0; i < cfg->charger_count; ++i) {
                auto &charger = charger_state[i];
                auto &charger_alloc = charger_allocation_state[i];

                uint16_t current_to_set = current_array[i];

                // > instead of >= to only catch chargers that were not already modified in stage 1.
                bool will_not_throttle = current_to_set > charger_alloc.allocated_current || current_to_set > charger.allowed_current;

                if (!will_not_throttle) {
                    continue;
                }

                LOCAL_LOG("stage 2: Unthrottled %s (%s) to %d mA.",
                          get_charger_name(i),
                          hosts[i],
                          current_to_set);

                bool current_changed = charger_alloc.allocated_current != current_to_set;
                charger_alloc.allocated_current = current_to_set;
                if (current_changed) {
                    print_local_log = true;
                    if (charger_alloc.error != CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE)
                        charger_alloc.last_sent_config = millis();
                }
            }
        } else {
            LOCAL_LOG("%s", "Skipping stage 2");
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
    return true;
}
