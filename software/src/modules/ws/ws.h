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
#include "tools/string_builder.h"

class WS final : public IModule, public IAPIBackend
{
public:
    WS() : web_sockets() {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;
    void pre_reboot() override;

    // IAPIBackend implementation
    bool pushStateUpdate(size_t stateIdx, const String &payload, const String &path) override;
    bool pushRawStateUpdate(const String &payload, const String &path) override;
    WantsStateUpdate wantsStateUpdate(size_t stateIdx) override;

    [[nodiscard("Returns false when failed to allocate sb's buffer to payload_max_len")]]
    bool pushStateUpdateBegin(StringBuilder *sb, size_t stateIdx, size_t payload_max_len, const char *path, size_t path_len);
    [[nodiscard("Returns false when failed to allocate sb's buffer to payload_max_len")]]
    inline bool pushStateUpdateBegin(StringBuilder *sb, size_t stateIdx, size_t payload_max_len, const char *path) { return pushStateUpdateBegin(sb, stateIdx, payload_max_len, path, strlen(path)); }
    bool pushStateUpdateEnd(StringBuilder *sb);

    [[nodiscard("Returns false when failed to allocate sb's buffer to payload_max_len")]]
    bool pushRawStateUpdateBegin(StringBuilder *sb, size_t payload_max_len, const char *path, size_t path_len);
    [[nodiscard("Returns false when failed to allocate sb's buffer to payload_max_len")]]
    inline bool pushRawStateUpdateBegin(StringBuilder *sb, size_t payload_max_len, const char *path) { return pushRawStateUpdateBegin(sb, payload_max_len, path, strlen(path)); }
    bool pushRawStateUpdateEnd(StringBuilder *sb);

    bool haveActiveClient();

    struct FirstPacketCb {
        const char *path;
        size_t path_len;
        size_t max_length;
        std::function<void(StringWriter *sb)> cb;
    };
    CoolArray<FirstPacketCb, 4> init_packet_cbs;

    void addFirstPacketCb(const char * const path, size_t path_len, size_t max_length, std::function<void(StringWriter *sb)> &&cb);


    WebSockets web_sockets;

};
