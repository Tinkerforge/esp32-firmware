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

#include "http.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

class HTTPChunkedResponse : public IBaseChunkedResponse
{
public:
    HTTPChunkedResponse(WebServerRequest *request): request(request) {}

    void begin(bool success)
    {
        request->beginChunkedResponse(success ? 200 : 400);
    }

    void alive()
    {
    }

    void end(String error)
    {
        if (error.isEmpty()) {
            request->endChunkedResponse();
        }
    }

protected:
    bool write_impl(const char *buf, size_t buf_size)
    {
        int result = request->sendChunk(buf, buf_size);

        if (result != ESP_OK) {
            printf("sendChunk failed: %d\n", result);

            return false;
        }

        return true;
    }

private:
    WebServerRequest *request;
};

bool custom_uri_match(const char *ref_uri, const char *in_uri, size_t len)
{
    if (boot_stage <= BootStage::REGISTER_URLS)
        return false;

    if (ref_uri == nullptr || in_uri == nullptr)
        return false;

    // Don't match the API handler.
    if (strncmp_with_same_len("/*", in_uri, len) == 0)
        return false;

    size_t ref_len = strlen(ref_uri);

    if (ref_len == 0)
        return false;

    // Match other wildcard handlers if:
    // - ref_uri is a wildcard handler (it ends in *)
    // - ref_uri is is not the API handler
    // - in_uri is at least as long as the wildcard handler without the *
    // - in_uri and ref_uri are the same up to one char before the *
    if ((ref_uri[ref_len - 1] == '*')
     && (strncmp_with_same_len(ref_uri, "/*", 2) != 0)
     && (strnlen(in_uri, len) >= (ref_len - 1))
     && (strncmp(ref_uri, in_uri, MIN(ref_len - 1, len)) == 0)) {
        return true;
    }

    // Match directly registered URLs.
    if (strncmp_with_same_len(ref_uri, in_uri, len) == 0)
        return true;

    // Only match in_uri with APIs if ref_uri is the registered API handler.
    if (strncmp_with_same_len(ref_uri, "/*", 2) != 0 || len < 2)
        return false;

    // Use + 1 to compare: in_uri starts with /; the api paths don't.
    for (size_t i = 0; i < api.commands.size(); i++)
        if (api.commands[i].path_len == len - 1 && memcmp(api.commands[i].path, in_uri + 1, len - 1) == 0)
            return true;

    for (size_t i = 0; i < api.states.size(); i++)
        if (api.states[i].path_len == len - 1 && memcmp(api.states[i].path, in_uri + 1, len - 1) == 0)
            return true;

    for (size_t i = 0; i < api.responses.size(); i++)
        if (api.responses[i].path_len == len - 1 && memcmp(api.responses[i].path, in_uri + 1, len - 1) == 0)
            return true;

    return false;
}

#if MODULE_AUTOMATION_AVAILABLE()
enum class HttpTriggerMethod : uint8_t {
    GET = 0,
    POST,
    PUT,
    POST_PUT,
    GET_POST_PUT
};
#endif

void Http::pre_setup()
{
    api.registerBackend(this);

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_trigger(
        AutomationTriggerID::HTTP,
        Config::Object({
            {"method", Config::Uint((uint8_t)HttpTriggerMethod::GET_POST_PUT,
                                    (uint8_t)HttpTriggerMethod::GET,
                                    (uint8_t)HttpTriggerMethod::GET_POST_PUT)},
            {"url_suffix", Config::Str("", 0, 32)},
            {"payload", Config::Str("", 0, 32)}
        })
    );
#endif
}

void Http::setup()
{
    initialized = true;
}

static WebServerRequestReturnProtect run_command(WebServerRequest req, size_t cmdidx)
{
    CommandRegistration &reg = api.commands[cmdidx];

    // Check stack usage after increasing buffer size.
    char recv_buf[4096];

    // TODO: Use streamed parsing
    int bytes_written = req.receive(recv_buf, ARRAY_SIZE(recv_buf));
    if (bytes_written == -1) {
        // buffer was not large enough
        return req.send(413);
    }

    if (bytes_written < 0) {
        logger.printfln("Failed to receive command payload: error code %d", bytes_written);
        return req.send(400);
    }

    String message;
    if (bytes_written == 0 && reg.config->is_null()) {
        message = api.callCommand(reg, nullptr, 0);
    } else {
        message = api.callCommand(reg, recv_buf, bytes_written);
    }

    if (message.isEmpty()) {
        return req.send(200);
    }
    return req.send(400, "text/plain; charset=utf-8", message.c_str());
}

