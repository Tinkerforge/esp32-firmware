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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "cool_string.h"
#include "options.h"

#include "gcc_warnings.h"

static const char *prefix = "{\"topic\":\"";
static const char *infix = "\",\"payload\":";
static const char *suffix = "}\n";
static size_t prefix_len = strlen(prefix);
static size_t infix_len = strlen(infix);
static size_t suffix_len = strlen(suffix);

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
    // TODO: Merge this and the very similar function print_regs_to_debug_report in api.cpp
    web_sockets.onConnect_HTTPThread([this](WebSocketsClient *client) {
        // Max payload size is OPTIONS_API_JSON_MAX_LENGTH.
        // The framing needs 10 + 12 + 3 bytes (with the second \n to mark the end of the API dump)
        // API path lengths should probably fit in the 103 bytes left.
        size_t buf_size = OPTIONS_API_JSON_MAX_LENGTH() + 128;
        StringBuilder sb;

        if (!sb.setCapacity(buf_size)) {
            // TODO: Technically we'd have to log the heap size of the heap that was used for heap_alloc_array.
            // However if the allocation fails we probably used the DRAM.
            multi_heap_info_t dram_info;
            heap_caps_get_info(&dram_info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            logger.printfln("Not enough memory to send initial state. %u > %u (%u)", buf_size, dram_info.largest_free_block, dram_info.total_free_bytes);
            client->close_HTTPThread();
            return false;
        }

        sb.printf("{\"topic\":\"info/keep_alive\",\"payload\":{\"uptime\":%llu}}\n", now_us().to<millis_t>().as<uint64_t>());

        size_t i = 0;
        bool done = false;

        while (!done) {
            auto result = task_scheduler.await([&i, &done, &sb]() {
                for (; i < api.states.size(); ++i) {
                    auto &reg = api.states[i];
                    auto path = reg.path;
                    auto path_len = reg.get_path_len();
                    auto config_len = reg.config->string_length();
                    size_t req = prefix_len + path_len + infix_len + config_len + suffix_len + 1; // +1 for the second \n

                    if (sb.getRemainingLength() < req) {
                        done = false;

                        if (req > sb.getCapacity()) {
                            logger.printfln("API %s exceeds max WS buffer capacity! Required %u, buffer capacity %u", path, req, sb.getCapacity());
                        }

                        return;
                    }

                    sb.puts(prefix, prefix_len);
                    sb.puts(path, path_len);
                    sb.puts(infix, infix_len);

                    reg.config->to_string_except(reg.keys_to_censor, reg.get_keys_to_censor_len(), &sb);

                    sb.puts(suffix, suffix_len);
                }

                done = true;
            });

            if (result != TaskScheduler::AwaitResult::Done) {
                return false;
            }

            if (!done && sb.getLength() == 0) {
                client->close_HTTPThread();
                return false;
            }

            if (done) {
                sb.putc('\n');
            }

            if (!client->sendOwnedNoFreeBlocking_HTTPThread(sb.getPtr(), sb.getLength())) {
                return false;
            }

            sb.clear();
        }

        return true;
    });

    web_sockets.start("/ws", "/info/ws");

    task_scheduler.scheduleUncancelable([this](){
        char *payload;
        int payload_len = asprintf(&payload, "{\"topic\":\"info/keep_alive\",\"payload\":{\"uptime\":%llu}}\n", now_us().to<millis_t>().as<uint64_t>());
        if (payload_len > 0)
            web_sockets.sendToAllOwned(payload, static_cast<size_t>(payload_len));
    }, 1_s, 1_s);
}

// returns true on success
bool WS::pushStateUpdate(size_t stateIdx, const String &payload, const String &path)
{
    if (!web_sockets.haveActiveClient()) {
        return true;
    }

    StringBuilder sb;

    if (!pushStateUpdateBegin(&sb, stateIdx, payload.length(), path.c_str(), path.length())) {
        return false;
    }

    sb.puts(payload);

    return pushStateUpdateEnd(&sb);
}

// returns true on success
bool WS::pushRawStateUpdate(const String &payload, const String &path)
{
    return pushStateUpdate(0, payload, path);
}

// returns true if it is okay to call pushStateUpdateEnd
bool WS::pushStateUpdateBegin(StringBuilder *sb, size_t stateIdx, size_t payload_len, const char *path, size_t path_len)
{
    if (!web_sockets.haveActiveClient()) {
        return false;
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
bool WS::pushStateUpdateEnd(StringBuilder *sb)
{
    sb->puts(suffix, suffix_len);

    size_t len = sb->getLength();
    char *buf = sb->take();

    return web_sockets.sendToAllOwned(buf, len);
}

// returns true if it is okay to call pushRawStateUpdateEnd
bool WS::pushRawStateUpdateBegin(StringBuilder *sb, size_t payload_len, const char *path, size_t path_len)
{
    return pushStateUpdateBegin(sb, 0, payload_len, path, path_len);
}

// returns true on success
bool WS::pushRawStateUpdateEnd(StringBuilder *sb)
{
    return pushStateUpdateEnd(sb);
}

IAPIBackend::WantsStateUpdate WS::wantsStateUpdate(size_t stateIdx)
{
    return web_sockets.haveActiveClient() ?
           IAPIBackend::WantsStateUpdate::AsString :
           IAPIBackend::WantsStateUpdate::No;
}

bool WS::haveActiveClient()
{
    return web_sockets.haveActiveClient();
}

void WS::pre_reboot() {
    web_sockets.pre_reboot();
}
