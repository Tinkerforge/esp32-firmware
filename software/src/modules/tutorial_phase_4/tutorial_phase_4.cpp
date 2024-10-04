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

#include "tutorial_phase_4.h"

#include <Arduino.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "bindings/hal_common.h"
#include "bindings/errors.h"

extern TF_HAL hal;

// Parse two digit hexadecimal number
static uint8_t hex2num(String hex)
{
    return strtol(hex.substring(0, 2).c_str(), nullptr, 16);
}

void TutorialPhase4::pre_setup()
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

void TutorialPhase4::setup()
{
    // Create RGB LED Button Bricklet object. Not specifying a UID or a port
    // name (second parameter is set to nullptr) makes the create function
    // automatically select the first available RGB LED Button Bricklet.
    if (tf_rgb_led_button_create(&rgb_led_button, nullptr, &hal) != TF_E_OK) {
        logger.printfln("No RGB LED Button Bricklet found, disabling Tutorial (Phase 4) module");

        // Could not create RGB LED Button Bricklet object. Return from the
        // setup function without setting initialized to true to indicate
        // that module could not be initialized properly. This also hides
        // the front-end module in the web interface.
        return;
    }

    // Set color of RGB LED Button Bricklet to initial value
    set_bricklet_color(config.get("color")->asString());

    logger.printfln("Tutorial (Phase 4) module initialized");

    initialized = true;
}

void TutorialPhase4::register_urls()
{
    // Add ConfigRoot object to the API manager as a state under the name
    // "tutorial_phase_4/config" to be exposed to the front-end module.
    // The API manager checks the ConfigRoot object for changes once every
    // second by default. If a change is detected, an update is sent.
    api.addState("tutorial_phase_4/config", &config);

    // Add extra ConfigRoot object to the API manager as a command target under
    // the name "tutorial_phase_4/config" to receive updates from the front-end
    // module. If an update is received, the lambda function is called to handle it.
    api.addCommand("tutorial_phase_4/config_update", &config_update, {}, [this](String &/*errmsg*/) {
        String color = config_update.get("color")->asString();

        logger.printfln("Tutorial (Phase 4) module received color update: %s", color.c_str());
        config.get("color")->updateString(color);
        set_bricklet_color(color);
    }, false);
}

void TutorialPhase4::loop()
{
}

void TutorialPhase4::set_bricklet_color(String color)
{
    // Parse a HTML color value #RRGGBB into its red, green and blue parts
    uint8_t red = hex2num(color.substring(1, 3));
    uint8_t green = hex2num(color.substring(3, 5));
    uint8_t blue = hex2num(color.substring(5, 7));

    // Update color of RGB LED Button Bricklet
    if (tf_rgb_led_button_set_color(&rgb_led_button, red, green, blue) != TF_E_OK) {
        logger.printfln("Tutorial (Phase 4) module could not set RGB LED Button Bricklet color");
    }
}
