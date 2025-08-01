/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include "debug_protocol.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "debug_protocol_backend.h"
#include "tools/string_builder.h"

#include "gcc_warnings.h"

static const char *debug_header_prefix = "\"millis\"";
static const size_t debug_header_prefix_len = strlen(debug_header_prefix);

static constexpr millis_t debug_log_interval        = 40_ms; // One task run takes about 10 ms, so this will result in a total interval length of 50 ms.
static constexpr millis_t debug_keep_alive_interval = 180_s;

void DebugProtocol::register_urls()
{
    // TODO: Make this an API command?
    server.on("/debug_protocol/start", HTTP_GET, [this](WebServerRequest request) {
        StringBuilder sb;
        size_t payload_len = debug_header_prefix_len;

        for (IDebugProtocolBackend *backend : backends) {
            payload_len += 1 + backend->get_debug_header_length(); // +1 for comma
        }

        if (!ws.pushRawStateUpdateBegin(&sb, payload_len, "debug_protocol/header")) {
            return request.send(500);
        }

        sb.puts("\"millis");

        for (IDebugProtocolBackend *backend : backends) {
            sb.putc(',');
            backend->get_debug_header(&sb);
        }

        sb.putc('"');
        ws.pushRawStateUpdateEnd(&sb);

        debug_keep_alive_until = now_us() + debug_keep_alive_interval;

        if (debug_task_id != 0) {
            return request.send(200);
        }

        debug_task_id = task_scheduler.scheduleWithFixedDelay([this]() {
            this->debug_task();
        }, 0_ms, debug_log_interval);

        return request.send(200);
    });

    // TODO: Make this an API command?
    server.on("/debug_protocol/continue", HTTP_GET, [this](WebServerRequest request) {
        debug_keep_alive_until = now_us() + debug_keep_alive_interval;

        return request.send(200);
    });

    // TODO: Make this an API command?
    server.on("/debug_protocol/stop", HTTP_GET, [this](WebServerRequest request) {
        task_scheduler.cancel(this->debug_task_id);
        this->debug_task_id = 0;

        return request.send(200);
    });
}

void DebugProtocol::debug_task()
{
    const micros_t t_now = now_us();

    if (t_now >= debug_keep_alive_until) {
        logger.printfln("Debug protocol creation canceled because no continue call was received for more than 180 seconds.");

        task_scheduler.cancel(debug_task_id);
        debug_task_id = 0;

        return;
    }

    StringBuilder sb;
    size_t payload_len = 1 + 10 + 1; // "%u"

    for (IDebugProtocolBackend *backend : backends) {
        payload_len += 1 + backend->get_debug_line_length(); // +1 for comma
    }

    if (!ws.pushRawStateUpdateBegin(&sb, payload_len, "debug_protocol/line")) {
        return;
    }

    sb.printf("\"%lu", t_now.to<millis_t>().as<uint32_t>());

    for (IDebugProtocolBackend *backend : backends) {
        sb.putc(',');
        backend->get_debug_line(&sb);
    }

    sb.putc('"');
    ws.pushRawStateUpdateEnd(&sb);
}

void DebugProtocol::register_backend(IDebugProtocolBackend *backend)
{
    if (boot_stage != BootStage::SETUP) {
        esp_system_abort("Registering debug protocol backends outside the setup stage is not allowed!");
    }

    backends.push_back(backend);
}
