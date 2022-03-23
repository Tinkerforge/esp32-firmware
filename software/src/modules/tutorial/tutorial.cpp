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

#include "tutorial.h"

#include <Arduino.h>

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "bindings/hal_common.h"
#include "bindings/errors.h"

extern API api;
extern EventLog logger;
extern TaskScheduler task_scheduler;
extern TF_HAL hal;

static uint8_t hex2num(String hex)
{
    return strtol(hex.substring(0, 2).c_str(), nullptr, 16);
}

static String num2hex(uint8_t num)
{
    String hex(num, HEX);

    if (hex.length() < 2) {
        hex = String("0") + hex;
    }

    return hex;
}

static void button_state_changed_handler(TF_RGBLEDButton *rgb_led_button, uint8_t state, void *user_data)
{
    Tutorial *tutorial = (Tutorial *)user_data;

    tutorial->tutorial_state.get("button")->updateBool(state == TF_RGB_LED_BUTTON_BUTTON_STATE_PRESSED);
}

Tutorial::Tutorial()
{
}

void Tutorial::setup()
{
    tutorial_config = Config::Object({
        {"color", Config::Str("#000000", 7, 7)}
    });

    tutorial_config_update = Config::Object({
        {"color", Config::Str("#000000", 7, 7)}
    });

    tutorial_state = Config::Object({
        {"button", Config::Bool(false)}
    });

    if (tf_rgb_led_button_create(&rgb_led_button, nullptr, &hal) != TF_E_OK) {
        logger.printfln("No RGB LED Button Bricklet found, disabling tutorial module");
        return;
    }

    tf_rgb_led_button_register_button_state_changed_callback(&rgb_led_button, button_state_changed_handler, this);

    uint8_t state;

    if (tf_rgb_led_button_get_button_state(&rgb_led_button, &state) != TF_E_OK) {
        logger.printfln("Could not get RGB LED Button Bricklet button state");
    } else {
        tutorial_state.get("button")->updateBool(state == TF_RGB_LED_BUTTON_BUTTON_STATE_PRESSED);
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        update_config();
    }, 0, 1000);

    initialized = true;
}

void Tutorial::register_urls()
{
    api.addState("tutorial/config", &tutorial_config, {}, 1000);
    api.addCommand("tutorial/config_update", &tutorial_config_update, {}, [this](){
        String color = tutorial_config_update.get("color")->asString();

        // Assuming a valid HTML color value #RRGGBB, doing no format checking here
        uint8_t red = hex2num(color.substring(1, 3));
        uint8_t green = hex2num(color.substring(3, 5));
        uint8_t blue = hex2num(color.substring(5, 7));

        if (tf_rgb_led_button_set_color(&rgb_led_button, red, green, blue) != TF_E_OK) {
            logger.printfln("Could not set RGB LED Button Bricklet color");
        }
    }, false);

    api.addState("tutorial/state", &tutorial_state, {}, 250);
}

void Tutorial::loop()
{
}

void Tutorial::update_config()
{
    uint8_t red, green, blue;

    if (tf_rgb_led_button_get_color(&rgb_led_button, &red, &green, &blue) != TF_E_OK) {
        logger.printfln("Could not get RGB LED Button Bricklet color");
        return;
    }

    tutorial_config.get("color")->updateString("#" + num2hex(red) + num2hex(green) + num2hex(blue));
}
