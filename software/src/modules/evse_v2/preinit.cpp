/* esp32-firmware
 * Copyright (C) 2022-2024 Erik Fleckstein <erik@tinkerforge.com>
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

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <LittleFS.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "bindings/hal_common.h"
#include "bindings/bricklet_evse_v2.h"
#include "tools.h"

#define BUTTON_MIN_PRESS_THRES 10000
#define BUTTON_MAX_PRESS_THRES 30000
#define BUTTON_IS_PRESSED 0xFFFFFFFF
#define FACTORY_RESET_DATA_MAGIC 0x0BADB007; //stage is written over the first 0. so this counts the number of bad boot(-up)s.

extern TF_HAL hal;

extern int8_t blue_led_pin;
extern int8_t green_led_pin;

void evse_v2_button_recovery_handler()
{
#if MODULE_ESP32_BRICK_AVAILABLE()
    auto esp_brick = esp32_brick;
#elif MODULE_ESP32_ETHERNET_BRICK_AVAILABLE()
    auto esp_brick = esp32_ethernet_brick;
#else
    #warning "Using EVSE module without ESP32 Brick or ESP32 Ethernet Brick module. Pre-init will not work!"
    return;
#endif

    if (!esp_brick.initHAL())
        return;

    defer {
        esp_brick.destroyHAL();
    };

    TF_EVSEV2 evse;
    int result = tf_evse_v2_create(&evse, nullptr, &hal);
    if (result != TF_E_OK)
        return;

    defer {
        tf_evse_v2_destroy(&evse);
    };

    uint32_t evse_uptime = 0;
    tf_evse_v2_get_low_level_state(&evse, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &evse_uptime);
    if (evse_uptime > 10000) {
        tf_evse_v2_set_indicator_led(&evse, 2005, 3000, 60, 255, 255, nullptr);
    }

    uint32_t start = millis();

    uint32_t button_press_time = BUTTON_IS_PRESSED;
    bool first = true;
    while (button_press_time == BUTTON_IS_PRESSED && !deadline_elapsed(start + BUTTON_MAX_PRESS_THRES)) {
        // Handle first boot with new firmware (i.e. the firmware supporting get_button_press_boot_time is not flashed yet)
        if (tf_evse_v2_get_button_press_boot_time(&evse, false, &button_press_time) == TF_E_NOT_SUPPORTED) {
            button_press_time = 0;
            break;
        }

        if (first && button_press_time == BUTTON_IS_PRESSED) {
            logger.printfln("Button is pressed. Waiting for release.");
            first = false;
        } else {
            led_blink(blue_led_pin, 200, 1, 0);
            led_blink(green_led_pin, 200, 1, 0);
        }
    }

    if (button_press_time == 0)
        return;

    if (deadline_elapsed(start + BUTTON_MAX_PRESS_THRES)) {
        logger.printfln("Button is pressed for more than 30 seconds. Assuming charger runs with front plate removed. Continuing normal boot.");
        return;
    }

    if (button_press_time < BUTTON_MIN_PRESS_THRES || button_press_time > BUTTON_MAX_PRESS_THRES){
        logger.printfln("Button was pressed for more %s. Continuing normal boot.", (button_press_time < BUTTON_MIN_PRESS_THRES) ? "less than 10 seconds" : "more than 30 seconds");
        return;
    }

    logger.printfln("Button was pressed for %.3f seconds on EVSE startup. Starting recovery routine.", button_press_time / 1000.0);

    uint8_t stage = 0;

    struct FactoryResetData {
        uint8_t stage:4;
        uint32_t magic:28;
        uint32_t uptime; // only for entropy
        uint16_t padding;
        uint16_t checksum;
    };

    uint8_t buf[63] = {0};
    tf_evse_v2_get_data_storage(&evse, DATA_STORE_PAGE_RECOVERY, buf);

    if (internet_checksum(buf, sizeof(FactoryResetData)) != 0) {
        logger.printfln("Checksum mismatch while reading recovery info from EVSE RAM. Assuming stage 0.");
        stage = 0;
    }
    else {
        FactoryResetData data;
        memcpy(&data, buf, sizeof(data));
        if (data.magic != 0x0BADB007 || data.uptime > 30000 || data.stage > 3)
            stage = 0;
        else
            stage = data.stage;
    }

    logger.printfln("Requested recovery stage %u.", stage);

    if (stage != 3) {
        logger.printfln("Writing request for stage %u into EVSE RAM in case this boot-up fails.", stage + 1);

        FactoryResetData data;
        data.stage = stage + 1;
        data.magic = FACTORY_RESET_DATA_MAGIC;
        data.uptime = millis();
        data.padding = 0;
        data.checksum = 0;
        memcpy(buf, &data, sizeof(data));
        data.checksum = internet_checksum(buf, sizeof(data));

        memcpy(buf, &data, sizeof(data));
        tf_evse_v2_set_data_storage(&evse, DATA_STORE_PAGE_RECOVERY, buf);
    }

    switch (stage) {
        // Stage 0 - User can't reach the web interface anymore. Remove network configuration and disable http_auth.
        case 0:
            logger.printfln("Running stage 0: Resetting network configuration and disabling web interface login");

            mount_or_format_spiffs();
#if MODULE_USERS_AVAILABLE()
            {
                // We can't use api.restorePersistent config here,
                // as we are before users module's pre_setup.
                // users.config is not yet initialized so we can't
                // use it to guesstimate the required memory to give
                // ArduinoJson to parse the config.
                String path = API::getLittleFSConfigPath("users/config");
                if (LittleFS.exists(path)) {
                    DynamicJsonDocument doc(4096);
                    DeserializationError error = DeserializationError::Ok;
                    {
                        auto file = LittleFS.open(path, "r");
                        error = deserializeJson(doc, file);
                    }

                    if (error != DeserializationError::Ok) {
                        logger.printfln("Failed to reset HTTP authentication! %s", error.c_str());
                    } else {
                        doc["http_auth_enabled"] = false;
                        auto file = LittleFS.open(path, "w");
                        serializeJson(doc, file);
                    }
                }
            }
#endif

            api.removeConfig("ethernet/config");
            api.removeConfig("wifi/sta_config");
            api.removeConfig("wifi/ap_config");
            // Reset network config in case the hostname is broken or the web interface listen port is changed.
            // Browsers block HTTP on "unsafe" ports:
            // https://superuser.com/questions/188058/which-ports-are-considered-unsafe-by-chrome
            api.removeConfig("network/config");
            LittleFS.end();
            logger.printfln("Stage 0 done");
            break;
        // Stage 1 - ESP crashed when booting after stage 0. Remove all configuration
        case 1:
            logger.printfln("Running stage 1: Removing configuration but keeping charge log.");
            mount_or_format_spiffs();
            api.removeAllConfig();
            LittleFS.end();
            logger.printfln("Stage 1 done");
            break;
        // Stage 2 - ESP still crashed. Format data partition. (This also removes tracked charges and the username file)
        case 2:
#if MODULE_SYSTEM_AVAILABLE()
            logger.printfln("Running stage 2: Formatting data partition");
            mount_or_format_spiffs();
            system_.factory_reset(false);
            logger.printfln("Stage 2 done");
#endif
            break;
        // Stage 3 - ESP still crashed after formatting the data partition. The firmware is unrecoverably broken. To prevent a fast boot loop, delay here.
        case 3:
            logger.printfln("Running stage 3: Firmware is probably broken. Delaying next crash for a minute.");
            delay(60 * 1000);
            logger.printfln("Stage 3 done");
            break;
    }
}
