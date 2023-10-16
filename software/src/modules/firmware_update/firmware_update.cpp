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

#include "firmware_update.h"
#include "module_dependencies.h"

#include <Arduino.h>

#include <Update.h>
#include <LittleFS.h>

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "build.h"
#include "web_server.h"

#include "./crc32.h"
#include "./recovery_html.embedded.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern bool firmware_update_allowed;
extern int8_t green_led_pin;

// Newer firmwares contain a firmware info page.
#define FIRMWARE_INFO_OFFSET (0xd000 - 0x1000)
#define FIRMWARE_INFO_LENGTH 0x1000

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

static bool factory_reset_running = false;

void factory_reset(bool restart_esp)
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

void FirmwareUpdate::setup()
{
    initialized = true;
}

void FirmwareUpdate::reset_firmware_info()
{
    calculated_checksum = 0;
    info = firmware_info_t{};
    info_offset = 0;
    checksum_offset = 0;
    update_aborted = false;
    info_found = false;
}

bool FirmwareUpdate::handle_firmware_info_chunk(size_t chunk_index, uint8_t *data, size_t chunk_length)
{
    uint8_t *start = data;
    size_t length = chunk_length;

    if (chunk_index < FIRMWARE_INFO_OFFSET) {
        size_t to_skip = FIRMWARE_INFO_OFFSET - chunk_index;
        start += to_skip;
        length -= to_skip;
    }

    length = MIN(length, (FIRMWARE_INFO_OFFSET + FIRMWARE_INFO_LENGTH) - chunk_index - 4); // -4 to not calculate the CRC of itself

    if (info_offset < sizeof(info)) {
        size_t to_write = MIN(length, sizeof(info) - info_offset);
        memcpy(((uint8_t *)&info) + info_offset, start, to_write);
        info_offset += to_write;
    }

    crc32_ieee_802_3_recalculate(start, length, &calculated_checksum);

    const size_t checksum_start = FIRMWARE_INFO_OFFSET + FIRMWARE_INFO_LENGTH - 4;

    if (chunk_index + chunk_length < checksum_start)
        return false;

    if (chunk_index < checksum_start) {
        size_t to_skip = checksum_start - chunk_index;
        start = data + to_skip;
        length = chunk_length - to_skip;
    }

    length = MIN(length, 4);

    if (checksum_offset < sizeof(checksum)) {
        size_t to_write = MIN(length, sizeof(checksum) - checksum_offset);
        memcpy((uint8_t *)&checksum + checksum_offset, start, to_write);
        checksum_offset += to_write;
    }

    info_found = checksum_offset == sizeof(checksum) && info.magic[0] == 0x12CE2171 && (info.magic[1] & 0x00FFFFFF) == 0x6E12F0;
    return info_found;
}

String FirmwareUpdate::check_firmware_info(bool firmware_info_found, bool detect_downgrade, bool log)
{
    if (!firmware_info_found && BUILD_REQUIRE_FIRMWARE_INFO) {
        if (log) {
            logger.printfln("Failed to update: Firmware update has no info page!");
        }
        return "{\"error\":\"firmware_update.script.no_info_page\"}";
    }
    if (firmware_info_found) {
        if (checksum != calculated_checksum) {
            if (log) {
                logger.printfln("Failed to update: Firmware info page corrupted! Embedded checksum %x calculated checksum %x", checksum, calculated_checksum);
            }
            return "{\"error\":\"firmware_update.script.info_page_corrupted\"}";
        }

        if (strncmp(BUILD_DISPLAY_NAME, info.firmware_name, ARRAY_SIZE(info.firmware_name)) != 0) {
            if (log) {
                logger.printfln("Failed to update: Firmware is for a %.*s but this is a %s!", static_cast<int>(ARRAY_SIZE(info.firmware_name)), info.firmware_name, BUILD_DISPLAY_NAME);
            }
            return "{\"error\":\"firmware_update.script.wrong_firmware_type\"}";
        }

        if (detect_downgrade && compare_version(info.fw_version[0], info.fw_version[1], info.fw_version[2],
                                                BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH) < 0) {
            if (log) {
                logger.printfln("Failed to update: Firmware is a downgrade!");
            }
            char buf[128];
            snprintf(buf, sizeof(buf)/sizeof(buf[0]), "{\"error\":\"firmware_update.script.downgrade\", \"fw\":\"%u.%u.%u\", \"installed\":\"%i.%i.%i\"}",
                     info.fw_version[0], info.fw_version[1], info.fw_version[2],
                     BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH);
            return String(buf);
        }
    }
    return "";
}

