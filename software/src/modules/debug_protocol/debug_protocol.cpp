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

#include <Arduino.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "debug_protocol_backend.h"
#include "string_builder.h"

#include "gcc_warnings.h"

static const char *debug_header_prefix = "\"millis\"";
static const size_t debug_header_prefix_len = strlen(debug_header_prefix);

void DebugProtocol::register_urls()
{
    // TODO: Make this an API command?
    server.on("/debug_protocol/start", HTTP_GET, [this](WebServerRequest request) {
        last_debug_keep_alive = millis();
        check_debug();

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

        debug = true;
        return request.send(200);
    });

    // TODO: Make this an API command?
    server.on("/debug_protocol/continue", HTTP_GET, [this](WebServerRequest request) {
        last_debug_keep_alive = millis();
        return request.send(200);
    });

    // TODO: Make this an API command?
    server.on("/debug_protocol/stop", HTTP_GET, [this](WebServerRequest request) {
        debug = false;
        return request.send(200);
    });
}

void DebugProtocol::loop()
{
    static uint32_t last_debug = 0;
    if (debug && deadline_elapsed(last_debug + 50)) {
        last_debug = millis();

        StringBuilder sb;
        size_t payload_len = 1 + 10 + 1; // "%u"

        for (IDebugProtocolBackend *backend : backends) {
            payload_len += 1 + backend->get_debug_line_length(); // +1 for comma
        }

        if (!ws.pushRawStateUpdateBegin(&sb, payload_len, "debug_protocol/line")) {
            return;
        }

        sb.printf("\"%u", last_debug);

        for (IDebugProtocolBackend *backend : backends) {
            sb.putc(',');
            backend->get_debug_line(&sb);
        }

        sb.putc('"');
        ws.pushRawStateUpdateEnd(&sb);
    }
}

void DebugProtocol::register_backend(IDebugProtocolBackend *backend)
{
    if (boot_stage != BootStage::SETUP) {
        esp_system_abort("Registering debug protocol backends outside the setup stage is not allowed!");
    }

    backends.push_back(backend);
}

void DebugProtocol::check_debug()
{
    task_scheduler.scheduleOnce([this](){
        if (deadline_elapsed(last_debug_keep_alive + 60000) && debug) {
            logger.printfln("Debug protocol creation canceled because no continue call was received for more than 60 seconds.");
            debug = false;
        }
        else if (debug) {
            check_debug();
        }
    }, 10000);
}
