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

#include "config.h"

#include "module.h"
#include "api.h"
#include "tools.h"

bool custom_uri_match(const char *ref_uri, const char *in_uri, size_t len);

class Http final : public IAPIBackend
{
public:
    Http(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    // IAPIBackend implementation
    void addCommand(size_t commandIdx, const CommandRegistration &reg) override;
    void addState(size_t stateIdx, const StateRegistration &reg) override;
    void addRawCommand(size_t rawCommandIdx, const RawCommandRegistration &reg) override;
    void addResponse(size_t responseIdx, const ResponseRegistration &reg) override;
    bool pushStateUpdate(size_t stateIdx, const String &payload, const String &path) override;
    bool pushRawStateUpdate(const String &payload, const String &path) override;
    WantsStateUpdate wantsStateUpdate(size_t stateIdx) override;
    WebServerRequestReturnProtect api_handler_get(WebServerRequest req);
    WebServerRequestReturnProtect api_handler_put(WebServerRequest req);

    WebServerRequestReturnProtect automation_trigger_handler(WebServerRequest req);

    struct HttpTrigger {
        WebServerRequest &req;
        const String &uri_suffix;
        std::unique_ptr<char[]> payload;
        bool payload_receive_failed;
    };

    enum class HttpTriggerActionResult {
        WrongUrl,
        WrongMethod,
        WrongPayloadLength,
        FailedToReceivePayload,
        WrongPayload,
        OK,
    };
    HttpTriggerActionResult trigger_action(Config *trigger_config, void *user_data);

    Ownership response_ownership;
};
