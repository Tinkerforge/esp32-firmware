/* esp32-firmware
 * Copyright (C) 2024 Erik Fleckstein <erik@tinkerforge.com>
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

#include <stdio.h>

#include "./malloc.h"

// minimal C++11 allocator with debug output
template <class Tp>
struct DebugAlloc {
    typedef Tp value_type;
    DebugAlloc() = default;
    template <class T> DebugAlloc(const DebugAlloc<T>&) {}

    int counter = 0;

    Tp *allocate(std::size_t n)
    {
        n *= sizeof(Tp);
        printf("!!! %d allocating %u bytes (%u)\n", counter, n, sizeof(Tp));
        ++counter;
        return static_cast<Tp *>(::operator new(n));
    }
    void deallocate(Tp *p, std::size_t n)
    {
        printf("!!! %d deallocating %u bytes\n", counter, n * sizeof *p);
        ++counter;
        ::operator delete(p);
    }
};
template <class T, class U>
bool operator==(const DebugAlloc<T>&, const DebugAlloc<U>&) { return true; }
template <class T, class U>
bool operator!=(const DebugAlloc<T>&, const DebugAlloc<U>&) { return false; }

constexpr size_t HEAP_ALLOC_OVERHEAD = 16; // 16 bytes overhead per heap allocation.
extern size_t allocated_by_counting_allocator;
extern size_t overhead_allocated_by_counting_allocator_heap;

// minimal C++11 allocator with debug output
template <class Tp>
struct CountingAllocator {
    typedef Tp value_type;
    CountingAllocator() = default;
    template <class T> CountingAllocator(const CountingAllocator<T>&) {}

    Tp *allocate(std::size_t n)
    {
        overhead_allocated_by_counting_allocator_heap += 16;
        allocated_by_counting_allocator += sizeof(Tp) * n;
        n *= sizeof(Tp);
        printf("!!! allocated %zu bytes; %zu bytes overhead\n", allocated_by_counting_allocator, overhead_allocated_by_counting_allocator_heap);
        return static_cast<Tp *>(::operator new(n));
    }
    void deallocate(Tp *p, std::size_t n)
    {
        allocated_by_counting_allocator -= sizeof(Tp) * n;
        overhead_allocated_by_counting_allocator_heap -= 16;
        printf("!!! allocated %zu bytes; %zu bytes overhead\n", allocated_by_counting_allocator, overhead_allocated_by_counting_allocator_heap);
        ::operator delete(p);
    }
};
template <class T, class U>
bool operator==(const CountingAllocator<T>&, const CountingAllocator<U>&) { return true; }
template <class T, class U>
bool operator!=(const CountingAllocator<T>&, const CountingAllocator<U>&) { return false; }

template <class Tp>
struct IRAMAlloc {
    typedef Tp value_type;
    IRAMAlloc() = default;
    template <class T> IRAMAlloc(const IRAMAlloc<T>&) {}

    Tp *allocate(std::size_t n)
    {
        return (Tp *)malloc_32bit_addressed(n * sizeof(Tp));
    }
    void deallocate(Tp *p, std::size_t n)
    {
        free_any(p);
    }
};
template <class T, class U>
bool operator==(const IRAMAlloc<T>&, const IRAMAlloc<U>&) { return true; }
template <class T, class U>
bool operator!=(const IRAMAlloc<T>&, const IRAMAlloc<U>&) { return false; }
