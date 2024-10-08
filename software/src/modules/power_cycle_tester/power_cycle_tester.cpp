/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "power_cycle_tester.h"

#include <string.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "bindings/errors.h"
#include "bindings/bricklet_rotary_poti_v2.h"
#include "bindings/bricklet_rgb_led_v2.h"
#include "bindings/base58.h"
#include "bindings/endian_convert.h"
#include "bindings/errors.h"

extern TF_HAL hal;

TF_RotaryPotiV2 poti;
TF_RGBLEDV2 led;

void PowerCycleTester::setup()
{
    logger.printfln("Waiting for 2 seconds...");
    delay(2000);

    int result = tf_rotary_poti_v2_create(&poti, nullptr, &hal);

    if (result != TF_E_OK) {
        logger.printfln("Failed to initialize Rotary Poti 2.0 Bricklet (%d).", result);
    }

    result = tf_rgb_led_v2_create(&led, nullptr, &hal);

    if (result != TF_E_OK) {
        logger.printfln("Failed to initialize RGB LED Bricklet (%d).", result);
    }

    tf_rotary_poti_v2_set_status_led_config(&poti, 0);
    tf_rgb_led_v2_set_status_led_config(&led, 0);

    initialized = true;
}

void PowerCycleTester::led_green()
{
    tf_rgb_led_v2_set_rgb_value(&led, 0, 255, 0);
}

void PowerCycleTester::led_yellow()
{
    tf_rgb_led_v2_set_rgb_value(&led, 255, 255, 0);
}

void PowerCycleTester::led_red()
{
    tf_rgb_led_v2_set_rgb_value(&led, 255, 0, 0);
}

void PowerCycleTester::trigger_on_poti_pos(POTI_POS poti_pos, uint32_t delay)
{
    int16_t pos = 0;
    if (tf_rotary_poti_v2_get_position(&poti, &pos) != TF_E_OK) {
        logger.printfln("Failed to get poti position");
        return;
    }

    int16_t low = 0, high = 0;

    switch (poti_pos) {
        case POTI_POS::MINUS_150:
            low = -200;
            high = -120;
            break;
        case POTI_POS::MINUS_90:
            low = -120;
            high = -67;
            break;
        case POTI_POS::MINUS_45:
            low = -67;
            high = -23;
            break;
        case POTI_POS::ZERO:
            low = -23;
            high = 23;
            break;
        case POTI_POS::PLUS_45:
            low = 23;
            high = 67;
            break;
        case POTI_POS::PLUS_90:
            low = 67;
            high = 120;
            break;
        case POTI_POS::PLUS_150:
            low = 120;
            high = 200;
            break;
    }

    if (pos >= low && pos <= high) {
        logger.printfln("Triggering power cycle in %u ms.", delay);
        if (delay < 2000)
            tf_rgb_led_v2_set_rgb_value(&led, 0, 255, 0);
        else {
            task_scheduler.scheduleOnce([](){
                tf_rgb_led_v2_set_rgb_value(&led, 0, 255, 0);
            }, millis_t{delay - 2000});
        }
    }
}
