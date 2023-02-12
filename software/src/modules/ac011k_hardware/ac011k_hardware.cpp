/* en+ hardware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C)      2021 Birger Schmidt <bs-warp@netgaroo.com>
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

#include "ac011k_hardware.h"

#include <Arduino.h>
#include "api.h"
#include "tools.h"
#include "task_scheduler.h"

extern TaskScheduler task_scheduler;

#define GREEN_LED 25
#define BLUE_LED 33
//#define BUTTON T9
#define BUTTON 32

extern uint32_t uid_numeric;
extern char uid[7];
extern char passphrase[20];
extern int8_t blue_led_pin;
extern int8_t green_led_pin;
extern int8_t button_pin;
extern bool factory_reset_requested;
extern API api;
extern char local_uid_str[32];

AC011KHardware::AC011KHardware()
{
}

void AC011KHardware::pre_setup()
{
    config = Config::Object({
        {"verbose_communication", Config::Bool(false)}
    });
    ac011k_hardware = Config::Object({
        {"UID", Config::Str("", 0, 16)},
    });

    meter = Config::Object({
        {"energy_rel",       Config::Float(0.0)},
        {"energy_rel_raw",   Config::Float(0.0)},
        {"energy_rel_delta", Config::Float(0.0)},
        {"energy_abs",       Config::Float(0.0)},
        {"energy_abs_raw",   Config::Float(0.0)},
        {"energy_abs_delta", Config::Float(0.0)},
    });

    if(!api.restorePersistentConfig("ac011k/hardware", &ac011k_hardware)) {
        logger.printfln("AC011K error, could not restore persistent storage ac011k_hardware");
    } else {
        sprintf(local_uid_str, "%s", ac011k_hardware.get("UID")->asEphemeralCStr());
    }
}

void AC011KHardware::persist_config()
{
    ac011k_hardware.get("UID")->updateString(local_uid_str);
    api.writeConfig("ac011k/hardware", &ac011k_hardware);
    logger.printfln("AC011K hardware persistent config stored.");
}

void AC011KHardware::setup()
{
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(BUTTON, INPUT);

    green_led_pin = GREEN_LED;
    blue_led_pin = BLUE_LED;
    button_pin = BUTTON;

    /* task_scheduler.scheduleWithFixedDelay([](){ */
    /*     static bool led_blink_state = false; */
    /*     led_blink_state = !led_blink_state; */
    /*     digitalWrite(BLUE_LED, led_blink_state ? HIGH : LOW); */
    /* }, 0, 1000); */

    api.restorePersistentConfig("ac011k_hardware/config", &config);
    api.restorePersistentConfig("ac011k_hardware/meter", &meter);

    initialized = true;
}

void AC011KHardware::register_urls()
{
    api.addPersistentConfig("ac011k_hardware/config", &config, {}, 1000);
    api.addPersistentConfig("ac011k_hardware/meter", &meter, {}, 1000);

    // this would take care of the energy_rel reset, if it would work
    /* meter.meter.registerResetCallback([this]() { */
    /*     if (!initialized) { */
    /*         return; */
    /*     } */

    /*     meter.values.get("energy_rel")->updateFloat(0); */
    /* }); */
}

void AC011KHardware::loop()
{
    static int last_btn_value = 0;
    static uint32_t last_btn_change = 0;

    //int btn = touchRead(BUTTON);
    bool btn = digitalRead(BUTTON);
    /* if (!factory_reset_requested) { */
    /*     //digitalWrite(GREEN_LED, btn); */
    /*     digitalWrite(BLUE_LED, btn); */
    /* } */

    if (last_btn_change == 0) logger.printfln("Button SW3 changed state to %d.", btn);
    if (btn != last_btn_value) {
        last_btn_change = millis();
        logger.printfln("Button SW3 changed state to %d.", btn);
    }

    last_btn_value = btn;

    /* if (!btn && deadline_elapsed(last_btn_change + 10000)) { */
    /*     logger.printfln("IO0 button was pressed for 10 seconds. Resetting to factory defaults."); */
    /*     last_btn_change = millis(); */
    /*     factory_reset_requested = true; */
    /* } */
}
