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

#include "tutorial_phase_5.h"

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

// Format two digit hexadecimal number
static String num2hex(uint8_t num)
{
    String hex(num, HEX);

    if (hex.length() < 2) {
        hex = String("0") + hex;
    }

    return hex;
}

static void button_state_changed_handler(TF_RGBLEDButton *rgb_led_button, uint8_t button_state, void *user_data)
{
    TutorialPhase5 *tutorial = (TutorialPhase5 *)user_data;

    // Update button state from RGB LED Button Bricklet button-state-changed callback
    tutorial->state.get("button")->updateBool(button_state == TF_RGB_LED_BUTTON_BUTTON_STATE_PRESSED);
}

void TutorialPhase5::pre_setup()
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

    // ConfigRoot object to represent the data to be send to the front-end
    // module. Containing one member "button" representing the button state:
    // true == pressed, false == released.
    state = Config::Object({
        {"button", Config::Bool(false)}
    });
}

void TutorialPhase5::setup()
{
    // Create RGB LED Button Bricklet object. Not specifying a UID or a port
    // name (second parameter is set to nullptr) makes the create function
    // automatically select the first available RGB LED Button Bricklet.
    if (tf_rgb_led_button_create(&rgb_led_button, nullptr, &hal) != TF_E_OK) {
        logger.printfln("No RGB LED Button Bricklet found, disabling Tutorial (Phase 5) module");

        // Could not create RGB LED Button Bricklet object. Return from the
        // setup function without setting initialized to true to indicate
        // that module could not be initialized properly. This also hides
        // the front-end module in the web interface.
        return;
    }

    // Set color of RGB LED Button Bricklet to initial value
    set_bricklet_color(config.get("color")->asString());

    // Register the button_state_changed_handler function as handler for the
    // button-state-changed callback to be notified if the button state of
    // the RGB LED Button Bricklet changes
    tf_rgb_led_button_register_button_state_changed_callback(&rgb_led_button, button_state_changed_handler, this);

    // Get the current button state from the RGB LED Button Bricklet to be
    // used as the initial state value.
    uint8_t button_state;

    if (tf_rgb_led_button_get_button_state(&rgb_led_button, &button_state) != TF_E_OK) {
        logger.printfln("Could not get RGB LED Button Bricklet button state");
    } else {
        state.get("button")->updateBool(button_state == TF_RGB_LED_BUTTON_BUTTON_STATE_PRESSED);
    }

    // Start task with 1 second interval to read back current color
    // value of the RGB LED Button Bricklet. This allows to externally change
    // the color value and make the ESP32 (Ethernet) Brick notice this.
    task_scheduler.scheduleWithFixedDelay([this]() {
        poll_bricklet_color();
    }, 1_s);

    logger.printfln("Tutorial (Phase 5) module initialized");

    initialized = true;
}

void TutorialPhase5::register_urls()
{
    // Add ConfigRoot object to the API manager as a state under the name
    // "tutorial_phase_5/config" to be exposed to the front-end module.
    // The API manager checks the ConfigRoot object for changes once every
    // second by default. If a change is detected, an update is sent.
    api.addState("tutorial_phase_5/config", &config);

    // Add extra ConfigRoot object to the API manager as a command target under
    // the name "tutorial_phase_5/config" to receive updates from the front-end
    // module. If an update is received, the lambda function is called to handle it.
    api.addCommand("tutorial_phase_5/config_update", &config_update, {}, [this](String &/*errmsg*/) {
        String color = config_update.get("color")->asString();

        logger.printfln("Tutorial (Phase 5) module received color update: %s", color.c_str());
        config.get("color")->updateString(color);
        set_bricklet_color(color);
    }, false);

    // Add ConfigRoot object to the API manager as a state under the name
    // "tutorial_phase_5/state" to be exposed to the front-end module.
    // The API manager checks the ConfigRoot object for changes every 250
    // milliseconds when low latency is requested. If a change is detected,
    // an update is sent.
    api.addState("tutorial_phase_5/state", &state, {}, {}, true);
}

void TutorialPhase5::loop()
{
}

void TutorialPhase5::set_bricklet_color(String color)
{
    // Parse a HTML color value #RRGGBB into its red, green and blue parts
    uint8_t red = hex2num(color.substring(1, 3));
    uint8_t green = hex2num(color.substring(3, 5));
    uint8_t blue = hex2num(color.substring(5, 7));

    // Update color of RGB LED Button Bricklet
    if (tf_rgb_led_button_set_color(&rgb_led_button, red, green, blue) != TF_E_OK) {
        logger.printfln("Tutorial (Phase 5) module could not set RGB LED Button Bricklet color");
    }
}

void TutorialPhase5::poll_bricklet_color()
{
    uint8_t red, green, blue;

    // Get the current color from the RGB LED Button Bricklet
    if (tf_rgb_led_button_get_color(&rgb_led_button, &red, &green, &blue) != TF_E_OK) {
        logger.printfln("Could not get RGB LED Button Bricklet color");
        return;
    }

    // Format a HTML color value #RRGGBB from red, green and blue parts
    String color = "#" + num2hex(red) + num2hex(green) + num2hex(blue);

    // Update ConfigRoot object to expose the potentially changed color
    // to the front-end module.
    config.get("color")->updateString(color);
}
