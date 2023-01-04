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

#include "api.h"
#include "task_scheduler.h"
#include "web_server.h"
#include "modules.h"

#if MODULE_ESP32_ETHERNET_BRICK_AVAILABLE()
#define RECV_BUF_SIZE 4096
#else
#define RECV_BUF_SIZE 2048
#endif

static char recv_buf[RECV_BUF_SIZE] = {0};

static int strncmp_with_same_len(const char *left, const char *right, size_t right_len) {
    size_t left_len = strlen(left);
    if (left_len != right_len)
        return -1;
    return strncmp(left, right, right_len);
}

bool custom_uri_match(const char *ref_uri, const char *in_uri, size_t len)
{
    // Don't match the API handler.
    if (strncmp_with_same_len("/*", in_uri, len) == 0)
        return false;

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
    CommandRegistration reg = api.commands[cmdidx];

    const String &reason = api.getCommandBlockedReason(cmdidx);
    if (reason != "")
        return req.send(400, "text/plain", reason.c_str());

    // TODO: Use streamed parsing
    int bytes_written = req.receive(recv_buf, 4096);
    if (bytes_written == -1) {
        // buffer was not large enough
        return req.send(413);
    } else if (bytes_written < 0) {
        logger.printfln("Failed to receive command payload: error code %d", bytes_written);
        return req.send(400);
    } else if (bytes_written == 0 && reg.config->is_null()) {
        task_scheduler.scheduleOnce([reg](){reg.callback();}, 0);
        return req.send(200, "text/html", "");
    }

    String message = reg.config->update_from_cstr(recv_buf, bytes_written);

    if (message == "") {
        task_scheduler.scheduleOnce([reg](){reg.callback();}, 0);
        return req.send(200, "text/html", "");
    }
    return req.send(400, "text/html", message.c_str());
}

// strcmp is save here: both String::c_str() and req.uriCStr() return null terminated strings.
// Also we know (because of the custom matcher) that req.uriCStr() contains an API path,
// we only have to find out which one.
// Use + 1 to compare: req.uriCStr() starts with /; the api paths don't.
WebServerRequestReturnProtect api_handler_get(WebServerRequest req)
{
    for (size_t i = 0; i < api.states.size(); i++)
    {
        if (strcmp(api.states[i].path.c_str(), req.uriCStr() + 1) != 0)
            continue;

        String response = api.states[i].config->to_string_except(api.states[i].keys_to_censor);
        return req.send(200, "application/json; charset=utf-8", response.c_str());
    }

    for (size_t i = 0; i < api.commands.size(); i++)
        if (strcmp(api.commands[i].path.c_str(), req.uriCStr() + 1) == 0 && api.commands[i].config->is_null())
            return run_command(req, i);

    // If we reach this point, the url matcher found an API with the req.uri() as path, but we did not.
    // This was probably a raw command or a command that requires a payload. Return 405 - Method not allowed
    return req.send(405, "text/html", "Request method for this URI is not handled by server");
}

WebServerRequestReturnProtect api_handler_put(WebServerRequest req) {
    for (size_t i = 0; i < api.commands.size(); i++)
        if (strcmp(api.commands[i].path.c_str(), req.uriCStr() + 1) == 0)
            return run_command(req, i);

    for (size_t i = 0; i < api.raw_commands.size(); i++)
    {
        if (strcmp(api.raw_commands[i].path.c_str(), req.uriCStr() + 1) != 0)
            continue;

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
    server.on("/*", HTTP_GET, api_handler_get);
    server.on("/*", HTTP_PUT, api_handler_put);
    server.on("/*", HTTP_POST, api_handler_put);
}

void Http::loop()
{
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

bool Http::pushStateUpdate(size_t stateIdx, const String &payload, const String &path)
{
    return true;
}

void Http::pushRawStateUpdate(const String &payload, const String &path)
{
}

void Http::wifiAvailable()
{
}
