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

#include "esp32_brick.h"

#include <Arduino.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "build.h"
#include "hal_arduino_esp32_brick/hal_arduino_esp32_brick.h"

#include "bindings/errors.h"

#if TF_LOCAL_ENABLE != 0

#include "bindings/local.h"
// FIXME: for now hardcode DEVICE_IDENTIFIER define here until bindings are ready
//#include "bindings/brick_esp32.h"
#define TF_ESP32_DEVICE_IDENTIFIER 113

#endif

#define GREEN_LED 4
#define BLUE_LED 32
#define BUTTON 0

TF_HAL hal;
extern uint32_t local_uid_num;
extern char local_uid_str[32];
extern char passphrase[20];
extern int8_t blue_led_pin;
extern int8_t green_led_pin;
extern int8_t button_pin;

#if TF_LOCAL_ENABLE != 0

static TF_Local local;

#endif

bool ESP32Brick::initHAL()
{
    int result = tf_hal_create(&hal);
    if (result != TF_E_OK)
        return false;
    tf_hal_set_timeout(&hal, 100000);
    return true;
}

bool ESP32Brick::destroyHAL() {
    return tf_hal_destroy(&hal) == TF_E_OK;
}

void ESP32Brick::setup()
{
    read_efuses(&local_uid_num, local_uid_str, passphrase);
    logger.printfln("ESP32 Brick UID: %s", local_uid_str);

    initHAL();

#if TF_LOCAL_ENABLE != 0
    uint8_t hw_version[3] = {1, 0, 0};
    uint8_t fw_version[3] = {BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH};

    check(tf_local_create(&local, local_uid_str, '0', hw_version, fw_version, TF_ESP32_DEVICE_IDENTIFIER, &hal), "local create");

    tf_hal_set_local(&hal, &local);
#endif

    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(BUTTON, INPUT);

    green_led_pin = GREEN_LED;
    blue_led_pin = BLUE_LED;
    button_pin = BUTTON;

    task_scheduler.scheduleWithFixedDelay([](){
#if MODULE_WATCHDOG_AVAILABLE()
    static int watchdog_handle = watchdog.add("esp_led_blink", "Main thread blocked");
    watchdog.reset(watchdog_handle);
#endif
        led_blink(BLUE_LED, 2000, 1, 0);
    }, 100_ms);

    initialized = true;
}

void ESP32Brick::loop()
{
#if MODULE_SYSTEM_AVAILABLE()
    static bool last_btn_value = false;
    static uint32_t last_btn_change = 0;

    bool btn = digitalRead(BUTTON);
    digitalWrite(GREEN_LED, btn);

    if (btn != last_btn_value) {
        last_btn_change = millis();
    }

    last_btn_value = btn;

    if (!btn && deadline_elapsed(last_btn_change + 10000)) {
        logger.printfln("IO0 button was pressed for 10 seconds. Resetting to factory defaults.");
        last_btn_change = millis();
        system_.factory_reset();
    }
#endif
}
