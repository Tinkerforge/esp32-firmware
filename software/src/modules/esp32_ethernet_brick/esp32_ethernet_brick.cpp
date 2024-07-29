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

#define EVENT_LOG_PREFIX "esp32_eth_brick"

#include "esp32_ethernet_brick.h"

#include <Arduino.h>
#include <driver/i2c.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools.h"
#include "hal_arduino_esp32_ethernet_brick/hal_arduino_esp32_ethernet_brick.h"
#include "bindings/errors.h"

#if TF_LOCAL_ENABLE != 0

#include "bindings/local.h"
// FIXME: for now hardcode DEVICE_IDENTIFIER define here until bindings are ready
//#include "bindings/brick_esp32_ethernet.h"
#define TF_ESP32_ETHERNET_DEVICE_IDENTIFIER 115

#endif

#define GREEN_LED 2
#define BLUE_LED_WARP_ESP32_ETHERNET_BRICK 2
#define BLUE_LED_ESP32_ETHERNET_BRICK 15
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

// Time in seconds.
#ifndef WEM_FACTORY_RESET_WAIT_TIME
#define WEM_FACTORY_RESET_WAIT_TIME 8
#endif

#if BUILD_IS_ENERGY_MANAGER() && MODULE_SYSTEM_AVAILABLE()
static void check_for_factory_reset()
{
    // A factory reset will leave the green LED on, even across a restart. Switch it off here.
    digitalWrite(green_led_pin, false);

    // Check if ethernet clock seems to be enabled.
    // If it's on, a low pin level can be seen, usually already on the first or second poll.
    bool seen_ethernet_clock = false;
    for (uint32_t i = 0; i < 256; i++) {
        if (!digitalRead(BUTTON)) {
            seen_ethernet_clock = true;
            break;
        }
        // Perturb sampling every 32 cycles for an increasing amount of clock cycles
        // to avoid perfectly syncing up to the ethernet clock by accident.
        if (i % 32 == 31) {
            for (uint32_t j = 0; j < (i/32); j++) {
                __asm("nop.n");
            }
        }
    }

    bool blue_led_off = false;
    if (!seen_ethernet_clock) {
        logger.printfln("Checking for factory reset for %d seconds", WEM_FACTORY_RESET_WAIT_TIME);

        // Flash LED for 8 seconds while waiting for button press.
        bool button_pressed = false;
        for (uint32_t i = 0; i < (WEM_FACTORY_RESET_WAIT_TIME*10); i++) {
            digitalWrite(blue_led_pin, i % 4 == 0 ? false : true);
            delay(100); // 8 * 10 * 100ms = 8s
            if (!digitalRead(BUTTON)) {
                button_pressed = true;
                break;
            }
        }
        // Blink LED quickly while the button has to stay pressed for almost 8 seconds.
        if (button_pressed) {
            for (uint32_t i = 0; i < 100; i++) {
                digitalWrite(blue_led_pin, i % 2);
                delay(75); // 100 * 75ms = 7.5s
                if (digitalRead(BUTTON)) {
                    button_pressed = false;
                    break;
                }
            }
            // If the button is still pressed, perform factory reset.
            if (button_pressed) {
                // Perform factory reset, switch on blue LED to show success.
                digitalWrite(blue_led_pin, false);
                system_.factory_reset();
            } else {
                // Factory reset aborted, switch off blue LED.
                blue_led_off = true;
            }
        }
    }
    digitalWrite(blue_led_pin, blue_led_off);
}
#endif

bool ESP32EthernetBrick::initHAL()
{
#if BUILD_IS_WARP3()
    uint8_t ports = 4;
#else
    uint8_t ports = 6;
#endif

    int result = tf_hal_create(&hal, ports);
    if (result != TF_E_OK)
        return false;
    tf_hal_set_timeout(&hal, 100000);
    return true;
}

bool ESP32EthernetBrick::destroyHAL() {
    return tf_hal_destroy(&hal) == TF_E_OK;
}

void ESP32EthernetBrick::pre_init()
{
    button_pin = BUTTON;

#if BUILD_IS_WARP3()
        blue_led_pin = BLUE_LED_WARP_ESP32_ETHERNET_BRICK;
        // green LED is connected directly to 3.3 V
#else
        blue_led_pin = BLUE_LED_ESP32_ETHERNET_BRICK;

        green_led_pin = GREEN_LED;
        pinMode(green_led_pin, OUTPUT);
#endif

    pinMode(blue_led_pin, OUTPUT);
    pinMode(button_pin, INPUT);
}

void ESP32EthernetBrick::setup()
{
    read_efuses(&local_uid_num, local_uid_str, passphrase);
#if BUILD_IS_WARP3()
    logger.printfln("WARP ESP32 Ethernet Brick UID: %s", local_uid_str);
#else
    logger.printfln("ESP32 Ethernet Brick UID: %s", local_uid_str);
#endif

#if BUILD_IS_ENERGY_MANAGER() && MODULE_SYSTEM_AVAILABLE()
    check_for_factory_reset();
#endif

    initHAL();

#if TF_LOCAL_ENABLE != 0
    uint8_t hw_version[3] = {1, 0, 0};
    uint8_t fw_version[3] = {BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH};

    check(tf_local_create(&local, local_uid_str, '0', hw_version, fw_version, TF_ESP32_ETHERNET_DEVICE_IDENTIFIER, &hal), "local create");

    tf_hal_set_local(&hal, &local);
#endif

    task_scheduler.scheduleWithFixedDelay([](){
#if MODULE_WATCHDOG_AVAILABLE()
    static int watchdog_handle = watchdog.add("esp_ethernet_led_blink", "Main thread blocked");
    watchdog.reset(watchdog_handle);
#endif
        led_blink(blue_led_pin, 2000, 1, 0);
    }, 0, 100);

    initialized = true;
}
