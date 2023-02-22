/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "em_rgb_led.h"

#include "modules.h"
#include "task_scheduler.h"
#include "tools.h"

EmRgbLed::EmRgbLed()
{
    hue_ok       = 120; // full green
    hue_warning  =  60; // full yellow
    hue_error    =   0; // full red
    hue_import   =  60; // full yellow
    hue_balanced = 240; // full blue
    hue_export   = 120; // full green

    have_grid_balance = false;
    show_grid_balance = false;
    hue = 120;
    hue_balance = 0;

    breathing_pos = 0;
    breathing_step = 1;
}

void EmRgbLed::setup()
{
    int n = ARRAY_SIZE(value_breathing);
    for (int i = 0; i < n; i++) {
        // Create normalized values in range [0-1]
        value_breathing[i] = 1 + ((cosf(M_PI * i / (n - 1)) - 1) / 4);
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        update_led();
    }, 0, 25);
}

void EmRgbLed::update_led()
{
    // https://www.codespeedy.com/hsv-to-rgb-in-cpp/
    // H = 360
    float H = show_grid_balance ? hue_balance : hue;
    // S = 100
    // V = 100
    // s = S / 100
    // v = V / 100
    float v = value_breathing[breathing_pos]; // already normalized
    //    C = (S  /100) * (V/100)
    float C = (100/100) * (v    );
    //    m = (V/100) - C
    float m = (v    ) - C;
    //    X = C * (1 -    |(   (H / 60)mod 2) - 1|)
    float X = C * (1 - abs(fmod(H / 60.0,  2) - 1));

    float r, g, b;
    if (H <= 120) {
        if (H <= 60) {      // 0-60
            r = C;
            g = X;
            b = 0;
        } else {            // 60-120
            r = X;
            g = C;
            b = 0;
        }
    } else {
        if (H <= 240) {
            if (H <= 180) { // 120-180
                r = 0;
                g = C;
                b = X;
            } else {        // 180-240
                r = 0;
                g = X;
                b = C;
            }
        } else {
            if (H <= 300) { // 240-300
                r = X;
                g = 0;
                b = C;
            } else {        // 300-360
                r = C;
                g = 0;
                b = X;
            }
        }
    }

    uint8_t R = static_cast<uint8_t>((r + m) * 255);
    uint8_t G = static_cast<uint8_t>((g + m) * 255);
    uint8_t B = static_cast<uint8_t>((b + m) * 255);

    energy_manager.set_rgb_led(R, G, B);

    breathing_pos += breathing_step;
    if (breathing_pos < 0) {
        breathing_pos = 0;
        breathing_step = 1;
    } else if (breathing_pos >= (ARRAY_SIZE(value_breathing) - 1)) {
        breathing_pos = ARRAY_SIZE(value_breathing) - 1;
        breathing_step = -2;
    }
}

void EmRgbLed::add_status(const String &id, EmRgbLed::Severity severity, EmRgbLed::Priority priority)
{
    //TODO
}

void EmRgbLed::remove_status(const String &id)
{
    //TODO
}

void EmRgbLed::update_grid_balance(EmRgbLed::GridBalance balance)
{
    if (!this->have_grid_balance) {
        this->have_grid_balance = true;
        //TODO update status thing
        this->show_grid_balance = true;
    }

    switch (balance) {
        case EmRgbLed::GridBalance::Export:   hue_balance = hue_export;   break;
        case EmRgbLed::GridBalance::Balanced: hue_balance = hue_balanced; break;
        case EmRgbLed::GridBalance::Import:   hue_balance = hue_import;   break;
        default:                              hue_balance = hue_error;
    }
}

// https://www.geeksforgeeks.org/vectorfront-vectorback-c-stl/?ref=rp
