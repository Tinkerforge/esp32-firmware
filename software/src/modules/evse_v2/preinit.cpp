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

#include <stdint.h>
#include <string.h>
#include <LittleFS.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "bindings/hal_common.h"
#include "bindings/bricklet_evse_v2.h"
#include "tools.h"
#include "tools/fs.h"

#include "preinit.h"

static constexpr micros_t BUTTON_MIN_PRESS_THRES = 10_s;
static constexpr micros_t BUTTON_MAX_PRESS_THRES = 30_s;
#define BUTTON_IS_PRESSED 0xFFFFFFFF
#define FACTORY_RESET_DATA_MAGIC 0x0BADB007; //stage is written over the first 0. so this counts the number of bad boot(-up)s.

extern TF_HAL hal;

extern gpio_num_t blue_led_pin;
extern gpio_num_t green_led_pin;

static constexpr uint8_t MAX_STAGE = 3;

struct FactoryResetData {
    uint8_t stage:4;
    uint32_t magic:28;
    uint32_t uptime; // only for entropy
    uint16_t padding;
    uint16_t checksum;
};

static uint8_t recover_stage_from_flash() {
    if (!mount_or_format_data_partition()) {
        logger.printfln("Failed to format data partition.");
        return 0;
    }

    if (!LittleFS.exists(RECOVERY_STATE_PATH)) {
        logger.printfln("Recovery state file not found.");
        return 0;
    }

    uint8_t buf[63];
    {
        File f = LittleFS.open(RECOVERY_STATE_PATH, "rb");
        if (!f) {
            logger.printfln("Failed to open recovery state file!");
            return 0;
        }


        if (int read = f.read(buf, std::size(buf)); read != std::size(buf)) {
            logger.printfln("Failed to read recovery state file: %d != %zu", read, std::size(buf));
            return 0;
        }
    }

    if (uint16_t chksum = internet_checksum(buf, sizeof(FactoryResetData)); chksum != 0) {
        logger.printfln("Checksum mismatch while reading recovery state file: %u != 0", chksum);
        return 0;
    }

    FactoryResetData data;
    memcpy(&data, buf, sizeof(data));
    if (data.magic != 0x0BADB007) {
        logger.printfln("Magic mismatch while reading recovery state file: %u != %u", data.magic, 0x0BADB007);
        return 0;
    }
    if (data.uptime > 30000) {
        logger.printfln("Unexpected uptime while reading recovery state file: %lu > %lu", data.uptime, 30000ul);
        return 0;
    }
    if (data.stage > 3) {
        logger.printfln("Unexpected stage while reading recovery state file: %u > %u", data.stage, MAX_STAGE);
        return 0;
    }

    return data.stage;
}

static void save_stage_to_flash(uint8_t stage) {
    logger.printfln("Writing request for stage %u into flash in case this boot-up fails.", stage + 1);

    FactoryResetData data;
    data.stage = stage + 1;
    data.magic = FACTORY_RESET_DATA_MAGIC;
    data.uptime = now_us().to<millis_t>().as<uint32_t>();
    data.padding = 0;
    data.checksum = 0;

    uint8_t buf[63];

    memcpy(buf, &data, sizeof(data));
    data.checksum = internet_checksum(buf, sizeof(data));

    memcpy(buf, &data, sizeof(data));
    memset(buf + sizeof(data), 0, std::size(buf) - sizeof(data));

    File f = LittleFS.open(RECOVERY_STATE_PATH, "wb", true);
    if (!f) {
        logger.printfln("Failed to open recovery state file!");
        return;
    }

    if (size_t written = f.write(buf, std::size(buf)); written != std::size(buf)) {
        logger.printfln("Failed to write recovery state file: %zu != %zu", written, std::size(buf));
        return;
    }
}

