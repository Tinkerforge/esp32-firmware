/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include "system.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "./recovery_html.embedded.h"

extern int8_t green_led_pin;

static TaskHandle_t xTaskBuffer;

void blinky(void *arg)
{
    for (;;) {
        digitalWrite(green_led_pin, true);
        delay(200);
        digitalWrite(green_led_pin, false);
        delay(200);
    }
}

void System::factory_reset(bool restart_esp)
{
    if (factory_reset_running)
        return;
    factory_reset_running = true;

    logger.printfln("Starting factory reset");

    if (green_led_pin >= 0)
        xTaskCreate(blinky,
            "fctyrst_blink",
            2048,
            nullptr,
            tskIDLE_PRIORITY,
            &xTaskBuffer);

#if MODULE_EVSE_COMMON_AVAILABLE()
    evse_common.factory_reset();
#endif

    LittleFS.end();
    LittleFS.format();
    if (restart_esp)
        ESP.restart();
}

void System::setup()
{
    initialized = true;
}

void System::register_urls()
{
    server.on_HTTPThread("/recovery", HTTP_GET, [](WebServerRequest req) {
        req.addResponseHeader("Content-Encoding", "gzip");
        req.addResponseHeader("ETag", "dontcachemeplease");
        // Intentionally don't handle the If-None-Match header:
        // This makes sure that a cached version is never used.
        return req.send(200, "text/html; charset=utf-8", recovery_html_data, recovery_html_length);
    });

    api.addCommand("factory_reset", Config::Confirm(), {Config::confirm_key}, [this](String &result) {
        if (!Config::Confirm()->get(Config::ConfirmKey())->asBool()) {
            result = "Factory reset NOT requested";
            return;
        }

        logger.printfln("Factory reset requested");

        task_scheduler.scheduleOnce([this](){
            factory_reset();
        }, 3000);
    }, true);

    api.addCommand("config_reset", Config::Confirm(), {Config::confirm_key}, [this](String &result) {
        if (!Config::Confirm()->get(Config::ConfirmKey())->asBool()) {
            result = "Config reset NOT requested";
            return;
        }

        logger.printfln("Config reset requested");

        task_scheduler.scheduleOnce([](){
#if MODULE_EVSE_COMMON_AVAILABLE()
            evse_common.factory_reset();
#endif

#if MODULE_USERS_AVAILABLE() && MODULE_CHARGE_TRACKER_AVAILABLE()
            for(int i = 0; i < users.config.get("users")->count(); ++i) {
                uint8_t id = users.config.get("users")->get(i)->get("id")->asUint();
                if (id == 0) // skip anonymous user
                    continue;
                if (!charge_tracker.is_user_tracked(id)) {
                    users.rename_user(id, "", "");
                }
            }
#endif

            API::removeAllConfig();
            ESP.restart();
        }, 3000);
    }, true);
}
