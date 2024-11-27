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

#include "tools/memory.h"

void Event::pre_setup()
{
    backendIdx = api.registerBackend(this);
}

void Event::setup()
{
    initialized = true;
}

int64_t Event::registerEvent(const String &path, const std::vector<ConfPath> values, std::function<EventResult(const Config *)> &&callback)
{
    if (boot_stage < BootStage::REGISTER_EVENTS) {
        logger.printfln("Attempted to register event for %s before the REGISTER_EVENTS BootStage!", path.c_str());
    }

    auto api_states = api.states.size();
    for (size_t i = 0; i < api_states; i++) {
        if (path.length() != api.states[i].path_len || path != api.states[i].path) {
            continue;
        }

        Config *config = api.states[i].config;
        Config *ptr = config;

        auto conf_path = values.size() != 0 ? heap_alloc_array<ConfPath>(values.size()) : nullptr;
        size_t conf_path_written = 0;

        for (auto value : values) {
            const char **obj_variant = strict_variant::get<const char *>(&value);
            bool is_obj = obj_variant != nullptr;
            if (is_obj) {
                if (!string_is_in_rodata(*obj_variant))
                    esp_system_abort("event path key not in flash! Please pass a string literal!");
                ptr = (Config *)ptr->get(*obj_variant);
            }
            else
                ptr = (Config *)ptr->get(*strict_variant::get<size_t>(&value));

            if (ptr == nullptr) {
                if (is_obj)
                    logger.printfln("Value %s in state %s not found", *obj_variant, path.c_str());
                else
                    logger.printfln("Index %u in state %s not found", *strict_variant::get<size_t>(&value), path.c_str());
                return -1;
            }

            conf_path[conf_path_written] = value;
            ++conf_path_written;
        }

        int64_t eventID = ++lastEventID;

        bool store_callback = true;

        // If the config updated flag is currently set
        // pushStateUpdate will call the callback soon.
        // If not, trigger the callback to make sure
        // it is always called at least once.
        if (!ptr->was_updated(1 << backendIdx)) {
            if (callback(ptr) == EventResult::Deregister) {
                store_callback = false;
            }
        }

        // Store callback after possibly calling it,
        // because the function object is forwarded to the vector and cannot be used locally afterwards.
        if (store_callback) {
            state_updates.push_back({eventID, i, std::move(callback), std::move(conf_path), conf_path_written});
        }

        return eventID;
    }

    logger.printfln("State %s not found", path.c_str());
    return -1;
}

void Event::deregisterEvent(int64_t eventID)
{
    if (eventID == -1)
        return;

    if (state_update_in_progress.load(std::memory_order_consume)) {
        logger.printfln("BUG: Tried to deregister an event handler for eventID %llu from within an event handler.", eventID);
        return;
    }

    for (auto it = state_updates.begin(); it != state_updates.end(); ++it) {
        if (it->eventID == eventID) {
            state_updates.erase(it);
            return;
        }
    }
}

void Event::addCommand(size_t commandIdx, const CommandRegistration &reg)
{
}

void Event::addState(size_t stateIdx, const StateRegistration &reg)
{
}

void Event::addResponse(size_t responseIdx, const ResponseRegistration &reg)
{
}

bool Event::pushStateUpdate(size_t stateIdx, const String &payload, const String &path)
{
    state_update_in_progress.store(true, std::memory_order_release);

    for (size_t i = 0; i < state_updates.size();) {
        EventResult result = EventResult::OK;

        const auto &reg = state_updates[i];

        if (reg.stateIdx == stateIdx) {
            Config *config = api.states[stateIdx].config;

            for (size_t conf_path_idx = 0; conf_path_idx < reg.conf_path_len; ++conf_path_idx) {
                auto *value = &reg.conf_path[conf_path_idx];
                const char **obj_variant = strict_variant::get<const char *>(value);
                bool is_obj = obj_variant != nullptr;
                if (is_obj)
                    config = (Config *)config->get(*obj_variant);
                else
                    config = (Config *)config->get(*strict_variant::get<size_t>(value));

                if (config == nullptr) {
                    return true;
                }
            }

            if (config->was_updated(1 << backendIdx)) {
                result = reg.callback(config);
            }
        }

        if (result == EventResult::OK)
            ++i;
        else
            state_updates.erase(state_updates.begin() + i);
    }

    state_update_in_progress.store(false, std::memory_order_release);

    return true;
}

bool Event::pushRawStateUpdate(const String &payload, const String &path)
{
    return true;
}

IAPIBackend::WantsStateUpdate Event::wantsStateUpdate(size_t stateIdx)
{
    for (size_t i = 0; i < state_updates.size(); ++i) {
        const auto &reg = state_updates[i];
        if (reg.stateIdx == stateIdx) {
            return IAPIBackend::WantsStateUpdate::AsConfig;
        }
    }
    return IAPIBackend::WantsStateUpdate::No;
}
