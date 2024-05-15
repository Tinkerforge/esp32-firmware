#include "current_allocator.h"

#include "module_dependencies.h"

#include "event_log.h"

// Only for snprintf_u
#include "tools.h"

#include "modules/cm_networking/cm_networking_defs.h"

//#include "gcc_warnings.h"

#define LOCAL_LOG(fmt, ...) if(local_log) local_log += snprintf_u(local_log, cfg->distribution_log_len - (local_log - cfg->distribution_log.get()), "    " fmt "%c", __VA_ARGS__, '\0');

#define TIMEOUT_MS 32000

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

    bool any_charger_blocking_firmware_update = false;

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
                any_charger_blocking_firmware_update = true;
        }

        if (unreachable_evse_found) {
            // Shut down everything.
            available = 0;
            LOCAL_LOG("%s", "stage 0: Unreachable, unreactive or misconfigured EVSE(s) found. Setting available current to 0 mA.");
            result = 2;

            // Any unreachable EVSE will block a firmware update.
            any_charger_blocking_firmware_update = true;
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
                uint16_t current_per_charger = MIN(32000, available / (chargers_allocated_current_to - chargers_reallocated));

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

                uint16_t current_to_add = MIN(requested_current - current_array[idx_array[i]], current_per_charger);

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
    firmware_update.firmware_update_allowed = !any_charger_blocking_firmware_update;
#else
    (void)any_charger_blocking_firmware_update;
#endif

    return result;
}
