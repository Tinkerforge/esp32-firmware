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

static void hsv2rgb(uint32_t H, /* S, */ float v, uint8_t *R, uint8_t *G, uint8_t *B)
{
    // https://www.codespeedy.com/hsv-to-rgb-in-cpp/
    // H = 360
    // S = 100
    // V = 100
    // s = S / 100
    // v = V / 100
    //    C = (S  /100) * (V/100)
    float C = (100/100) * (v    );
    //    m = (V/100) - C
    float m = (v    ) - C;
    //    X = C * (1 -    |(   (H / 60)mod 2) - 1|)
    float X = C * (1 - abs(fmod(H / 60.0f,  2.0f) - 1));

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

    *R = static_cast<uint8_t>((r + m) * 255);
    *G = static_cast<uint8_t>((g + m) * 255);
    *B = static_cast<uint8_t>((b + m) * 255);
}

EmRgbLed::EmRgbLed()
{
    status = Status::OK;
    have_grid_balance = false;
    hue_balance = 0;

    breathing_pos = 0;
    breathing_step = 1;

    blink_counter = BLINK_PERIOD / 2;
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
    }, 0, LED_TASK_INTERVAL);
}

void EmRgbLed::update_led()
{
    if (status == Status::OK) {
        uint32_t H = have_grid_balance ? hue_balance : HUE_OK;
        float    v = value_breathing[breathing_pos]; // already normalized

        set_led_hsv(H, v);

        breathing_pos += breathing_step;
        if (breathing_pos < 0) {
            breathing_pos = 0;
            breathing_step = 1;
        } else if (breathing_pos >= (ARRAY_SIZE(value_breathing) - 1)) {
            breathing_pos = ARRAY_SIZE(value_breathing) - 1;
            breathing_step = -2;
        }
    } else {
        uint32_t H;
        if (status == Status::Warning)
            H = HUE_WARNING;
        else if (status == Status::Error)
            H = HUE_ERROR;
        else
            H = HUE_UNKNOWN;

        float v = blink_counter / (BLINK_PERIOD / 2);
        blink_counter = (blink_counter + 1) % BLINK_PERIOD;

        set_led_hsv(H, v);
    }
}

void EmRgbLed::set_led_hsv(uint32_t H, /* S, */ float v) {
        uint8_t R, G, B;
        hsv2rgb(H, v, &R, &G, &B);
        energy_manager.set_rgb_led(R, G, B);
}

void EmRgbLed::set_status(Status status)
{
    this->status = status;
}

void EmRgbLed::update_grid_balance(EmRgbLed::GridBalance balance)
{
    this->have_grid_balance = true;

    switch (balance) {
        case EmRgbLed::GridBalance::Export:   hue_balance = HUE_EXPORT;   break;
        case EmRgbLed::GridBalance::Balanced: hue_balance = HUE_BALANCED; break;
        case EmRgbLed::GridBalance::Import:   hue_balance = HUE_IMPORT;   break;
        default:                              hue_balance = HUE_UNKNOWN;
    }
}
