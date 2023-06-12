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

void Event::pre_setup()
{
    backendIdx = api.registerBackend(this);
}

void Event::setup()
{
    initialized = true;
}

void Event::registerEvent(const String &path, const std::vector<ConfPath> values, std::function<void(Config *)> callback)
{
    for (size_t i = 0; i < api.states.size(); i++) {
        if (api.states[i].path != path) {
            continue;
        }

        Config *config = api.states[i].config;

        for (auto value : values) {
            bool is_obj = strict_variant::get<const char *>(&value) != nullptr;
            if (is_obj)
                config = (Config *)config->get(*strict_variant::get<const char *>(&value));
            else
                config = (Config *)config->get(*strict_variant::get<uint16_t>(&value));

            if (config == nullptr) {
                if (is_obj)
                    logger.printfln("Value %s in state %s not found", *strict_variant::get<const char *>(&value), path.c_str());
                else
                    logger.printfln("Index %u in state %s not found", *strict_variant::get<uint16_t>(&value), path.c_str());
                return;
            }
        }

        state_updates.push_back({i, config, callback});
        return;
    }

    logger.printfln("State %s not found", path.c_str());
}

void Event::addCommand(size_t commandIdx, const CommandRegistration &reg)
{
}

void Event::addState(size_t stateIdx, const StateRegistration &reg)
{
}

void Event::addRawCommand(size_t rawCommandIdx, const RawCommandRegistration &reg)
{
}

void Event::addResponse(size_t responseIdx, const ResponseRegistration &reg)
{
}

bool Event::pushStateUpdate(size_t stateIdx, const String &payload, const String &path)
{
    for (const StateUpdateRegistration &reg : state_updates) {
        if (reg.stateIdx == stateIdx && reg.config->was_updated(1 << backendIdx)) {
            reg.callback(reg.config);
        }
    }

    return true;
}

void Event::pushRawStateUpdate(const String &payload, const String &path)
{
}

void Event::wifiAvailable()
{
}
