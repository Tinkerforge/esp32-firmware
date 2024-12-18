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

void *malloc_32bit_addressed(size_t s)
{
    return heap_caps_malloc(s, MALLOC_CAP_32BIT);
}

void *malloc_psram(size_t s)
{
    return heap_caps_malloc(s, MALLOC_CAP_SPIRAM);
}

void *malloc_psram_or_dram(size_t s)
{
    return heap_caps_malloc_prefer(s, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
}

void *calloc_32bit_addressed(size_t c, size_t s)
{
    return heap_caps_calloc(c, s, MALLOC_CAP_32BIT);
}

void *calloc_psram_or_dram(size_t c, size_t s)
{
    return heap_caps_calloc_prefer(c, s, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
}

void *calloc_dram(size_t c, size_t s)
{
    return heap_caps_calloc(c, s, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
}
