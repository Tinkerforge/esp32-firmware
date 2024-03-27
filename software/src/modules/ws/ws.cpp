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

        auto buffer = heap_alloc_array<char>(buf_size);
        size_t buf_used = 0;
        if (!buffer) {
            // TODO: Technically we'd have to log the heap size of the heap that was used for heap_alloc_array.
            // However if the allocation fails we probably used the DRAM.
            multi_heap_info_t dram_info;
            heap_caps_get_info(&dram_info,  MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            logger.printfln("ws: Not enough memory to send initial state. %u > %u (%u)", buf_size, dram_info.largest_free_block, dram_info.total_free_bytes);
            client.close_HTTPThread();
            return;
        }

        int i = 0;
        bool done = false;
        char *buf = buffer.get();
        while (!done) {
            auto result = task_scheduler.await([&i, &done, &buf, &buf_used, buf_size]() {
                for (; i < api.states.size(); ++i) {
                    auto &reg = api.states[i];

                    auto prefix = "{\"topic\":\"";
                    auto prefix_len = strlen(prefix);

                    auto path = reg.path;
                    auto path_len = reg.path_len;

                    auto infix = "\",\"payload\":";
                    auto infix_len = strlen(infix);

                    auto config_len = reg.config->string_length();

                    auto suffix = "}\n";
                    auto suffix_len = strlen(suffix);

                    int req = prefix_len + path_len + infix_len + config_len + suffix_len + 1; // +1 for the second \n
                    if ((buf_size - buf_used) < req) {
                        done = false;
                        if (req > buf_size)
                            logger.printfln("API %s exceeds max WS buffer size! Required %u buf_size %u", path, req, buf_size);
                        return;
                    }

                    memcpy(buf + buf_used, prefix, prefix_len);
                    buf_used += prefix_len;

                    memcpy(buf + buf_used, path, path_len);
                    buf_used += path_len;

                    memcpy(buf + buf_used, infix, infix_len);
                    buf_used += infix_len;

                    // We don't have to null-terminate the buffer because we know buf_used and the buffer will be handled as bytes.
                    // So we only need suffix_len - 1 bytes here:
                    // ArduinoJSON writes a \0 but we will immediately overwrite it with the first byte of the suffix.
                    // However this could be the last API state, in this case we need one byte more for the second \n.
                    buf_used += reg.config->to_string_except(reg.keys_to_censor, reg.keys_to_censor_len, buf + buf_used, buf_size - buf_used - suffix_len);

                    memcpy(buf + buf_used, suffix, suffix_len);
                    buf_used += suffix_len;
                }
                done = true;
            });

            if (result == TaskScheduler::AwaitResult::Done) {
                if (buf_used == 0) {
                    client.close_HTTPThread();
                    return;
                }

                if (done) {
                    buf[buf_used] = '\n';
                    buf_used += 1;
                }

                if (!client.sendOwnedNoFreeBlocking_HTTPThread(buf, buf_used))
                    return;

                buf_used = 0;
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

// returns true on success
bool WS::pushStateUpdate(size_t stateIdx, const String &payload, const String &path)
{
    if (!web_sockets.haveActiveClient()) {
        return true;
    }

    StringBuilder sb;
    size_t payload_len = payload.length();

    if (!pushStateUpdateBegin(&sb, stateIdx, payload_len, path.c_str(), path.length())) {
        return false;
    }

    sb.puts(payload.c_str(), payload_len);

    return pushStateUpdateEnd(&sb);
}

// returns true on success
bool WS::pushRawStateUpdate(const String &payload, const String &path)
{
    return pushStateUpdate(0, payload, path);
}

// returns true if it is okay to call pushStateUpdateEnd
bool WS::pushStateUpdateBegin(StringBuilder *sb, size_t stateIdx, size_t payload_len, const char *path, ssize_t path_len)
{
    if (!web_sockets.haveActiveClient()) {
        return false;
    }

    if (path_len < 0) {
        path_len = strlen(path);
    }

    if (!sb->setCapacity(prefix_len + path_len + infix_len + payload_len + suffix_len)) {
        return false;
    }

    sb->puts(prefix, prefix_len);
    sb->puts(path, path_len);
    sb->puts(infix, infix_len);

    return true;
}

// returns true on success
bool WS::pushStateUpdateEnd(StringBuilder* sb)
{
    sb->puts(suffix, suffix_len);

    size_t len = sb->getLength();
    char *buf = sb->take().release();

    return web_sockets.sendToAllOwned(buf, len);
}

// returns true if it is okay to call pushRawStateUpdateEnd
bool WS::pushRawStateUpdateBegin(StringBuilder *sb, size_t payload_len, const char *path, ssize_t path_len)
{
    return pushStateUpdateBegin(sb, 0, payload_len, path, path_len);
}

// returns true on success
bool WS::pushRawStateUpdateEnd(StringBuilder* sb)
{
    return pushStateUpdateEnd(sb);
}

IAPIBackend::WantsStateUpdate WS::wantsStateUpdate(size_t stateIdx)
{
    return web_sockets.haveActiveClient() ?
           IAPIBackend::WantsStateUpdate::AsString :
           IAPIBackend::WantsStateUpdate::No;
}
