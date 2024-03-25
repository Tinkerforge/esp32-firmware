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
#include "module_dependencies.h"
#include "task_scheduler.h"
#include "debug_protocol_backend.h"

#include "gcc_warnings.h"

void DebugProtocol::register_urls()
{
    // terminate header now, all backend registrations must have been done by now
    header += "\"";

    server.on("/debug_protocol/start", HTTP_GET, [this](WebServerRequest request) {
        last_debug_keep_alive = millis();
        check_debug();
        ws.pushRawStateUpdate(header, "debug_protocol/header");
        debug = true;
        return request.send(200);
    });

    server.on("/debug_protocol/continue", HTTP_GET, [this](WebServerRequest request) {
        last_debug_keep_alive = millis();
        return request.send(200);
    });

    server.on("/debug_protocol/stop", HTTP_GET, [this](WebServerRequest request){
        debug = false;
        return request.send(200);
    });
}

void DebugProtocol::loop()
{
    static uint32_t last_debug = 0;
    if (debug && deadline_elapsed(last_debug + 50)) {
        last_debug = millis();

        char buf[32];
        snprintf(buf, sizeof(buf), "\"%u", last_debug);

        String line(buf);

        for (IDebugProtocolBackend *backend : backends) {
            line += "," + backend->get_debug_line();
        }

        ws.pushRawStateUpdate(line + "\"", "debug_protocol/line");
    }
}

void DebugProtocol::register_backend(IDebugProtocolBackend *backend)
{
    if (boot_stage != BootStage::SETUP) {
        esp_system_abort("Registering debug protocol backends outside the setup stage is not allowed!");
    }

    this->header += "," + backend->get_debug_header();
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
