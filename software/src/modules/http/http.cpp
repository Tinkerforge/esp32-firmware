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
#include "options.h"

class HTTPChunkedResponse : public IBaseChunkedResponse
{
public:
    HTTPChunkedResponse(WebServerRequest *request): request(request) {}

    void begin(bool success)
    {
        request->beginChunkedResponse_plain(success ? 200 : 400);
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

bool Http::api_handler(WebServerRequest &req, size_t in_uri_len)
{
    // Skip first char when matching APIs: request uri starts with /; the api paths don't.
    const char *in_uri = req.uriCStr() + 1;
    const size_t len = in_uri_len - 1;

    const size_t command_size = api.commands.size();
    for (size_t i = 0; i < command_size; i++)
        if (API::complete_or_prefix_match(in_uri, len, api.commands[i])) {
            const size_t path_len = api.commands[i].get_path_len();

            api_match_data match_data = {
                .type = APIType::COMMAND,
                .idx = i,
                .suffix = in_uri + path_len,
                .suffix_len = len - path_len,
            };

            api_handler_run(req, match_data);
            return true;
        }

    const size_t state_size = api.states.size();
    for (size_t i = 0; i < state_size; i++)
        if (API::complete_or_prefix_match(in_uri, len, api.states[i])) {
            const size_t path_len = api.states[i].get_path_len();

            api_match_data match_data = {
                .type = APIType::STATE,
                .idx = i,
                .suffix = in_uri + path_len,
                .suffix_len = len - path_len,
            };

            api_handler_run(req, match_data);
            return true;
        }

    const size_t response_size = api.responses.size();
    for (size_t i = 0; i < response_size; i++)
        // prefix matches are not supported for responses
        if (API::complete_match(in_uri, len, api.responses[i])) {
            const size_t path_len = api.responses[i].get_path_len();

            api_match_data match_data = {
                .type = APIType::RESPONSE,
                .idx = i,
                .suffix = in_uri + path_len,
                .suffix_len = len - path_len,
            };

            api_handler_run(req, match_data);
            return true;
        }

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
            {"method", Config::Enum(HttpTriggerMethod::GET_POST_PUT,
                                    HttpTriggerMethod::GET,
                                    HttpTriggerMethod::GET_POST_PUT)},
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

static WebServerRequestReturnProtect run_command(WebServerRequest req, size_t cmdidx, const API::SuffixPath &suffix_path = {})
{
    CommandRegistration &reg = api.commands[cmdidx];

    // Check stack usage after increasing buffer size.
    char stack_recv_buf[4096];
    char *recv_buf;
    size_t recv_size;
    std::unique_ptr<char[]> heap_recv_buf;

    const size_t content_length = req.contentLength();

    if (content_length <= ARRAY_SIZE(stack_recv_buf)) {
        recv_buf  = stack_recv_buf;
        recv_size = ARRAY_SIZE(stack_recv_buf);
    } else if (content_length > OPTIONS_API_JSON_MAX_LENGTH()) {
        return req.send_plain(413);
    } else {
        heap_recv_buf = std::unique_ptr<char[]>{new(std::nothrow) char[content_length]};

        if (heap_recv_buf == nullptr) {
            return req.send_plain(413);
        }

        recv_buf  = heap_recv_buf.get();
        recv_size = content_length;
    }

    // TODO: Use streamed parsing
    int bytes_written = req.receive(recv_buf, recv_size);
    if (bytes_written == -1) {
        // buffer was not large enough
        return req.send_plain(413);
    }

    if (bytes_written < 0) {
        logger.printfln("Failed to receive command payload: error code %d", bytes_written);
        return req.send_plain(400);
    }

    String message;
    if (bytes_written == 0 && reg.config->is_null()) {
        message = api.callCommand(reg, nullptr, 0, suffix_path.path.data(), suffix_path.path.size());
    } else {
        message = api.callCommand(reg, recv_buf, bytes_written, suffix_path.path.data(), suffix_path.path.size());
    }

    if (message.isEmpty()) {
        return req.send_plain(200);
    }
    return req.send_plain(400, message);
}

WebServerRequestReturnProtect Http::run_response(WebServerRequest req, size_t respidx)
{
    // Check stack usage after increasing buffer size.
    char recv_buf[2048];

    // TODO: Use streamed parsing
    int bytes_written = req.receive(recv_buf, ARRAY_SIZE(recv_buf));
    if (bytes_written == -1) {
        // buffer was not large enough
        return req.send_plain(413);
    } else if (bytes_written < 0) {
        logger.printfln("Failed to receive response payload: error code %d", bytes_written);
        return req.send_plain(400);
    }

    uint32_t response_owner_id = response_ownership.current();
    HTTPChunkedResponse http_response(&req);
    QueuedChunkedResponse queued_response(&http_response, 500);
    BufferedChunkedResponse buffered_response(&queued_response);

    task_scheduler.scheduleOnce(
        [this, respidx, &recv_buf, bytes_written, &buffered_response, response_owner_id] {
            api.callResponse(api.responses[respidx], recv_buf, bytes_written, &buffered_response, &response_ownership, response_owner_id);
        });

    String error = queued_response.wait();

    if (!error.isEmpty()) {
        logger.printfln("Response processing failed after update: %s (%s %s)", error.c_str(), req.methodString(), req.uriCStr());
    }

    response_ownership.next();
    return WebServerRequestReturnProtect{};
}

WebServerRequestReturnProtect Http::api_handler_get(WebServerRequest &req, const api_match_data &match_data)
{
    switch (match_data.type) {
        case APIType::STATE: {
            API::SuffixPath suffix_path;

            const char *error = API::build_suffix_path(suffix_path, match_data.suffix, match_data.suffix_len);
            if (error != nullptr)
                return req.send_plain(404, error);

            String response;
            uint16_t status_code = 200;
            auto result = task_scheduler.await([&response, &status_code, i=match_data.idx, &suffix_path]() {
                Config *cfg = api.states[i].config;
                cfg = cfg->walk(suffix_path.path.data(), suffix_path.path.size());
                if (cfg == nullptr) {
                    status_code = 404;
                    response = "Path not found";
                    return;
                }

                response = cfg->to_string_except(api.states[i].keys_to_censor, api.states[i].get_keys_to_censor_len());
            });

            if (result == TaskScheduler::AwaitResult::Timeout) {
                status_code = 500;
                response = "Failed to get config. Task timed out.";
            }

            if (status_code == 200) {
                return req.send_json(status_code, response);
            }

            return req.send_plain(status_code, response);
        }
        case APIType::COMMAND: {
            API::SuffixPath suffix_path;

            const char *error = API::build_suffix_path(suffix_path, match_data.suffix, match_data.suffix_len);
            if (error != nullptr)
                return req.send_plain(404, error);

            return run_command(req, match_data.idx, suffix_path);
        }
        case APIType::RESPONSE:
            break;
    }

    // If we reach this point, the url matcher found an API with the req.uri() as path, but we did not.
    // This was probably a raw command or a command that requires a payload. Return 405 - Method not allowed
    return req.send_plain(405, "Request method for this URI is not handled by server");
}

WebServerRequestReturnProtect Http::api_handler_put(WebServerRequest &req, api_match_data &match_data)
{
    switch (match_data.type) {
        case APIType::RESPONSE:
            return run_response(req, match_data.idx);
        case APIType::STATE: {
            // If the matched API type is state, the user wants to call the corresponding _update command.
            // For example `curl warp-abc/foo/bar -X PUT -d {"enable": true}` should be the same as if foo/bar_update was called.

            String uri_update = StringSumHelper(api.states[match_data.idx].path) + "_update";
            size_t command_idx = 0xFFFFFFFF;
            for (size_t a = 0; a < api.commands.size(); a++) {
                if (api.commands[a].get_path_len() == uri_update.length() && memcmp(api.commands[a].path, uri_update.c_str(), uri_update.length()) == 0) {
                    command_idx = a;
                    break;
                }
            }

            // foo/bar exists as a state (as it was matched by the uri matcher), but we could not find foo/bar_update
            if (command_idx == 0xFFFFFFFF)
                return req.send_plain(405, "Request method for this URI is not handled by server");

            // We've found the command that was meant to be called. Update the api index and fall through to normal command handling
            match_data.idx = command_idx;
            [[fallthrough]];
        }
        case APIType::COMMAND: {
            API::SuffixPath suffix_path;

            const char *error = API::build_suffix_path(suffix_path, match_data.suffix, match_data.suffix_len);
            if (error != nullptr)
                return req.send_plain(404, error);

            return run_command(req, match_data.idx, suffix_path);
        }
    }

    // If we reach this point, the url matcher found an API with the req.uri() as path, but we did not.
    // This was probably a raw command or a command that requires a payload. Return 405 - Method not allowed
    return req.send_plain(405, "Request method for this URI is not handled by server");
}

WebServerRequestReturnProtect Http::api_handler_run(WebServerRequest &req, api_match_data &match_data)
{
    const httpd_method_t method = req.method();

    switch (method) {
        case HTTP_GET:
            return api_handler_get(req, match_data);
        case HTTP_PUT:
        case HTTP_POST:
            return api_handler_put(req, match_data);
        default:
            return req.send_plain(405, "Method not allowed for API path");
    }
}

#if MODULE_AUTOMATION_AVAILABLE()
bool Http::has_triggered(const Config *conf, void *data)
{
    const Config *cfg = static_cast<const Config *>(conf->get());
    auto *trigger = (HttpTrigger *)data;

    if (trigger->uri_suffix != cfg->get("url_suffix")->asUnsafeCStr()) {
        trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::WrongUrl);
        return false;
    }

    auto method = trigger->req.method();
    switch (cfg->get("method")->asEnum<HttpTriggerMethod>()) {
        case HttpTriggerMethod::GET:
            if (method != HTTP_GET) {
                trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::WrongMethod);
                return false;
            }

            break;
        case HttpTriggerMethod::POST:
            if (method != HTTP_POST) {
                trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::WrongMethod);
                return false;
            }

            break;
        case HttpTriggerMethod::PUT:
            if (method != HTTP_PUT) {
                trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::WrongMethod);
                return false;
            }

