/* esp32-brick
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

extern API api;
extern WebServer server;
extern TaskScheduler task_scheduler;

static char recv_buf[4096] = {0};
static StaticJsonDocument<4096> json_buf;

Http::Http()
{
    api.registerBackend(this);
}

void Http::setup()
{

}

void Http::register_urls()
{

}

void Http::loop()
{

}

void Http::addCommand(const CommandRegistration &reg)
{
    server.on((String("/") + reg.path).c_str(), HTTP_PUT, [reg](WebServerRequest request) {
        String reason = api.getCommandBlockedReason(reg.path);
        if (reason != "") {
            request.send(400, "text/plain", reason.c_str());
            return;
        }

        //TODO: Use streamed parsing
        int bytes_written = request.receive(recv_buf, 4096);
        if (bytes_written == -1) {
            //buffer was not large enough
            request.send(413);
            return;
        } else if (bytes_written <= 0) {
            logger.printfln("Failed to receive command payload: error code %d", bytes_written);
            request.send(400);
        }

        //json_buf.clear(); // happens implicitly in deserializeJson
        DeserializationError error = deserializeJson(json_buf, recv_buf, bytes_written);
        if(error) {
            logger.printfln("Failed to parse command payload: %s", error.c_str());
            request.send(400);
            return;
        }
        JsonVariant json = json_buf.as<JsonVariant>();
        String message = reg.config->update_from_json(json);

        if (message == "") {
            task_scheduler.scheduleOnce((String("notify command update for ") + reg.path).c_str(), [reg](){reg.callback();}, 0);
            request.send(200, "text/html", "");
        } else {
            request.send(400, "text/html", message.c_str());
        }
    });
}

void Http::addState(const StateRegistration &reg)
{
    server.on((String("/") + reg.path).c_str(), HTTP_GET, [reg](WebServerRequest request) {
        String response = reg.config->to_string_except(reg.keys_to_censor);
        request.send(200, "application/json; charset=utf-8", response.c_str());
    });
}

void Http::pushStateUpdate(String payload, String path)
{

}

void Http::wifiAvailable()
{

}
