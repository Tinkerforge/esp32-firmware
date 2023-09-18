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
#include "module_dependencies.h"

#include "api.h"
#include "task_scheduler.h"
#include "web_server.h"

#if MODULE_ESP32_ETHERNET_BRICK_AVAILABLE()
#define RECV_BUF_SIZE 4096
#else
#define RECV_BUF_SIZE 2048
#endif

class HTTPChunkedResponse : public IBaseChunkedResponse
{
public:
    HTTPChunkedResponse(WebServerRequest *request): request(request) {}

    void begin(bool success)
    {
        request->beginChunkedResponse(success ? 200 : 400, "text/plain");
    }

    void alive()
    {

    }

    void end(String error)
    {
        if (error == "") {
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

static char recv_buf[RECV_BUF_SIZE] = {0};

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
    if (!strncmp_with_same_len(ref_uri, in_uri, len))
        return true;

    // Only match in_uri with APIs if ref_uri is the registered API handler.
    if (strncmp_with_same_len(ref_uri, "/*", 2) != 0 || len < 2)
        return false;

    // Use + 1 to compare: in_uri starts with /; the api paths don't.
    for (size_t i = 0; i < api.commands.size(); i++)
        if (strncmp_with_same_len(api.commands[i].path.c_str(), in_uri + 1, len - 1) == 0)
            return true;

    for (size_t i = 0; i < api.states.size(); i++)
        if (strncmp_with_same_len(api.states[i].path.c_str(), in_uri + 1, len - 1) == 0)
            return true;

    for (size_t i = 0; i < api.raw_commands.size(); i++)
        if (strncmp_with_same_len(api.raw_commands[i].path.c_str(), in_uri + 1, len - 1) == 0)
            return true;

    for (size_t i = 0; i < api.responses.size(); i++)
        if (strncmp_with_same_len(api.responses[i].path.c_str(), in_uri + 1, len - 1) == 0)
            return true;

    return false;
}

void Http::pre_setup()
{
    api.registerBackend(this);
}

void Http::setup()
{
    initialized = true;
}

static WebServerRequestReturnProtect run_command(WebServerRequest req, size_t cmdidx)
{
    CommandRegistration &reg = api.commands[cmdidx];

    // TODO: Use streamed parsing
    int bytes_written = req.receive(recv_buf, RECV_BUF_SIZE);
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

    if (message == "") {
        return req.send(200, "text/html", "");
    }
    return req.send(400, "text/html", message.c_str());
}

// strcmp is safe here: both String::c_str() and req.uriCStr() return null terminated strings.
// Also we know (because of the custom matcher) that req.uriCStr() contains an API path,
// we only have to find out which one.
// Use + 1 to compare: req.uriCStr() starts with /; the api paths don't.
WebServerRequestReturnProtect Http::api_handler_get(WebServerRequest req)
{
    for (size_t i = 0; i < api.states.size(); i++)
    {
        if (strcmp(api.states[i].path.c_str(), req.uriCStr() + 1) != 0)
            continue;

        String response;
        auto tid = task_scheduler.scheduleOnce([&response, i](){
            response = api.states[i].config->to_string_except(api.states[i].keys_to_censor);
        }, 0);
        if (task_scheduler.await(tid, 10000) == TaskScheduler::AwaitResult::Timeout)
            return req.send(500, "text/html", "Failed to get config. Task timed out.");

        return req.send(200, "application/json; charset=utf-8", response.c_str());
    }

    for (size_t i = 0; i < api.commands.size(); i++)
        if (strcmp(api.commands[i].path.c_str(), req.uriCStr() + 1) == 0 && api.commands[i].config->is_null())
            return run_command(req, i);

    // If we reach this point, the url matcher found an API with the req.uri() as path, but we did not.
    // This was probably a raw command or a command that requires a payload. Return 405 - Method not allowed
    return req.send(405, "text/html", "Request method for this URI is not handled by server");
}

WebServerRequestReturnProtect Http::api_handler_put(WebServerRequest req) {
    for (size_t i = 0; i < api.commands.size(); i++)
        if (strcmp(api.commands[i].path.c_str(), req.uriCStr() + 1) == 0)
            return run_command(req, i);

    for (size_t i = 0; i < api.raw_commands.size(); i++)
    {
        if (strcmp(api.raw_commands[i].path.c_str(), req.uriCStr() + 1) != 0)
            continue;

        // TODO: Use streamed parsing
        int bytes_written = req.receive(recv_buf, RECV_BUF_SIZE);
        if (bytes_written == -1) {
            // buffer was not large enough
            return req.send(413);
        } else if (bytes_written <= 0) {
            logger.printfln("Failed to receive raw command payload: error code %d", bytes_written);
            return req.send(400);
        }

        String message = api.raw_commands[i].callback(recv_buf, bytes_written);
        if (message == "") {
            return req.send(200, "text/html", "");
        }
        return req.send(400, "text/html", message.c_str());
    }

    for (size_t i = 0; i < api.responses.size(); i++)
    {
        if (strcmp(api.responses[i].path.c_str(), req.uriCStr() + 1) != 0)
            continue;

        // TODO: Use streamed parsing
        int bytes_written = req.receive(recv_buf, RECV_BUF_SIZE);
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
            [this, i, bytes_written, &buffered_response, response_owner_id]{
                api.callResponse(api.responses[i], recv_buf, bytes_written, &buffered_response, &response_ownership, response_owner_id);
            },
            0);

        String error = queued_response.wait();

        if (error != "") {
            logger.printfln("Response processing failed after update: %s (%s %s)", error.c_str(), req.methodString(), req.uriCStr());
        }

        response_ownership.next();
        return WebServerRequestReturnProtect{};
    }

    if (req.uri().endsWith("_update")) {
        return req.send(405, "text/html", "Request method for this URI is not handled by server");
    }

    for (size_t i = 0; i < api.states.size(); i++)
    {
        if (strcmp(api.states[i].path.c_str(), req.uriCStr() + 1) != 0)
            continue;

        String uri_update = req.uri() + "_update";
        for (size_t a = 0; a < api.commands.size(); a++)
            if (!strcmp(api.commands[a].path.c_str(), uri_update.c_str() + 1))
                return run_command(req, a);
    }

    // If we reach this point, the url matcher found an API with the req.uri() as path, but we did not.
    // This was probably a raw command or a command that requires a payload. Return 405 - Method not allowed
    return req.send(405, "text/html", "Request method for this URI is not handled by server");
}

void Http::register_urls()
{
    server.on("/*", HTTP_GET, [this](WebServerRequest request){return api_handler_get(request);});
    server.on("/*", HTTP_PUT, [this](WebServerRequest request){return api_handler_put(request);});
    server.on("/*", HTTP_POST, [this](WebServerRequest request){return api_handler_put(request);});
}

void Http::addCommand(size_t commandIdx, const CommandRegistration &reg)
{
}

void Http::addState(size_t stateIdx, const StateRegistration &reg)
{
}

void Http::addRawCommand(size_t rawCommandIdx, const RawCommandRegistration &reg)
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
