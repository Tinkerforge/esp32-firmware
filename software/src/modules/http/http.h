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

#include "module.h"
#include "config.h"
#include "modules/api/api.h"
#include "tools.h"
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

bool custom_uri_match(const char *ref_uri, const char *in_uri, size_t len);

class Http final : public IModule,
                   public IAPIBackend
#if MODULE_AUTOMATION_AVAILABLE()
                 , public IAutomationBackend
#endif
{
public:
    Http(){}
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
    WebServerRequestReturnProtect api_handler_get(WebServerRequest req);
    WebServerRequestReturnProtect api_handler_put(WebServerRequest req);

    WebServerRequestReturnProtect automation_trigger_handler(WebServerRequest req);

#if MODULE_AUTOMATION_AVAILABLE()
    enum class HttpTriggerActionResult {
        WrongUrl,
        WrongMethod,
        WrongPayloadLength,
        FailedToReceivePayload,
        WrongPayload,
        OK,
    };

    struct HttpTrigger {
        WebServerRequest &req;
        const String &uri_suffix;
        std::unique_ptr<char[]> payload;
        bool payload_receive_failed;
        HttpTriggerActionResult most_specific_error;
    };

    bool has_triggered(const Config *conf, void *data) override;
#endif

private:
    WebServerRequestReturnProtect run_response(WebServerRequest req, ResponseRegistration &reg);

    Ownership response_ownership;
};