void evse_v2_button_recovery_handler()
{
#if MODULE_ESP32_BRICK_AVAILABLE()
    auto &esp_brick = esp32_brick;
#elif MODULE_ESP32_ETHERNET_BRICK_AVAILABLE()
    auto &esp_brick = esp32_ethernet_brick;
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

    auto start = now_us();

    uint32_t button_press_time = BUTTON_IS_PRESSED;
    bool first = true;
    while (true) {
        // Handle first boot with new firmware (i.e. the firmware supporting get_button_press_boot_time is not flashed yet)
        if (tf_evse_v2_get_button_press_boot_time(&evse, false, &button_press_time) == TF_E_NOT_SUPPORTED) {
            button_press_time = 0;
            break;
        }

        if (button_press_time == 0) {
            return;
        }

        if (button_press_time != BUTTON_IS_PRESSED) {
            break;
        }

        if (first) {
            logger.printfln("Button is pressed. Waiting for release.");
            first = false;
        } else {
            led_blink(blue_led_pin, 200, 1, 0);
            led_blink(green_led_pin, 200, 1, 0);
        }

        if (deadline_elapsed(start + BUTTON_MAX_PRESS_THRES)) {
            logger.printfln("Button is pressed for more than 30 seconds. Assuming charger runs with front plate removed. Continuing normal boot.");
            return;
        }
    }

    auto min_ms = BUTTON_MIN_PRESS_THRES.to<millis_t>().as<uint32_t>();
    auto max_ms = BUTTON_MAX_PRESS_THRES.to<millis_t>().as<uint32_t>();

    if (button_press_time < min_ms || button_press_time > max_ms){
        logger.printfln("Button was pressed for more %s. Continuing normal boot.", (button_press_time < min_ms) ? "less than 10 seconds" : "more than 30 seconds");
        return;
    }

    logger.printfln("Button was pressed for %.3f seconds on EVSE startup. Starting recovery routine.", button_press_time / 1000.0);

    uint8_t stage = recover_stage_from_flash();

    logger.printfln("Requested recovery stage %u.", stage);

    // If this device is encrypted, don't allow access to PII without authentication.
    bool is_encrypted = ESP32Common::is_encrypted();
    logger.printfln("Flash is%s encrypted", is_encrypted ? "" : " not");

    if (stage != MAX_STAGE) {
        save_stage_to_flash(stage);
    }

    switch (stage) {
        // Stage 0 - User can't reach the web interface anymore. Remove network configuration and disable http_auth.
        case 0:
            logger.printfln("Running stage 0: Resetting network configuration%s", is_encrypted ? "" : " and disabling web interface login");

#if MODULE_USERS_AVAILABLE()
            if (!is_encrypted) {
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

            logger.printfln("Stage 0 done");
            break;
        // Stage 1 - ESP crashed when booting after stage 0. Remove all configuration
        case 1:
            if (!is_encrypted) {
                logger.printfln("Running stage 1: Removing configuration but keeping charge log.");

                api.removeAllConfig();

                logger.printfln("Stage 1 done");
            } else {
                logger.printfln("Running stage 1: Encryption is enabled, doing nothing.");
            }
            break;
        // Stage 2 - ESP still crashed. Format data partition. (This also removes tracked charges and the username file)
        case 2:
#if MODULE_SYSTEM_AVAILABLE()
            logger.printfln("Running stage 2: Formatting data partition");

            system_.factory_reset(false);
            logger.printfln("Stage 2 done");
#endif
            break;
        // Stage 3 - ESP still crashed after formatting the data partition. The firmware is unrecoverably broken. To prevent a fast boot loop, delay here.
        case MAX_STAGE:
            logger.printfln("Running stage 3: Firmware is probably broken. Delaying next crash for a minute.");
            delay(60 * 1000);
            logger.printfln("Stage 3 done");
            break;
    }

    // LittleFS is mounted in recover_stage_from_flash before running any stages.
    // Stage 2 unmounts, but immediately reboots.
    LittleFS.end();
}
