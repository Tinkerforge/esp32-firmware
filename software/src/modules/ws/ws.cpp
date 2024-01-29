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

#include "ws.h"

#include <esp_http_server.h>

#include "api.h"
#include "task_scheduler.h"
#include "web_server.h"
#include "cool_string.h"

void WS::pre_setup()
{
    api.registerBackend(this);
    web_sockets.pre_setup();
}

void WS::setup()
{
    initialized = true;
}

void WS::register_urls()
{
    web_sockets.onConnect_HTTPThread([this](WebSocketsClient client) {
        // Max payload size is 4k.
        // The framing needs 10 + 12 + 3 bytes (with the second \n to mark the end of the API dump)
        // API path lengths should probably fit in the 103 bytes left.
        size_t buf_size = 4096 + 128;

        CoolString to_send;
        if (!to_send.reserve(buf_size)) {
            multi_heap_info_t dram_info;
            heap_caps_get_info(&dram_info,  MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            logger.printfln("ws: Not enough memory to send initial state. %u > %u (%u)", buf_size, dram_info.largest_free_block, dram_info.total_free_bytes);
            client.close_HTTPThread();
            return;
        }

        int i = 0;
        bool done = false;
        while (!done) {
            auto result = task_scheduler.await([&to_send, &i, &done, buf_size](){
                for(; i < api.states.size(); ++i) {
                    auto &reg = api.states[i];
                    int req = 10 + reg.path.length() + 12 + reg.config->string_length() + 2;
                    if ((to_send.getCapacity() - to_send.length()) < req) {
                        done = false;
                        if (req > buf_size)
                            logger.printfln("API %s exceeds max WS buffer size! Required %u buf_size %u", reg.path.c_str(), req, buf_size);
                        return;
                    }

                    to_send.concat("{\"topic\":\"");
                    to_send.concat(reg.path);
                    to_send.concat("\",\"payload\":");

                    size_t length_before = to_send.length();
                    size_t written = reg.config->to_string_except(reg.keys_to_censor, to_send.begin() + to_send.length(), to_send.getCapacity() - to_send.length() - 3);
                    // Truncation should not happen: We've checked above that the payload will fit.
                    // setLength still writes a null terminator.
                    to_send.setLength(length_before + written);

                    to_send.concat("}\n");
                }
                done = true;
            });

            if (result == TaskScheduler::AwaitResult::Done) {
                if (to_send.length() == 0) {
                    client.close_HTTPThread();
                    return;
                }

                if (done)
                    to_send.concat("\n");

                if (!client.sendOwnedNoFreeBlocking_HTTPThread(to_send.begin(), to_send.length()))
                    return;

                to_send.clear();
            }
        }
    });

    web_sockets.start("/ws");

    task_scheduler.scheduleWithFixedDelay([this](){
        char *payload;
        int len = asprintf(&payload, "{\"topic\": \"info/keep_alive\", \"payload\": {\"uptime\": %lu}}\n", millis());
        if (len > 0)
            web_sockets.sendToAllOwned(payload, len);
    }, 1000, 1000);
}

void WS::addCommand(size_t commandIdx, const CommandRegistration &reg)
{
}

void WS::addState(size_t stateIdx, const StateRegistration &reg)
{
}

void WS::addRawCommand(size_t rawCommandIdx, const RawCommandRegistration &reg)
{
}

void WS::addResponse(size_t responseIdx, const ResponseRegistration &reg)
{
}

static const char *prefix = "{\"topic\":\"";
static const char *infix = "\",\"payload\":";
static const char *suffix = "}\n";
static size_t prefix_len = strlen(prefix);
static size_t infix_len = strlen(infix);
static size_t suffix_len = strlen(suffix);

bool WS::pushStateUpdate(size_t stateIdx, const String &payload, const String &path)
{
    if (!web_sockets.haveActiveClient())
        return true;
    //String to_send = String("{\"topic\":\"") + path + String("\",\"payload\":") + payload + String("}\n");
    size_t path_len = path.length();
    size_t payload_len = payload.length();

    size_t to_send_len = prefix_len + path_len + infix_len + payload_len + suffix_len;
    char *to_send = (char *)malloc(to_send_len);
    if (to_send == nullptr)
        return false;

    char *ptr = to_send;
    memcpy(ptr, prefix, prefix_len);
    ptr += prefix_len;

    memcpy(ptr, path.c_str(), path_len);
    ptr += path_len;

    memcpy(ptr, infix, infix_len);
    ptr += infix_len;

    memcpy(ptr, payload.c_str(), payload_len);
    ptr += payload_len;

    memcpy(ptr, suffix, suffix_len);
    ptr += suffix_len;

    return web_sockets.sendToAllOwned(to_send, to_send_len);
}

bool WS::pushRawStateUpdate(const String &payload, const String &path)
{
    return pushStateUpdate(0, payload, path);
}

IAPIBackend::WantsStateUpdate WS::wantsStateUpdate(size_t stateIdx) {
    return web_sockets.haveActiveClient() ?
           IAPIBackend::WantsStateUpdate::AsString :
           IAPIBackend::WantsStateUpdate::No;
}
