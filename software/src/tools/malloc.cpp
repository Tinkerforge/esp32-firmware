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

#include "linear_allocator.h"

static LinearAllocator dram_lin_alloc{
    [](size_t alignment, size_t size){return heap_caps_aligned_alloc(alignment, size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);},
    1, 8192, 1024};

static LinearAllocator iram_lin_alloc{
    [](size_t alignment, size_t size){return heap_caps_aligned_alloc(alignment, size, MALLOC_CAP_IRAM);},
    4, 8192, 1024};
#if defined(BOARD_HAS_PSRAM)
static LinearAllocator psram_lin_alloc{
    [](size_t alignment, size_t size){return heap_caps_aligned_alloc(alignment, size, MALLOC_CAP_SPIRAM);},
    1, 8192, 8192};
#endif

void tools_malloc_pre_setup() {
    dram_lin_alloc.setup({});
    iram_lin_alloc.setup({});
    psram_lin_alloc.setup({});
}

void *leak_aligned(size_t alignment, size_t size, RAM r) {
    switch (r) {
        case DRAM:
            return dram_lin_alloc.aligned_alloc(alignment, size);
        case IRAM:
            return iram_lin_alloc.aligned_alloc(alignment, size);
#if defined(BOARD_HAS_PSRAM)
        case PSRAM:
            return psram_lin_alloc.aligned_alloc(alignment, size);
#endif
        case _NONE:
            break;
    }
    return nullptr;
}

static size_t align(size_t size) {
     // Have to check size == 0 here to prevent undefined left shift.
    if (size == 0) {
        return 0;
    }

    size_t align = 1 << std::countr_zero(size);
    return std::min(align, alignof(max_align_t));
}

void *leak(size_t size, RAM r) {
    return leak_aligned(align(size), size, r);
}

void *leak_aligned_prefer(size_t alignment, size_t size, RAM r1, RAM r2, RAM r3) {
    void *result = leak_aligned(alignment, size, r1);
    if (result == nullptr)
        result = leak_aligned(alignment, size, r2);
    if (result == nullptr)
        result = leak_aligned(alignment, size, r3);

    return result;
}

void *leak_prefer(size_t size, RAM r1, RAM r2, RAM r3) {
    return leak_aligned_prefer(align(size), size, r1, r2, r3);
}

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

void *malloc_iram_or_psram_or_dram(size_t size)
{
    return heap_caps_malloc_prefer(size, 3, MALLOC_CAP_IRAM, MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT);
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
