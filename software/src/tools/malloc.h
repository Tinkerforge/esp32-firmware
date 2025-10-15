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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <memory>
//#include <stdio.h>

#include "linear_allocator.h"

void tools_malloc_pre_setup();

enum RAM {
    DRAM,
    IRAM,
    PSRAM,
    _NONE
};

// There's also gnu::alloc_size but that seems to be broken:
// https://nullprogram.com/blog/2023/09/27/#enhance-alloc-with-attributes
// https://lists.sr.ht/~skeeto/public-inbox/%3Cane2ee7fpnyn3qxslygprmjw2yrvzppxuim25jvf7e6f5jgxbd@p7y6own2j3it%3E
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96503
[[gnu::malloc]]
void *perm_alloc(size_t size, RAM r);

[[gnu::malloc, gnu::alloc_align(1)]]
void *perm_alloc_aligned(size_t alignment, size_t size, RAM r);

[[gnu::malloc]]
void *perm_alloc_prefer(size_t size, RAM r1, RAM r2, RAM r3 = RAM::_NONE);

[[gnu::malloc, gnu::alloc_align(1)]]
void *perm_alloc_aligned_prefer(size_t alignment, size_t size, RAM r1, RAM r2, RAM r3 = RAM::_NONE);

template<typename T, class... Args>
[[gnu::malloc]]
T *perm_new(RAM r, Args&&... args) {
    static_assert(alignof(T) <= LinearAllocator::MAX_ALIGNMENT);

    void *mem = perm_alloc_aligned(alignof(T), sizeof(T), r);
    if (mem == nullptr)
        return nullptr;

    T *ptr = static_cast<T *>(mem);
    new (ptr) T{std::forward<Args>(args)...};
    return ptr;
}

template<typename T, class... Args>
[[gnu::malloc]]
T *perm_new_prefer(RAM r1, RAM r2, RAM r3, Args&&... args) {
    static_assert(alignof(T) <= LinearAllocator::MAX_ALIGNMENT);

    void *mem = perm_alloc_aligned_prefer(alignof(T), sizeof(T), r1, r2, r3);
    if (mem == nullptr)
        return nullptr;

    T *ptr = static_cast<T *>(mem);
    new (ptr) T{std::forward<Args>(args)...};
    return ptr;
}

// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/mem_alloc.html#bit-accessible-memory
void *malloc_32bit_addressed(size_t size);

void *malloc_psram(size_t size);

void *malloc_psram_or_dram(size_t size);

void *malloc_iram_or_psram_or_dram(size_t size);

void *malloc_aligned_psram_or_dram(size_t alignment, size_t size);

void *calloc_32bit_addressed(size_t count, size_t size);

void *calloc_psram_or_dram(size_t count, size_t size);

void *calloc_dram(size_t count, size_t size);

void free_any(void *ptr);

template<typename T>
T *new_psram_or_dram()
{
    static_assert(alignof(T) <= 8);

    void *ptr_heap = malloc_aligned_psram_or_dram(alignof(T), sizeof(T));

    if (ptr_heap == nullptr) {
        return nullptr;
    }

    T *ptr = static_cast<T *>(ptr_heap);

    //printf("new_psram_or_dram alignof(T)=%zu sizeof(T)=%zu ptr=%p\n", alignof(T), sizeof(T), static_cast<void *>(ptr));

    new (ptr) T;

    return ptr;
}

template<typename T>
void delete_psram_or_dram(T *ptr)
{
    if (ptr == nullptr) {
        return;
    }

    //printf("delete_psram_or_dram alignof(T)=%zu sizeof(T)=%zu ptr=%p\n", alignof(T), sizeof(T), static_cast<void *>(ptr));

    ptr->~T();

    free_any(ptr);
}

template<typename T>
T *new_array_psram_or_dram(size_t count)
{
    static_assert(alignof(T) <= 8);

    size_t size;

    if (__builtin_mul_overflow(count, sizeof(T), &size)) {
        return nullptr;
    }

    size_t header_size = sizeof(size_t);

    if (header_size < alignof(T)) {
        header_size = alignof(T);
    }

    void *ptr_heap = malloc_aligned_psram_or_dram(alignof(T), header_size + size);

    if (ptr_heap == nullptr) {
        return nullptr;
    }

    *reinterpret_cast<size_t *>(ptr_heap) = count;

    T *ptr = reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(ptr_heap) + header_size);

    //printf("new_array_psram_or_dram alignof(T)=%zu sizeof(T)=%zu count=%zu header_size=%zu size=%zu ptr_heap=%p ptr=%p\n",
    //       alignof(T), sizeof(T), count, header_size, size, ptr_heap, static_cast<void *>(ptr));

    for (size_t i = 0; i < count; ++i) {
        new (&ptr[i]) T;
    }

    return ptr;
}

template<typename T>
void delete_array_psram_or_dram(T *ptr)
{
    if (ptr == nullptr) {
        return;
    }

    size_t header_size = sizeof(size_t);

    if (header_size < alignof(T)) {
        header_size = alignof(T);
    }

    void *ptr_heap = reinterpret_cast<uint8_t *>(ptr) - header_size;
    size_t count = *reinterpret_cast<size_t *>(ptr_heap);

    //printf("delete_array_psram_or_dram alignof(T)=%zu sizeof(T)=%zu ptr=%p ptr_heap=%p header_size=%zu count=%zu\n",
    //       alignof(T), sizeof(T), static_cast<void *>(ptr), ptr_heap, header_size, count);

    for (size_t i = 0; i < count; ++i) {
        ptr[i].~T();
    }

    free_any(ptr_heap);
}

template <typename T>
class DeleterAny {
public:
    void operator()(T *t) {
        delete_psram_or_dram(t);
    }
};

// A std::unique_ptr that can free non-DRAM memory correctly.
template <typename T>
using unique_ptr_any = std::unique_ptr<T, DeleterAny<T>>;

// Allocates into PSRAM if possible, or in DRAM is no(t enough) PSRAM is available.
template<typename T, class... Args>
unique_ptr_any<T> make_unique_psram(Args&&... args)
{
    static_assert(alignof(T) <= 8);

    void *ptr_heap = malloc_aligned_psram_or_dram(alignof(T), sizeof(T));

    if (ptr_heap == nullptr) {
        return unique_ptr_any<T>{nullptr};
    }

    T *ptr = static_cast<T *>(ptr_heap);

    new (ptr) T(std::forward<Args>(args)...);

    return unique_ptr_any<T>{ptr};
}

