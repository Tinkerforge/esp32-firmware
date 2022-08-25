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

extern API api;
extern WebServer server;
extern TaskScheduler task_scheduler;

#if MODULE_ESP32_ETHERNET_BRICK_AVAILABLE()
#define RECV_BUF_SIZE 4096
#else
#define RECV_BUF_SIZE 2048
#endif

static char recv_buf[RECV_BUF_SIZE] = {0};

static StaticJsonDocument<RECV_BUF_SIZE> json_buf;


bool custom_uri_match(const char *ref_uri, const char *in_uri, size_t len)
{
    if (!strncmp(ref_uri, in_uri, len))
        return true;

    if (strncmp(ref_uri, "/*", 2) != 0 || len < 2)
        return false;

    for (size_t i = 0; i < api.commands.size(); i++)
    {
        if (strncmp(api.commands[i].path.c_str(), in_uri + 1, len) == 0)
            return true;
    }
    for (size_t i = 0; i < api.states.size(); i++)
    {
        if (strncmp(api.states[i].path.c_str(), in_uri + 1, len) == 0)
            return true;
    }
    for (size_t i = 0; i < api.raw_commands.size(); i++)
    {
        if (strncmp(api.raw_commands[i].path.c_str(), in_uri + 1, len) == 0)
            return true;
    }
    return false;
}

Http::Http()
{
    api.registerBackend(this);
}

void Http::setup()
{
    initialized = true;
}

void api_handler(WebServerRequest req) {
    for (size_t i = 0; i < api.commands.size(); i++) {
        // strcmp is save here: both String::c_str() and req.uriCStr() return null terminated strings.
        // Also we know (because of the custom matcher) that req.uriCStr() contains an API path,
        // we only have to find out which one.
        // Use + 1 to compare: req.uriCStr() starts with /; the api paths don't.
        if (strcmp(api.commands[i].path.c_str(), req.uriCStr() + 1) == 0)
        {
            String reason = api.getCommandBlockedReason(i);
            if (reason != "") {
                req.send(400, "text/plain", reason.c_str());
                return;
            }

            // TODO: Use streamed parsing
            int bytes_written = req.receive(recv_buf, 4096);
            if (bytes_written == -1) {
                // buffer was not large enough
                req.send(413);
                return;
            } else if (bytes_written <= 0) {
                logger.printfln("Failed to receive command payload: error code %d", bytes_written);
                req.send(400);
                return;
            }

            //json_buf.clear(); // happens implicitly in deserializeJson
            DeserializationError error = deserializeJson(json_buf, recv_buf, bytes_written);
            if (error) {
                logger.printfln("Failed to parse command payload: %s", error.c_str());
                req.send(400);
                return;
            }
            JsonVariant json = json_buf.as<JsonVariant>();
            String message = api.commands[i].config->update_from_json(json);

            CommandRegistration reg = api.commands[i];

            if (message == "") {
                task_scheduler.scheduleOnce([reg](){reg.callback();}, 0);
                req.send(200, "text/html", "");
            } else {
                req.send(400, "text/html", message.c_str());
            }
            return;
        }
    }
    for (size_t i = 0; i < api.states.size(); i++)
    {
        // strcmp is save here: both String::c_str() and req.uriCStr() return null terminated strings.
        // Also we know (because of the custom matcher) that req.uriCStr() contains an API path,
        // we only have to find out which one.
        // Use + 1 to compare: req.uriCStr() starts with /; the api paths don't.
        if (strcmp(api.states[i].path.c_str(), req.uriCStr() + 1) == 0)
        {
            String response = api.states[i].config->to_string_except(api.states[i].keys_to_censor);
            req.send(200, "application/json; charset=utf-8", response.c_str());
            return;
        }
    }
    for (size_t i = 0; i < api.raw_commands.size(); i++)
    {
        // strcmp is save here: both String::c_str() and req.uriCStr() return null terminated strings.
        // Also we know (because of the custom matcher) that req.uriCStr() contains an API path,
        // we only have to find out which one.
        // Use + 1 to compare: req.uriCStr() starts with /; the api paths don't.
        if (strcmp(api.raw_commands[i].path.c_str(), req.uriCStr() + 1) == 0)
        {
            int bytes_written = req.receive(recv_buf, RECV_BUF_SIZE);
            if (bytes_written == -1) {
                // buffer was not large enough
                req.send(413);
                return;
            } else if (bytes_written <= 0) {
                logger.printfln("Failed to receive raw command payload: error code %d", bytes_written);
                req.send(400);
                return;
            }

            String message = api.raw_commands[i].callback(recv_buf, bytes_written);
            if (message == "") {
                req.send(200, "text/html", "");
            } else {
                req.send(400, "text/html", message.c_str());
            }
            return;
        }
    }
}

void Http::register_urls()
{
    server.on("/*", HTTP_GET, api_handler);
    server.on("/*", HTTP_PUT, api_handler);
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

bool Http::pushStateUpdate(size_t stateIdx, String payload, String path)
{
    return true;
}

void Http::pushRawStateUpdate(String payload, String path)
{
}

void Http::wifiAvailable()
{
}
