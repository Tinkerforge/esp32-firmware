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
#include "modules/web_server/web_server.h"
#include "tools.h"
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

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

    bool api_handler(WebServerRequest &req, size_t in_uri_len);

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
    WebServerRequestReturnProtect automation_trigger_handler(WebServerRequest req);
#endif

private:
    enum class APIType {STATE, COMMAND, RESPONSE};

    struct api_match_data {
        APIType type;
        size_t idx;
        const char *suffix;
        size_t suffix_len;
    };

    WebServerRequestReturnProtect api_handler_get(WebServerRequest &req, const api_match_data &match_data);
    WebServerRequestReturnProtect api_handler_put(WebServerRequest &req, api_match_data &match_data);
    WebServerRequestReturnProtect api_handler_run(WebServerRequest &req, api_match_data &match_data);

    WebServerRequestReturnProtect run_response(WebServerRequest req, size_t respidx);

    Ownership response_ownership;
};

#include "module_available_end.h"
