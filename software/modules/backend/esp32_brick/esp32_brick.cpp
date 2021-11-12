/* esp32-brick
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

#include "esp32_brick.h"
#include "Arduino.h"

#include "tools.h"
#include "hal_arduino_esp32/hal_arduino_esp32.h"
#include "task_scheduler.h"

extern TaskScheduler task_scheduler;

#define GREEN_LED 4
#define BLUE_LED 32
#define BUTTON 0

TF_HalContext hal;
extern uint32_t uid_numeric;
extern char uid[7];
extern char passphrase[20];
extern int8_t blue_led_pin;
extern int8_t green_led_pin;
extern int8_t button_pin;
extern bool factory_reset_requested;

ESP32Brick::ESP32Brick()
{

}

void ESP32Brick::setup()
{
    check(tf_hal_create(&hal), "hal create");
    tf_hal_set_timeout(&hal, 100000);

    read_efuses(&uid_numeric, uid, passphrase);

    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(BUTTON, INPUT);

    green_led_pin = GREEN_LED;
    blue_led_pin = BLUE_LED;
    button_pin = BUTTON;

    task_scheduler.scheduleWithFixedDelay("led_blink", [](){
        static bool led_blink_state = false;
        led_blink_state = !led_blink_state;
        digitalWrite(BLUE_LED, led_blink_state ? HIGH : LOW);
    }, 0, 1000);
}

void ESP32Brick::register_urls()
{

}

void ESP32Brick::loop()
{
    static bool last_btn_value = false;
    static uint32_t last_btn_change = 0;

    bool btn = digitalRead(BUTTON);
    if (!factory_reset_requested)
        digitalWrite(GREEN_LED, btn);

    if(btn != last_btn_value) {
        last_btn_change = millis();
    }

    last_btn_value = btn;

    if(!btn && deadline_elapsed(last_btn_change + 10000)) {
        logger.printfln("IO0 button was pressed for 10 seconds. Resetting to factory defaults.");
        last_btn_change = millis();
        factory_reset_requested = true;
    }
}
