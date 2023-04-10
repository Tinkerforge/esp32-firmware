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

#pragma once

#include <vector>

#include "api.h"

struct StateUpdateRegistration {
    size_t stateIdx;
    Config *config;
    std::function<void(Config *)> callback;
};

class Event final : public IAPIBackend
{
public:
    Event() {}
    void pre_setup();
    void setup();

    void addStateUpdate(const String &path, const std::vector<const char *> values, std::function<void(Config *)> callback);

    // IAPIBackend implementation
    void addCommand(size_t commandIdx, const CommandRegistration &reg) override;
    void addState(size_t stateIdx, const StateRegistration &reg) override;
    void addRawCommand(size_t rawCommandIdx, const RawCommandRegistration &reg) override;
    void addResponse(size_t responseIdx, const ResponseRegistration &reg) override;
    bool pushStateUpdate(size_t stateIdx, const String &payload, const String &path) override;
    void pushRawStateUpdate(const String &payload, const String &path) override;
    void wifiAvailable() override;

private:
    size_t backendIdx;
    std::vector<StateUpdateRegistration> state_updates;
};