            break;
        case HttpTriggerMethod::POST_PUT:
            if (method != HTTP_POST
             && method != HTTP_PUT) {
                trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::WrongMethod);
                return false;
            }

            break;
        case HttpTriggerMethod::GET_POST_PUT:
            if (method != HTTP_GET
             && method != HTTP_POST
             && method != HTTP_PUT) {
                trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::WrongMethod);
                return false;
            }

            break;
    }

    const auto &expected_payload = cfg->get("payload")->asString();

    if (expected_payload.length() != 0 && expected_payload.length() != trigger->req.contentLength()) {
        trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::WrongPayloadLength);
        return false;
    }

    if (expected_payload.length() == 0) {
        trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::OK);
        return true;
    }

    if (trigger->payload == nullptr) {
        auto size = trigger->req.contentLength();
        trigger->payload = heap_alloc_array<char>(size + 1);

        if (trigger->req.receive(trigger->payload.get(), size) < 0) {
            trigger->payload_receive_failed = true;
            trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::FailedToReceivePayload);
            return false;
        }

        trigger->payload[size] = '\0';
        trigger->payload_receive_failed = false;
    } else if (trigger->payload_receive_failed) {
        trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::FailedToReceivePayload);
        return false;
    }

    if (expected_payload != trigger->payload.get()) {
        trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::WrongPayload);
        return false;
    }

    trigger->most_specific_error = std::max(trigger->most_specific_error, HttpTriggerActionResult::OK);
    return true;
}

