/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

#include "event.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"

#include "tools/malloc.h"
#include "tools/memory.h"

static constexpr const size_t INITIAL_STATE_RESERVE = 224;

void Event::pre_setup()
{
    state_lut = static_cast<decltype(state_lut)>(calloc_32bit_addressed(1, sizeof(StateLUTBlock) + INITIAL_STATE_RESERVE * sizeof(StateUpdateRegistration *)));
    state_lut->states_count = INITIAL_STATE_RESERVE;

    const size_t backendIdx = api.registerBackend(this);
    if (backendIdx >= sizeof(api_backend_flag) * 8) {
        esp_system_abort("backendIdx too large");
    }
    api_backend_flag = 1 << backendIdx;
}

void Event::setup()
{
    initialized = true;

    //task_scheduler.scheduleOnce([this]() {
    //    const StateLUTBlock *lut_block = state_lut;
    //    size_t state_counter = 0;
    //    while (lut_block) {
    //        for (size_t i = 0; i < lut_block->states_count; i++) {
    //            const StateUpdateRegistration *reg = lut_block->registrations[i];
    //            size_t reg_counter = 0;
    //            while (reg) {
    //                reg_counter++;
    //                reg = reg->next_registration;
    //            }
    //            logger.printfln("%zu %3zu %s", reg_counter, state_counter + i, api.states[state_counter + i].path);
    //        }
    //        state_counter += lut_block->states_count;
    //        lut_block = lut_block->next_block;
    //    }
    //});
}

[[gnu::noinline]]
[[gnu::noreturn]]
static void registerEvent_stage_abort(const String &path)
{
    char msg[128];
    snprintf(msg, ARRAY_SIZE(msg), "Attempted to register event for %s before the REGISTER_EVENTS BootStage!", path.c_str());
    esp_system_abort(msg);
}