bool FirmwareUpdate::handle_update_chunk(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length, bool final, size_t complete_length) {
    // The firmware files are merged with the bootloader, partition table, firmware_info and slot configuration bins.
    // The bootloader starts at offset 0x1000, which is the first byte in the firmware file.
    // The first firmware slot (i.e. the one that is flashed over USB) starts at 0x10000.
    // So we have to skip the first 0x10000 - 0x1000 bytes, after them the actual firmware starts.
    // Don't skip anything if we flash the LittleFS.
    const size_t firmware_offset = command == U_FLASH ? 0x10000 - 0x1000 : 0;
    static bool firmware_info_found = false;

    if (chunk_index == 0 && !Update.begin(complete_length - firmware_offset, command)) {
        logger.printfln("Failed to start update: %s", Update.errorString());
        request.send(400, "text/plain", Update.errorString());
        Update.abort();
        update_aborted = true;
        return true;
    }

    if (chunk_index == 0) {
        reset_firmware_info();
        firmware_info_found = false;
    }

    if (update_aborted) {
        return true;
    }

    if (chunk_index + chunk_length >= FIRMWARE_INFO_OFFSET && chunk_index < FIRMWARE_INFO_OFFSET + FIRMWARE_INFO_LENGTH) {
        firmware_info_found = handle_firmware_info_chunk(chunk_index, data, chunk_length);
    }

    if (chunk_index + chunk_length >= FIRMWARE_INFO_OFFSET + FIRMWARE_INFO_LENGTH) {
        String error = this->check_firmware_info(firmware_info_found, false, true);
        if (error != "") {
            request.send(400, "application/json", error.c_str());
            Update.abort();
            update_aborted = true;
            return true;
        }
    }

    if (chunk_index + chunk_length < firmware_offset) {
        return true;
    }

    uint8_t *start = data;
    size_t length = chunk_length;

    if (chunk_index < firmware_offset) {
        size_t to_skip = firmware_offset - chunk_index;
        start += to_skip;
        length -= to_skip;
    }

    auto written = Update.write(start, length);
    if (written != length) {
        logger.printfln("Failed to write update chunk with length %u; written %u, error: %s", length, written, Update.errorString());
        request.send(400, "text/plain", (String("Failed to write update: ") + Update.errorString()).c_str());
        this->firmware_update_running = false;
        Update.abort();
        return false;
    }

    if (final && !Update.end(true)) {
        logger.printfln("Failed to apply update: %s", Update.errorString());
        request.send(400, "text/plain", (String("Failed to apply update: ") + Update.errorString()).c_str());
        this->firmware_update_running = false;
        Update.abort();
        return false;
    }

    return true;
}

void FirmwareUpdate::register_urls()
{
    server.on_HTTPThread("/recovery", HTTP_GET, [](WebServerRequest req) {
        req.addResponseHeader("Content-Encoding", "gzip");
        req.addResponseHeader("ETag", "dontcachemeplease");
        // Intentionally don't handle the If-None-Match header:
        // This makes sure that a cached version is never used.
        return req.send(200, "text/html; charset=utf-8", recovery_html_data, recovery_html_length);
    });

    server.on("/check_firmware", HTTP_POST, [this](WebServerRequest request){
        if (!this->info_found && BUILD_REQUIRE_FIRMWARE_INFO) {
            return request.send(400, "application/json", "{\"error\":\"firmware_update.script.no_info_page\"}");
        }
        return request.send(200);
    },[this](WebServerRequest request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        if (index == 0) {
            this->reset_firmware_info();
        }

        bool firmware_update_allowed_check_required = true;
#if MODULE_ENERGY_MANAGER_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
        firmware_update_allowed_check_required = energy_manager.disallow_fw_update_with_vehicle_connected();
#endif
        if (firmware_update_allowed_check_required && !firmware_update_allowed) {
            request.send(400, "application/json", "{\"error\":\"firmware_update.script.vehicle_connected\"}");
            return false;
        }

        if (index > FIRMWARE_INFO_LENGTH) {
            request.send(400, "text/plain", "Too long!");
            return false;
        }

        bool firmware_info_found = handle_firmware_info_chunk(index + FIRMWARE_INFO_OFFSET, data, len);

        if (index + len >= FIRMWARE_INFO_LENGTH) {
            String error = this->check_firmware_info(firmware_info_found, true, false);
            if (error != "") {
                request.send(400, "application/json", error.c_str());
            }
        }

        return true;
    });

    server.on_HTTPThread("/flash_firmware", HTTP_POST, [this](WebServerRequest request){
        if (update_aborted)
            return request.unsafe_ResponseAlreadySent(); // Already sent in upload callback.

        this->firmware_update_running = false;

        if(!Update.hasError()) {
            logger.printfln("Firmware flashed successfully! Rebooting in one second.");
            task_scheduler.scheduleOnce([](){ESP.restart();}, 1000);
        }

        return request.send(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
    },[this](WebServerRequest request, String filename, size_t index, uint8_t *data, size_t len, bool final){

        this->firmware_update_running = true;
        return handle_update_chunk(U_FLASH, request, index, data, len, final, request.contentLength());
    });

    server.on_HTTPThread("/flash_spiffs", HTTP_POST, [this](WebServerRequest request){
        if(!Update.hasError()) {
            logger.printfln("SPFFS flashed successfully! Rebooting in one second.");
            task_scheduler.scheduleOnce([](){ESP.restart();}, 1000);
        }

        return request.send(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
    },[this](WebServerRequest request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        return handle_update_chunk(U_SPIFFS, request, index, data, len, final, request.contentLength());
    });

    api.addCommand("factory_reset", Config::Confirm(), {Config::ConfirmKey()}, [this](String &result) {
        if (!Config::Confirm()->get(Config::ConfirmKey())->asBool()) {
            result = "Factory reset NOT requested";
            return;
        }

        task_scheduler.scheduleOnce([](){
            logger.printfln("Factory reset requested");
            factory_reset();
        }, 3000);
    }, true);

    api.addCommand("config_reset", Config::Confirm(), {Config::ConfirmKey()}, [this](String &result) {
        if (!Config::Confirm()->get(Config::ConfirmKey())->asBool()) {
            result = "Config reset NOT requested";
            return;
        }

        task_scheduler.scheduleOnce([](){
            logger.printfln("Config reset requested");

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
