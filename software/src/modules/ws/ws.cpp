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
#include "generated/module_dependencies.h"
#include "cool_string.h"

#include "options.h"
#include "gcc_warnings.h"

static constexpr const char * const prefix = "{\"topic\":\"";
static constexpr const char * const infix  = "\",\"payload\":";
static constexpr const char * const suffix = "}\n";
static constexpr const size_t prefix_len   = constexpr_strlen(prefix);
static constexpr const size_t infix_len    = constexpr_strlen(infix);
static constexpr const size_t suffix_len   = constexpr_strlen(suffix);

void WS::pre_setup()
{
    api.registerBackend(this);
    web_sockets.pre_setup();
}

void WS::setup()
{
    initialized = true;
}

static bool send_first_packet_payload(StringWriter *sw, const char * path, size_t path_len, size_t payload_max_len, std::function<void(StringWriter *sw)> cb) {
    size_t req = prefix_len + path_len + infix_len + payload_max_len + suffix_len + 1; // +1 for the second \n

    if (sw->getRemainingLength() < req) {
        if (req > sw->getCapacity()) {
            logger.printfln("%s exceeds max WS buffer capacity! Required %u, buffer capacity %u", path, req, sw->getCapacity());
        }

        return false;
    }

    sw->puts(prefix, prefix_len);
    sw->puts(path, path_len);
    sw->puts(infix, infix_len);
    cb(sw);
    sw->puts(suffix, suffix_len);

    return true;
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
        enum class State {
            Api, Callbacks, Done
        };
        State state = State::Api;
        bool send_called = false;

        while (state != State::Done) {
            auto result = task_scheduler.await([this, &i, &state, &sb]() {
                switch (state) {
                    case State::Api:
                        for (; i < api.states.size(); ++i) {
                            auto &reg = api.states[i];

                            bool sent = send_first_packet_payload(
                                &sb,
                                reg.path,
                                reg.get_path_len(),
                                reg.config->string_length(reg.keys_to_censor, reg.get_keys_to_censor_len()),
                                [&reg](StringWriter *sw){reg.config->to_string_except(reg.keys_to_censor, reg.get_keys_to_censor_len(), sw);}
                            );

                            if (!sent){
                                return;
                            }
                        }

                        state = static_cast<State>(to_underlying(state) + 1);
                        i = 0;
                        [[fallthrough]];

                    case State::Callbacks:
                        for (; i < this->init_packet_cbs.size(); ++i) {
                            auto &reg = this->init_packet_cbs[i];

                            bool sent = send_first_packet_payload(
                                &sb,
                                reg.path,
                                reg.path_len,
                                reg.max_length,
                                reg.cb
                            );

                            if (!sent){
                                return;
                            }
                        }

                        state = static_cast<State>(to_underlying(state) + 1);
                        i = 0;
                        [[fallthrough]];

                    case State::Done:
                    default:
                        break;
                }

            });

            if (!result) {
                client->close_HTTPThread();
                return false;
            }

            if (state != State::Done && sb.getLength() == 0) {
                client->close_HTTPThread();
                return false;
            }

            if (state == State::Done) {
                sb.putc('\n');
            }

            WebSocketsClient::Frag f;
            if (send_called) {
                if (state == State::Done) {
                    f = WebSocketsClient::Frag::Last;
                } else {
                    f = WebSocketsClient::Frag::Middle;
                }
            } else {
                if (state == State::Done) {
                    f = WebSocketsClient::Frag::No;
                } else {
                    f = WebSocketsClient::Frag::First;
                }
            }

            if (!client->sendOwnedNoFreeBlocking_HTTPThread(sb.getPtr(), sb.getLength(), HTTPD_WS_TYPE_TEXT, f)) {
                return false;
            }
            send_called = true;

            sb.clear();
        }

        return true;
    });

    task_scheduler.scheduleUncancelable([this](){
        char *payload;
        int payload_len = asprintf(&payload, "{\"topic\":\"info/keep_alive\",\"payload\":{\"uptime\":%llu}}\n", now_us().to<millis_t>().as<uint64_t>());
        if (payload_len > 0)
            web_sockets.sendToAllOwned(payload, static_cast<size_t>(payload_len));
    }, 1_s, 1_s);
}

void WS::register_events()
{
    web_sockets.start("/ws", "/info/ws");
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
bool WS::pushStateUpdateBegin(StringBuilder *sb, size_t stateIdx, size_t payload_max_len, const char *path, size_t path_len)
{
    if (!web_sockets.haveActiveClient()) {
        return false;
    }

    if (!sb->setCapacity(prefix_len + path_len + infix_len + payload_max_len + suffix_len)) {
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

bool WS::pushRawStateUpdateBegin(StringBuilder *sb, size_t payload_max_len, const char *path, size_t path_len)
{
    return pushStateUpdateBegin(sb, 0, payload_max_len, path, path_len);
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

void WS::addFirstPacketCb(const char * const path, size_t path_len, size_t max_length, std::function<void(StringWriter *sb)> &&cb)
{
    this->init_packet_cbs.add({path, path_len, max_length, cb});
}

void WS::pre_reboot() {
    web_sockets.pre_reboot();
}
