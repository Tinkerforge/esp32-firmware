/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "coredump.h"

#include <LittleFS.h>
#include <esp_core_dump.h>
#include <esp_flash.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools.h"

// Pre- and postfix take up 54 characters.
COREDUMP_RTC_DATA_ATTR char tf_coredump_info[512];

Coredump::Coredump()
{
    StaticJsonDocument<500> tf_coredump_json;
    tf_coredump_json["firmware_version"] = build_version_full_str();
    tf_coredump_json["firmware_commit_id"] = build_commit_id_str();
    tf_coredump_json["firmware_file_name"] = build_filename_str();

    if (build_coredump_info(tf_coredump_json)) {
        setup_error = CoredumpSetupError::OK;
        return;
    }

    tf_coredump_json.remove("firmware_file_name");

    if (build_coredump_info(tf_coredump_json)) {
        setup_error = CoredumpSetupError::Truncated;
        return;
    }

    setup_error = CoredumpSetupError::BufferToSmall;
}

bool Coredump::build_coredump_info(JsonDocument &tf_coredump_json)
{
    String tf_coredump_prefix = "___tf_coredump_info_start___";
    String tf_coredump_suffix = "___tf_coredump_info_end___";

    String tf_coredump_json_string;
    serializeJson(tf_coredump_json, tf_coredump_json_string);

    String tf_coredump_string = tf_coredump_prefix;

    if (tf_coredump_prefix.length() + tf_coredump_json_string.length() + tf_coredump_suffix.length() >= sizeof(tf_coredump_info))
        return false;

    tf_coredump_string += tf_coredump_json_string + tf_coredump_suffix;

    memcpy(tf_coredump_info, tf_coredump_string.c_str(), tf_coredump_string.length() + 1); // Include termination.

    return true;
}

void Coredump::pre_init()
{
    if (esp_reset_reason() != ESP_RST_PANIC) {
        return;
    }

    char task_name[16];

    { // Scope for summary
        esp_core_dump_summary_t summary;
        if (esp_core_dump_get_summary(&summary) == ESP_OK) {
            memcpy(task_name, summary.exc_task, sizeof(task_name));
        } else {
            snprintf(task_name, sizeof(task_name), "<unknown>");
        }
    }

    { // Scope for panic_reason
        char panic_reason[256];
        if (esp_core_dump_get_panic_reason(panic_reason, sizeof(panic_reason)) == ESP_OK) {
            logger.printfln("Task '%.16s' crashed: '%s'", task_name, panic_reason);
        } else {
            logger.printfln("Task '%.16s' crashed for unknown reasons", task_name);
        }
    }
}

void Coredump::pre_setup()
{
    bool coredump_available;
    esp_err_t status = esp_core_dump_image_check();

    if (status == ESP_ERR_INVALID_CRC) {
        printf("Core dump image invalid");
        coredump_available = false;
    } else {
        coredump_available = status == ESP_OK;
    }

    state = Config::Object({
        {"coredump_available", Config::Bool(coredump_available)}
    });

    if (setup_error == CoredumpSetupError::BufferToSmall) {
        logger.printfln("Buffer too small for any info data.");
    } else if (setup_error == CoredumpSetupError::Truncated) {
        logger.printfln("Buffer too small for all data; info data truncated.");
    }
}

void Coredump::register_urls()
{
    api.addState("coredump/state", &state);

    server.on_HTTPThread("/coredump/erase", HTTP_GET, [this](WebServerRequest request) {
        esp_core_dump_image_erase();

        if (esp_core_dump_image_check() == ESP_OK) {
            return request.send(503, "text/plain", "Error while erasing core dump");
        }

        task_scheduler.scheduleOnce([this]() {
            this->state.get("coredump_available")->updateBool(false);
        });

        return request.send(200);
    });

    server.on_HTTPThread("/coredump/coredump.elf", HTTP_GET, [this](WebServerRequest request) {
        constexpr size_t OFFSET_BEFORE_ELF_HEADER = 24;
        constexpr size_t BUFFER_SIZE = 2048;
        char buffer[BUFFER_SIZE];

        esp_err_t ret = esp_core_dump_image_check();
        if (ret != ESP_OK) {
            StringWriter sw(buffer, BUFFER_SIZE);
            sw.printf("No core dump image available: %s (0x%lX)", esp_err_to_name(ret), static_cast<uint32_t>(ret));
            return request.send(404, "text/plain", buffer, sw.getLength());
        }

        size_t addr;
        size_t size;
        ret = esp_core_dump_image_get(&addr, &size);
        if (ret != ESP_OK) {
            StringWriter sw(buffer, BUFFER_SIZE);
            sw.printf("Failed to get core dump image: %s (0x%lX)", esp_err_to_name(ret), static_cast<uint32_t>(ret));
            return request.send(503, "text/plain", buffer, sw.getLength());
        }

        request.beginChunkedResponse(200, "application/octet-stream");

        for (size_t i = 0; i < size; i += BUFFER_SIZE) {
            size_t to_send = std::min(BUFFER_SIZE, size - i);
            if (esp_flash_read(NULL, buffer, addr + i, to_send) != ESP_OK) {
                request.sendChunk("\n\nESP_FLASH_READ failed. Core dump truncated", HTTPD_RESP_USE_STRLEN);
                break;
            }
            request.sendChunk(buffer + (i == 0 ? OFFSET_BEFORE_ELF_HEADER : 0), to_send - (i == 0 ? OFFSET_BEFORE_ELF_HEADER : 0));
        }

        return request.endChunkedResponse();
    });
}