int64_t Event::registerEvent(const String &path, const std::vector<Config::Key> values, std::function<EventResult(const Config *)> &&callback)
{
    ASSERT_MAIN_THREAD();

    if (boot_stage < BootStage::REGISTER_EVENTS) {
        registerEvent_stage_abort(path);
    }

    size_t stateIdx;
    const Config *state_config = nullptr;

    // Find requested API state.
    const size_t path_length = path.length();
    const size_t api_states_count = api.states.size();
    for (stateIdx = 0; stateIdx < api_states_count; stateIdx++) {
        const StateRegistration &state = api.states[stateIdx];
        if (path_length == state.get_path_len() && path == state.path) {
            state_config = state.config;
            break;
        }
    }

    if (!state_config) {
        logger.printfln("State %s not found", path.c_str());
        return -1;
    }

    // Walk Config::Key to find requested member inside state.
    const Config *state_member = state_config;
    std::unique_ptr<Config::Key []> conf_path;
    size_t conf_path_written = 0;

    const size_t values_count = values.size();
    if (values_count > 0) {
        conf_path = heap_alloc_array<Config::Key>(values_count);

        for (const Config::Key &value : values) {
            const char *const *obj_variant = strict_variant::get<const char *>(&value);
            const bool is_obj = obj_variant != nullptr;
            if (is_obj) {
                if (!address_is_in_rodata(*obj_variant))
                    esp_system_abort("event path key not in flash! Please pass a string literal!");
                state_member = (const Config *)state_member->get(*obj_variant);
            }
            else
                state_member = (const Config *)state_member->get(*strict_variant::get<size_t>(&value));

            if (state_member == nullptr) {
                if (is_obj)
                    logger.printfln("Value %s in state %s not found", *obj_variant, path.c_str());
                else
                    logger.printfln("Index %u in state %s not found", *strict_variant::get<size_t>(&value), path.c_str());
                return -1;
            }

            conf_path[conf_path_written] = value;
            ++conf_path_written;
        }
    }

    const int64_t eventID = ++lastEventID;

    // If the config updated flag is currently set pushStateUpdate will call the callback soon.
    // If not, trigger the callback to make sure it is always called at least once.
    if (!state_member->was_updated(api_backend_flag)) {
        if (callback(state_member) == EventResult::Deregister) {
            // The callback only wanted to be called once, so no need to store it.
            //logger.printfln("Immediately handled %s with stateIdx %zu with eventID %li", path.c_str(), stateIdx, static_cast<int32_t>(eventID));
            return eventID;
        }
    }

    // Store callback.

    // Find registration chain for state.
    StateLUTBlock *state_lut_block = state_lut;
    size_t stateIdx_in_block = stateIdx;
    while (stateIdx_in_block >= state_lut_block->states_count) {
        stateIdx_in_block -= state_lut_block->states_count;

        if (!state_lut_block->next_block) {
            logger.printfln("Have more states than expected: expected %zu, need %zu, total %zu", INITIAL_STATE_RESERVE, stateIdx + 1, api_states_count);

            const size_t additional_states = api_states_count - (stateIdx - stateIdx_in_block);
            state_lut_block->next_block = static_cast<StateLUTBlock *>(calloc_32bit_addressed(sizeof(StateLUTBlock) + additional_states * sizeof(RegistrationBlock *), 1));
            if (!state_lut_block->next_block) {
                logger.printfln("Cannot register event on %s: no memory for StateLUTBlock", path.c_str());
                return -1;
            }

            state_lut_block->next_block->states_count = additional_states;
        }

        state_lut_block = state_lut_block->next_block;
    }

    // Registration chain found, walk to end.
    StateUpdateRegistration **registration_ptr = &state_lut_block->registrations[stateIdx_in_block];
    while (*registration_ptr) {
        registration_ptr = &(*registration_ptr)->next_registration;
    }

    // Find free registration slot.
    StateUpdateRegistration *reg_slot;
    RegistrationBlock **registration_block_ptr = &registration_block_chain;
    for (;;) {
        // Allocate new registration block
        if (!*registration_block_ptr) {
            //logger.printfln("Allocate new registration block");
            *registration_block_ptr = static_cast<RegistrationBlock *>(calloc(1, sizeof(RegistrationBlock)));
            if (!*registration_block_ptr) {
                logger.printfln("Cannot register event on %s: no memory for RegistrationBlock", path.c_str());
                return -1;
            }

            StateUpdateRegistration *regs = (*registration_block_ptr)->regs;
            for (size_t i = 0; i < ARRAY_SIZE((*registration_block_ptr)->regs); i++) {
                regs[i].eventID = -1;
            }
        }

        // Find free registration slot in block.
        StateUpdateRegistration *regs = (*registration_block_ptr)->regs;
        for (size_t i = 0; i < ARRAY_SIZE((*registration_block_ptr)->regs); i++) {
            if (regs[i].eventID == -1) {
                reg_slot = regs + i;
                goto reg_slot_found;
            }
        }

        registration_block_ptr = &(*registration_block_ptr)->next_block;
    }

reg_slot_found:
    reg_slot->eventID           = eventID;
    reg_slot->callback          = std::move(callback);
    reg_slot->conf_path         = std::move(conf_path);
    reg_slot->conf_path_len     = conf_path_written;
    reg_slot->next_registration = nullptr;

    // Append registration to the end of the state's registration chain.
    *registration_ptr = reg_slot;

    return eventID;
}

[[gnu::noinline]]
[[gnu::noreturn]]
static void deregisterEvent_concurrent_abort(int64_t eventID)
{
    char msg[128];
    snprintf(msg, ARRAY_SIZE(msg), "Tried to deregister an event handler for eventID %llu from within an event handler.", eventID);
    esp_system_abort(msg);
}

[[gnu::noinline]]
static void wipe_StateUpdateRegistration(StateUpdateRegistration *reg)
{
    reg->callback = nullptr; // free std::function
    reg->conf_path.reset();  // free std::uniqe_ptr
    reg->conf_path_len = 0;
    reg->next_registration = nullptr;
    reg->eventID = -1; // Mark as free
}

