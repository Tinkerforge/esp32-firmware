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

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <type_traits>

template <typename T, size_t SIZE, typename AlignedT, void*(*malloc_fn)(size_t), void(*free_fn)(void*)>
class TF_Ringbuffer {
    static_assert(sizeof(AlignedT) % sizeof(T) == 0, "TF_Ringbuffer: Aligned type size must be divisible by target type size");
    static_assert((sizeof(T) & (sizeof(T) - 1)) == 0, "TF_Ringbuffer: Target type must have a size that is a power of two");
    static_assert((sizeof(AlignedT) & (sizeof(AlignedT) - 1)) == 0, "TF_Ringbuffer: Aligned type must have a size that is a power of two");
    static_assert(std::is_unsigned<AlignedT>::value, "TF_Ringbuffer: Aligned type must be unsigned");

public:
    TF_Ringbuffer() : start(0), end(0)
    {
    }

    void setup()
    {
        auto buf_size = sizeof(T) * SIZE;
        if (buf_size % sizeof(AlignedT) != 0) {
            // Allocate up to one AlignedT more, as we need to store at least one T in it.
            buf_size += sizeof(AlignedT) - (buf_size % sizeof(AlignedT));
        }
        buffer = (AlignedT *)malloc_fn(buf_size);
    }

    void clear()
    {
        start = 0;
        end = 0;
    }

    size_t size()
    {
        return SIZE - 1;
    }

    size_t used()
    {
        if (end < start) {
            return SIZE + end - start;
        }

        return end - start;
    }

    size_t free()
    {
        return size() - used();
    }

    void write_aligned(size_t idx, T val)
    {
        if (sizeof(T) == sizeof(AlignedT)) {
            buffer[idx] = val;
            return;
        }

        size_t items_per_slot = sizeof(AlignedT) / sizeof(T);
        size_t buffer_idx = idx / items_per_slot;
        size_t buffer_offset = idx % items_per_slot;

        AlignedT bits = typename std::make_unsigned<T>::type(-1LL);
        AlignedT write_mask = bits << (buffer_offset * 8 * sizeof(T));
        AlignedT keep_mask = ~write_mask;

        buffer[buffer_idx] = (buffer[buffer_idx] & keep_mask) | (((AlignedT)val) << (buffer_offset * 8 * sizeof(T)));
    }

    T read_aligned(size_t idx)
    {
        if (sizeof(T) == sizeof(AlignedT)) {
            return buffer[idx];
        }

        size_t items_per_slot = sizeof(AlignedT) / sizeof(T);
        size_t buffer_idx = idx / items_per_slot;
        size_t buffer_offset = idx % items_per_slot;

        AlignedT bits = typename std::make_unsigned<T>::type(-1LL);

        return (buffer[buffer_idx] >> (buffer_offset * 8 * sizeof(T))) & bits;
    }

    void push(T val)
    {
        write_aligned(end, val);
        end++;
        if (end >= SIZE) {
            end = 0;
        }

        // This is true if we've just overwritten the oldest item
        if (end == start) {
            ++start;
            if (start >= SIZE) {
                start = 0;
            }
        }
    }
    bool pop(T *val)
    {
        // Silence Wmaybe-uninitialized in the _read_[type] functions.
        *val = 0;

        if (used() == 0) {
            return false;
        }

        *val = read_aligned(start);
        start++;
        if (start >= SIZE) {
            start = 0;
        }

        return true;
    }

    bool peek(T *val)
    {
        // Silence Wmaybe-uninitialized in the _read_[type] functions.
        *val = 0;

        if (used() == 0) {
            return false;
        }

        *val = read_aligned(start);
        return true;
    }

    bool peek_offset(T *val, size_t offset)
    {
        // Silence Wmaybe-uninitialized in the _read_[type] functions.
        *val = 0;

        if (used() <= offset) {
            return false;
        }

        size_t idx = start + offset >= SIZE ? start + offset - SIZE : start + offset;
        *val = read_aligned(idx);

        return true;
    }

    // index of first valid elemnt
    size_t start;
    // index of first invalid element
    size_t end;
    AlignedT *buffer;
};
