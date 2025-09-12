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

#include <string.h>

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
    dram_lin_alloc.setup({
#if defined(MANUALLY_RESERVED_6432B_AT_0x3FFAE6E0_DRAM)
        {reinterpret_cast<void *>(0x3FFAE6E0),  6432},
#endif
#if defined(MANUALLY_RESERVED_15072B_AT_0x3FFE0440_DRAM)
        {reinterpret_cast<void *>(0x3FFE0440), 15072},
#endif
    });

    iram_lin_alloc.setup({
#if defined(MANUALLY_RESERVED_8192B_AT_0x4009E000_IRAM)
        {reinterpret_cast<void *>(0x4009E000),  8192},
#endif
    });

#if defined(BOARD_HAS_PSRAM)
    psram_lin_alloc.setup({});
#endif
}

void *perm_alloc_aligned(size_t alignment, size_t size, RAM r) {
    switch (r) {
        case DRAM:
            return dram_lin_alloc.aligned_alloc(alignment, size);
        case IRAM:
            return iram_lin_alloc.aligned_alloc(alignment, size);

        case PSRAM:
#if defined(BOARD_HAS_PSRAM)
            return psram_lin_alloc.aligned_alloc(alignment, size);
#else
        break;
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

void *perm_alloc(size_t size, RAM r) {
    return perm_alloc_aligned(align(size), size, r);
}

void *perm_alloc_aligned_prefer(size_t alignment, size_t size, RAM r1, RAM r2, RAM r3) {
    void *result = perm_alloc_aligned(alignment, size, r1);
    if (result == nullptr)
        result = perm_alloc_aligned(alignment, size, r2);
    if (result == nullptr)
        result = perm_alloc_aligned(alignment, size, r3);

    return result;
}

void *perm_alloc_prefer(size_t size, RAM r1, RAM r2, RAM r3) {
    return perm_alloc_aligned_prefer(align(size), size, r1, r2, r3);
}

char *perm_strdup(const char *c) {
    auto len = strlen(c) + 1;
    auto *dst = perm_alloc(len, DRAM);
    if (dst == nullptr)
        return nullptr;
    memcpy(dst, c, len);
    return static_cast<char *>(dst);
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
