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
#include <vector>

#include "module.h"
#include "modules/api/api.h"
#include "web_sockets.h"
#include "string_builder.h"

class WS final : public IModule, public IAPIBackend
{
public:
    WS() : web_sockets() {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    // IAPIBackend implementation
    void addCommand(size_t commandIdx, const CommandRegistration &reg) override;
    void addState(size_t stateIdx, const StateRegistration &reg) override;
    void addResponse(size_t responseIdx, const ResponseRegistration &reg) override;
    bool pushStateUpdate(size_t stateIdx, const String &payload, const String &path) override;
    bool pushRawStateUpdate(const String &payload, const String &path) override;
    WantsStateUpdate wantsStateUpdate(size_t stateIdx) override;

    bool pushStateUpdateBegin(StringBuilder *sb, size_t stateIdx, size_t payload_len, const char *path, ssize_t path_len = -1);
    bool pushStateUpdateEnd(StringBuilder *sb);
    bool pushRawStateUpdateBegin(StringBuilder *sb, size_t payload_len, const char *path, ssize_t path_len = -1);
    bool pushRawStateUpdateEnd(StringBuilder *sb);

    WebSockets web_sockets;
};