WebServerRequestReturnProtect Http::automation_trigger_handler(WebServerRequest req)
{
    const String uri = req.uriCStr() + std::size("/automation_trigger/") - 1; // -1 to exclude null-termination
    HttpTrigger trigger{req, uri, nullptr, false, HttpTriggerActionResult::WrongUrl};

    automation.trigger(AutomationTriggerID::HTTP, &trigger, this);

    switch (trigger.most_specific_error) {
        case HttpTriggerActionResult::OK:
            return req.send_plain(200);
        case HttpTriggerActionResult::WrongUrl:
            return req.send_plain(404, "No automation rule matches this URL");
        case HttpTriggerActionResult::WrongMethod:
            return req.send_plain(405, "No automation rule for this URL matches this method");
        case HttpTriggerActionResult::WrongPayloadLength:
            return req.send_plain(422, "No automation rule for this URL and method matches this payload length");
        case HttpTriggerActionResult::FailedToReceivePayload:
            return req.send_plain(500, "Failed to receive payload");
        case HttpTriggerActionResult::WrongPayload:
            return req.send_plain(422, "No automation rule for this URL and method matches this payload");
        default:
            return req.send_plain(500, "Unspecified automation error");
    }
}
#endif

void Http::register_urls()
{
#if MODULE_AUTOMATION_AVAILABLE()
    server.on("/automation_trigger/*", static_cast<httpd_method_t>(HTTP_ANY), [this](WebServerRequest request) {return automation_trigger_handler(request);});
#endif
}
