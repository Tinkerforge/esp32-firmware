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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools.h"

// Pre- and postfix take up 54 characters.
COREDUMP_DRAM_ATTR char tf_coredump_info[512];

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

void Coredump::pre_setup()
{
    state = Config::Object({
        {"coredump_available", Config::Bool(false)}
    });

    if (setup_error == CoredumpSetupError::BufferToSmall) {
        logger.printfln("Buffer too small for any info data.");
    } else if (setup_error == CoredumpSetupError::Truncated) {
        logger.printfln("Buffer too small for all data; info data truncated.");
    }
}

void Coredump::setup()
{
    if (esp_core_dump_image_check() == ESP_OK)
        state.get("coredump_available")->updateBool(true);

    initialized = true;
}

void Coredump::register_urls()
{
    api.addState("coredump/state", &state);

    // TODO: Make this an API command?
    server.on("/coredump/erase", HTTP_GET, [this](WebServerRequest request) {
        esp_core_dump_image_erase();
        if (esp_core_dump_image_check() == ESP_OK)
            return request.send(503, "text/plain", "Error while erasing core dump");

        state.get("coredump_available")->updateBool(false);

        return request.send(200);
    });

    server.on_HTTPThread("/coredump/coredump.elf", HTTP_GET, [this](WebServerRequest request) {
        if (esp_core_dump_image_check() != ESP_OK)
            return request.send(404);

        auto buffer = heap_caps_calloc_prefer(4096, 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
        defer {free(buffer);};

        if (!buffer)
            return request.send(503, "text/plain", "Out of memory");

        esp_core_dump_summary_t summary;
        if (esp_core_dump_get_summary(&summary) != ESP_OK)
            return request.send(503, "text/plain", "Failed to get core dump summary");

        size_t addr;
        size_t size;
        if (esp_core_dump_image_get(&addr, &size) != ESP_OK)
            return request.send(503, "text/plain", "Failed to get core dump image size");

        request.beginChunkedResponse(200, "application/octet-stream");

        for (size_t i = 0; i < size; i += 4096) {
            size_t to_send = min((size_t)4096, size - i);
            if (esp_flash_read(NULL, buffer, addr + i, to_send) != ESP_OK) {
                String s = "ESP_FLASH_READ failed. Core dump truncated";
                request.sendChunk(s.c_str(), s.length());
                return request.endChunkedResponse();
            }
            request.sendChunk((char *)buffer + (i == 0 ? 20 : 0), to_send - (i == 0 ? 20 : 0));
        }

        return request.endChunkedResponse();
    });
}
