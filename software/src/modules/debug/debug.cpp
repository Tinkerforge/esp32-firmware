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

#include "debug.h"

#include <Arduino.h>

#include "esp_core_dump.h"

#include "api.h"
#include "tools.h"
#include "task_scheduler.h"

void Debug::pre_setup()
{
}

void Debug::setup()
{
    debug_state = Config::Object({
        {"uptime", Config::Uint32(0)},
        {"free_heap", Config::Uint32(0)},
        {"largest_free_heap_block", Config::Uint32(0)},
        {"free_psram", Config::Uint32(0)},
        {"largest_free_psram_block", Config::Uint32(0)}
    });

    task_scheduler.scheduleWithFixedDelay([this](){
        debug_state.get("uptime")->updateUint(millis());
        debug_state.get("free_heap")->updateUint(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        debug_state.get("largest_free_heap_block")->updateUint(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        debug_state.get("free_psram")->updateUint(heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        debug_state.get("largest_free_psram_block")->updateUint(heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    }, 1000, 1000);

    initialized = true;
}

void Debug::register_urls()
{
    api.addState("debug/state", &debug_state, {}, 1000);

    server.on("/debug/coredump.elf", HTTP_GET, [this](WebServerRequest request) {
        if (esp_core_dump_image_check() != ESP_OK)
            return request.send(404);

        auto buffer = heap_caps_calloc_prefer(4096, 1, MALLOC_CAP_32BIT | MALLOC_CAP_INTERNAL, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
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

        for (size_t i = 0; i < size; i += 4096)
        {
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

void Debug::loop()
{
}
