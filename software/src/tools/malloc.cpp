/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "malloc.h"

#include <esp_heap_caps.h>

void *malloc_32bit_addressed(size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_32BIT);
}

void *malloc_psram(size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void *malloc_psram_or_dram(size_t size)
{
    return heap_caps_malloc_prefer(size, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
}

void *malloc_aligned_psram_or_dram(size_t alignment, size_t size)
{
    void *ptr = nullptr;

#if defined(BOARD_HAS_PSRAM)
    // Only try PSRAM if the board has PSRAM to avoid malloc failure callback without PSRAM
    ptr = heap_caps_aligned_alloc(alignment, size, MALLOC_CAP_SPIRAM);
#endif

    if (ptr == nullptr) {
        ptr = heap_caps_aligned_alloc(alignment, size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    }

    return ptr;
}

void *calloc_32bit_addressed(size_t count, size_t size)
{
    return heap_caps_calloc(count, size, MALLOC_CAP_32BIT);
}

void *calloc_psram_or_dram(size_t count, size_t size)
{
    return heap_caps_calloc_prefer(count, size, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
}

void *calloc_dram(size_t count, size_t size)
{
    return heap_caps_calloc(count, size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
}

void free_any(void *ptr)
{
    heap_caps_free(ptr);
}
