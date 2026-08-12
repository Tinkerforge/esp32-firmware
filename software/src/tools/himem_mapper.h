/* esp32-firmware
 * Copyright (C) 2026 Erik Fleckstein <erik@tinkerforge.com>
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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <condition_variable>
#include <mutex>

#include <esp32/himem.h>

#include "TFTools/Micros.h"

struct HimemMapper {
    static struct {
        esp_himem_rangehandle_t rh;

        uint8_t in_use; // 0 - not in use; increment in map, decrement in unmap
        esp_himem_handle_t mh;
        size_t offset;
        void *ptr;
        uint64_t last_used;
    } handles[CONFIG_SPIRAM_BANKSWITCH_RESERVE];
    static std::mutex handle_mutex;
    static std::condition_variable handle_free_cv;

    static uint64_t cache_hits;
    static uint64_t cache_misses;

    static uint64_t lru_counter;

    static void pre_init();

    static esp_himem_handle_t alloc(size_t size);

    static void *map(esp_himem_handle_t mh, size_t offset);
    static void unmap(void *ptr, bool madv_dontneed=false);
};
