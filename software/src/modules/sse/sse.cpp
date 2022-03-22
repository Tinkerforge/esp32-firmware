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

#include "sse.h"

#include "ESPAsyncWebServer.h"

#include "api.h"
#include "task_scheduler.h"

extern API api;
extern AsyncWebServer server;
extern TaskScheduler task_scheduler;

Sse::Sse() : events("/events")
{
    api.registerBackend(this);
}

void Sse::setup()
{
    events.onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId()) {
            logger.printfln("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
        }

        for (auto &reg : api.states) {
            client->send(reg.config->to_string_except(reg.keys_to_censor).c_str(), reg.path.c_str(), millis(), 1000);
        }
    });

    server.addHandler(&events);
    task_scheduler.scheduleWithFixedDelay([this](){
        events.send("keep-alive", "keep-alive", millis());
    }, 1000, 1000);
}

void Sse::register_urls()
{
}

void Sse::loop()
{
}

void Sse::addCommand(CommandRegistration reg)
{
}

void Sse::addState(StateRegistration reg)
{
}

bool Sse::pushStateUpdate(String payload, String path)
{
    events.send(payload.c_str(), path.c_str(), millis());
    return true;
}

void Sse::wifiAvailable()
{
}
