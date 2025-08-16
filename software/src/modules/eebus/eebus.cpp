/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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
#include "eebus.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "ship.h"
#include <TFJson.h>

void EEBus::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
        {"key_id", Config::Int(-1, -1, MAX_CERT_ID)},
    }), [this](Config &update, ConfigSource source) -> String {
        return "";
    }};

    state = Config::Object({
        {"ski", Config::Str("", 0, 64)},
    });

    ship.pre_setup();
}

void EEBus::setup()
{
    api.restorePersistentConfig("eebus/config", &config);

    initialized = true;
}

void EEBus::register_urls()
{
    api.addPersistentConfig("eebus/config", &config);
    api.addState("eebus/state",             &state);

api.addCommand("eebus/discover_devices", Config::Null(), {}, [this](String &/*errmsg*/) {
        ship.scan_skis();
    }, true);
    server.on("/eebus/discovered_devices", HTTP_GET, [this](WebServerRequest request) {
        StringBuilder sb;
        
        ship.print_skis(&sb);
        
        return request.send(200, "application/json", sb.getPtr());
    });

    ship.setup();
}