WebServerRequestReturnProtect Http::run_response(WebServerRequest req, ResponseRegistration &reg)
{
    // Check stack usage after increasing buffer size.
    char recv_buf[2048];

    // TODO: Use streamed parsing
    int bytes_written = req.receive(recv_buf, ARRAY_SIZE(recv_buf));
    if (bytes_written == -1) {
        // buffer was not large enough
        return req.send(413);
    } else if (bytes_written < 0) {
        logger.printfln("Failed to receive response payload: error code %d", bytes_written);
        return req.send(400);
    }

    uint32_t response_owner_id = response_ownership.current();
    HTTPChunkedResponse http_response(&req);
    QueuedChunkedResponse queued_response(&http_response, 500);
    BufferedChunkedResponse buffered_response(&queued_response);

    task_scheduler.scheduleOnce(
        [this, &reg, &recv_buf, bytes_written, &buffered_response, response_owner_id] {
            api.callResponse(reg, recv_buf, bytes_written, &buffered_response, &response_ownership, response_owner_id);
        },
        0);

    String error = queued_response.wait();

    if (!error.isEmpty()) {
        logger.printfln("Response processing failed after update: %s (%s %s)", error.c_str(), req.methodString(), req.uriCStr());
    }

    response_ownership.next();
    return WebServerRequestReturnProtect{};
}

// Use + 1 to compare: req.uriCStr() starts with /; the api paths don't.
WebServerRequestReturnProtect Http::api_handler_get(WebServerRequest req)
{
    size_t req_uri_len = strlen(req.uriCStr() + 1);

    for (size_t i = 0; i < api.states.size(); i++) {
        if (api.states[i].path_len != req_uri_len || memcmp(api.states[i].path, req.uriCStr() + 1, req_uri_len) != 0)
            continue;

        String response;
        auto result = task_scheduler.await([&response, i]() {
            response = api.states[i].config->to_string_except(api.states[i].keys_to_censor, api.states[i].keys_to_censor_len);
        });
        if (result == TaskScheduler::AwaitResult::Timeout)
            return req.send(500, "text/plain", "Failed to get config. Task timed out.");

        return req.send(200, "application/json; charset=utf-8", response.c_str());
    }

    for (size_t i = 0; i < api.commands.size(); i++)
        if (api.commands[i].path_len == req_uri_len && memcmp(api.commands[i].path, req.uriCStr() + 1, req_uri_len) == 0 && api.commands[i].config->is_null())
            return run_command(req, i);

    // If we reach this point, the url matcher found an API with the req.uri() as path, but we did not.
    // This was probably a raw command or a command that requires a payload. Return 405 - Method not allowed
    return req.send(405, "text/plain", "Request method for this URI is not handled by server");
}

WebServerRequestReturnProtect Http::api_handler_put(WebServerRequest req)
{
    size_t req_uri_len = strlen(req.uriCStr() + 1);

    for (size_t i = 0; i < api.commands.size(); i++)
        if (api.commands[i].path_len == req_uri_len && memcmp(api.commands[i].path, req.uriCStr() + 1, req_uri_len) == 0)
            return run_command(req, i);

    for (size_t i = 0; i < api.responses.size(); i++)
        if (api.responses[i].path_len == req_uri_len && memcmp(api.responses[i].path, req.uriCStr() + 1, req_uri_len) == 0)
            return run_response(req, api.responses[i]);

    if (req.uri().endsWith("_update")) {
        return req.send(405, "text/plain", "Request method for this URI is not handled by server");
    }

    for (size_t i = 0; i < api.states.size(); i++) {
        if (api.states[i].path_len != req_uri_len || memcmp(api.states[i].path, req.uriCStr() + 1, req_uri_len) != 0)
            continue;

        String uri_update = req.uri() + "_update";
        for (size_t a = 0; a < api.commands.size(); a++)
            if (api.commands[a].path_len == uri_update.length() - 1 && memcmp(api.commands[a].path, uri_update.c_str() + 1, uri_update.length() - 1) == 0)
                return run_command(req, a);

        // If we've found the api state that matches req.uriCStr() but did not find a corresponding command with _update,
        // break here because there can't be two states with the same path. We don't have to check the rest.
        break;
    }

    // If we reach this point, the url matcher found an API with the req.uri() as path, but we did not.
    // This was probably a raw command or a command that requires a payload. Return 405 - Method not allowed
    return req.send(405, "text/plain", "Request method for this URI is not handled by server");
}

