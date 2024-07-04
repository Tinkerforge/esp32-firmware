/* esp32-firmware
 * Copyright (C) 2022 Matthias Bolte <matthias@tinkerforge.com>
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

#include "tutorial_phase_3.h"

#include <Arduino.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"

void TutorialPhase3::pre_setup()
{
    // ConfigRoot object to represent the data to be send to the front-end
    // module. Containing one member "color" representing the color value
    // in HTML #RRGGBB notation. The string is limited to exactly 7 byte
    // in length.
    config = Config::Object({
        {"color", Config::Str("#FF0000", 7, 7)}
    });

    // Extra ConfigRoot object to represent data updates received from the
    // front-end module. This has the same structure as the first ConfigRoot
    // object. Create it by copying the first one.
    config_update = config;
}

void TutorialPhase3::setup()
{
    logger.printfln("Tutorial (Phase 3) module initialized");

    initialized = true;
}

void TutorialPhase3::register_urls()
{
    // Add ConfigRoot object to the API manager as a state under the name
    // "tutorial_phase_3/config" to be exposed to the front-end module.
    // The API manager checks the ConfigRoot object for changes once every
    // second by default. If a change is detected, an update is sent.
    api.addState("tutorial_phase_3/config", &config);

    // Add extra ConfigRoot object to the API manager as a command target under
    // the name "tutorial_phase_3/config" to receive updates from the front-end
    // module. If an update is received, the lambda function is called to handle it.
    api.addCommand("tutorial_phase_3/config_update", &config_update, {}, [this]() {
        String color = config_update.get("color")->asString();

        logger.printfln("Tutorial (Phase 3) module received color update: %s", color.c_str());
        config.get("color")->updateString(color);
    }, false);
}

void TutorialPhase3::loop()
{
}
