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

#include <atomic>
#include <vector>

#include "event_result.h"
#include "module.h"
#include "modules/api/api.h"

struct StateUpdateRegistration {
    int64_t eventID;
    std::function<EventResult(const Config *)> callback;
    std::unique_ptr<Config::Key[]> conf_path;
    size_t conf_path_len;
    StateUpdateRegistration *next_registration;
};

class Event final : public IModule, public IAPIBackend
{
public:
    Event() {}
    void pre_setup() override;
    void setup() override;

    int64_t registerEvent(const String &path, const std::vector<Config::Key> values, std::function<EventResult(const Config *)> &&callback);
    void deregisterEvent(int64_t eventID);

    // IAPIBackend implementation
    bool pushStateUpdate(size_t stateIdx, const String &payload, const String &path) override;
    WantsStateUpdate wantsStateUpdate(size_t stateIdx) override;

private:
    struct RegistrationBlock {
        RegistrationBlock *next_block;
        StateUpdateRegistration regs[16];
    };

    struct StateLUTBlock {
        StateLUTBlock *next_block;
        size_t states_count;
        StateUpdateRegistration *registrations[];
    };

    int64_t lastEventID = -1;
    StateLUTBlock *state_lut = nullptr;
    RegistrationBlock *registration_block_chain = nullptr;
    std::atomic<bool> state_update_in_progress;
    uint8_t api_backend_flag = 0;
};