#if MODULE_AUTOMATION_AVAILABLE()
bool Http::has_triggered(const Config *conf, void *data)
{
    const Config *cfg = static_cast<const Config *>(conf->get());
    auto *trigger = (HttpTrigger *)data;

    auto method = trigger->req.method();
    switch (cfg->get("method")->asEnum<HttpTriggerMethod>()) {
        case HttpTriggerMethod::GET:
            if (method != HTTP_GET) {
                trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::WrongMethod);
                return false;
            }

            break;
        case HttpTriggerMethod::POST:
            if (method != HTTP_POST) {
                trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::WrongMethod);
                return false;
            }

            break;
        case HttpTriggerMethod::PUT:
            if (method != HTTP_PUT) {
                trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::WrongMethod);
                return false;
            }

            break;
        case HttpTriggerMethod::POST_PUT:
            if (method != HTTP_POST
             && method != HTTP_PUT) {
                trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::WrongMethod);
                return false;
            }

            break;
        case HttpTriggerMethod::GET_POST_PUT:
            if (method != HTTP_GET
             && method != HTTP_POST
             && method != HTTP_PUT) {
                trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::WrongMethod);
                return false;
            }

            break;
    }

    if (trigger->uri_suffix != cfg->get("url_suffix")->asEphemeralCStr()) {
        trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::WrongUrl);
        return false;
    }

    const auto &expected_payload = cfg->get("payload")->asString();

    if (expected_payload.length() != 0 && expected_payload.length() != trigger->req.contentLength()) {
        trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::WrongPayloadLength);
        return false;
    }

    if (expected_payload.length() == 0) {
        trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::OK);
        return true;
    }

    if (trigger->payload == nullptr) {
        auto size = trigger->req.contentLength();
        trigger->payload = heap_alloc_array<char>(size + 1);

        if (trigger->req.receive(trigger->payload.get(), size) < 0) {
            trigger->payload_receive_failed = true;
            trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::FailedToReceivePayload);
            return false;
        }

        trigger->payload[size] = '\0';
        trigger->payload_receive_failed = false;
    } else if (trigger->payload_receive_failed) {
        trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::FailedToReceivePayload);
        return false;
    }

    if (expected_payload != trigger->payload.get()) {
        trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::WrongPayload);
        return false;
    }

    trigger->most_specific_error = MAX(trigger->most_specific_error, HttpTriggerActionResult::OK);
    return true;
}
#endif

WebServerRequestReturnProtect Http::automation_trigger_handler(WebServerRequest req)
{
#if MODULE_AUTOMATION_AVAILABLE()
    String uri = req.uri();
    int idx = uri.indexOf("automation_trigger/");
    if (idx < 0) {
        return req.send(405, "text/plain", "Request method for this URI is not handled by server");
    }
    uri = uri.substring(idx + strlen("automation_trigger/"));

    HttpTrigger trigger{req, uri, nullptr, false, HttpTriggerActionResult::WrongUrl};
    automation.trigger(AutomationTriggerID::HTTP, &trigger, this);

    switch (trigger.most_specific_error) {
        case HttpTriggerActionResult::WrongUrl:
            return req.send(404, "text/plain", "No automation rule matches this URL");
        case HttpTriggerActionResult::WrongMethod:
            return req.send(405, "text/plain", "No automation rule for this URL matches this method");
        case HttpTriggerActionResult::WrongPayloadLength:
            return req.send(400, "text/plain", "No automation rule for this URL and method matches this payload length");
        case HttpTriggerActionResult::FailedToReceivePayload:
            return req.send(500, "text/plain", "Failed to receive payload");
        case HttpTriggerActionResult::WrongPayload:
            return req.send(400, "text/plain", "No automation rule for this URL and method matches this payload");
        case HttpTriggerActionResult::OK:
            return req.send(200);
    }

    // The switch above should be exhaustive. Return "normal" 405 if it is not.
#endif
    return req.send(405, "text/plain", "Request method for this URI is not handled by server");
}

void Http::register_urls()
{
#if MODULE_AUTOMATION_AVAILABLE()
    server.on("/automation_trigger/*", HTTP_GET, [this](WebServerRequest request){return automation_trigger_handler(request);});
    server.on("/automation_trigger/*", HTTP_PUT, [this](WebServerRequest request){return automation_trigger_handler(request);});
    server.on("/automation_trigger/*", HTTP_POST, [this](WebServerRequest request){return automation_trigger_handler(request);});
#endif

    server.on_HTTPThread("/*", HTTP_GET, [this](WebServerRequest request){return api_handler_get(request);});
    server.on_HTTPThread("/*", HTTP_PUT, [this](WebServerRequest request){return api_handler_put(request);});
    server.on_HTTPThread("/*", HTTP_POST, [this](WebServerRequest request){return api_handler_put(request);});
}

void Http::addCommand(size_t commandIdx, const CommandRegistration &reg)
{
}

void Http::addState(size_t stateIdx, const StateRegistration &reg)
{
}

void Http::addResponse(size_t responseIdx, const ResponseRegistration &reg)
{
}

bool Http::pushStateUpdate(size_t stateIdx, const String &payload, const String &path)
{
    return true;
}

bool Http::pushRawStateUpdate(const String &payload, const String &path)
{
    return true;
}

IAPIBackend::WantsStateUpdate Http::wantsStateUpdate(size_t stateIdx)
{
    return IAPIBackend::WantsStateUpdate::No;
}