void Event::deregisterEvent(int64_t eventID)
{
    if (eventID == -1)
        return;

    ASSERT_MAIN_THREAD();

    if (state_update_in_progress.load(std::memory_order_consume)) {
        deregisterEvent_concurrent_abort(eventID);
    }

    // Walk state blocks
    StateLUTBlock *state_lut_block = state_lut;
    while (state_lut_block) {
        // Walk states in block
        for (size_t i = 0; i < state_lut_block->states_count; i++) {
            // Walk state registration chain
            StateUpdateRegistration **reg_ptr = &state_lut_block->registrations[i];
            while (*reg_ptr) {
                StateUpdateRegistration *reg = *reg_ptr;
                if (reg->eventID == eventID) {
                    *reg_ptr = reg->next_registration;
                    wipe_StateUpdateRegistration(reg);
                    return;
                }
                reg_ptr = &reg->next_registration;
            }
        }
        state_lut_block = state_lut_block->next_block;
    }

    logger.printfln("Cannot deregister event ID %lli: ID not found", eventID);
}

[[gnu::noinline]]
[[gnu::noreturn]]
static void pushStateUpdate_ConfPath_abort(const char *state_path)
{
    char msg[128];
    snprintf(msg, ARRAY_SIZE(msg), "Reached end of nested config in %s before reaching end of Config::Key", state_path);
    esp_system_abort(msg);
}

bool Event::pushStateUpdate(size_t stateIdx, const String &/*payload*/, const String &path)
{
    state_update_in_progress.store(true, std::memory_order_release);

    StateLUTBlock *state_lut_block = state_lut;
    size_t stateIdx_in_block = stateIdx;

    // Find LUT block with requested state inside.
    while (stateIdx_in_block >= state_lut_block->states_count) {
        stateIdx_in_block -= state_lut_block->states_count;
        state_lut_block = state_lut_block->next_block;

        if (!state_lut_block) {
            // stateIdx is beyond end of LUT -> no registration for that state
            state_update_in_progress.store(false, std::memory_order_release);
            return true;
        }
    }

    // Registration chain for state.
    StateUpdateRegistration **reg_ptr = &state_lut_block->registrations[stateIdx_in_block];
    if (*reg_ptr) {
        const Config *config_root = api.states[stateIdx].config;

        do {
            StateUpdateRegistration *reg = *reg_ptr;
            const Config *config = config_root;

            // Walk registration's Config::Key to find desired Config.
            for (size_t conf_path_idx = 0; conf_path_idx < reg->conf_path_len; ++conf_path_idx) {
                const Config::Key *value = &reg->conf_path[conf_path_idx];
                const char *const *obj_variant = strict_variant::get<const char *>(value);
                if (obj_variant != nullptr) { // is ConfObject
                    config = (const Config *)config->get(*obj_variant);
                } else {
                    config = (const Config *)config->get(*strict_variant::get<size_t>(value));
                }

                if (config == nullptr) {
                    pushStateUpdate_ConfPath_abort(path.c_str());
                }
            }

            // Call callback.
            if (config->was_updated(api_backend_flag) && reg->callback(config) != EventResult::OK) {
                // Remove current registration by changing the pointer that points to it so that it points to the next registration instead.
                *reg_ptr = reg->next_registration;
                wipe_StateUpdateRegistration(reg);
            } else {
                // Keep current registration, move ptr to point to current registration's next pointer.
                reg_ptr = &reg->next_registration;
            }
        } while (*reg_ptr);
    }

    state_update_in_progress.store(false, std::memory_order_release);

    return true;
}

IAPIBackend::WantsStateUpdate Event::wantsStateUpdate(size_t stateIdx)
{
    const StateLUTBlock *state_lut_block = state_lut;
    size_t stateIdx_in_block = stateIdx;

    while (stateIdx_in_block >= state_lut_block->states_count) {
        stateIdx_in_block -= state_lut_block->states_count;
        state_lut_block = state_lut_block->next_block;

        if (!state_lut_block) {
            // stateIdx is beyond end of LUT -> no registration for that state
            return IAPIBackend::WantsStateUpdate::No;
        }
    }

    if (state_lut_block->registrations[stateIdx_in_block]) {
        return IAPIBackend::WantsStateUpdate::AsConfig;
    } else {
        return IAPIBackend::WantsStateUpdate::No;
    }
}
