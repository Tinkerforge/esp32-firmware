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
class TF_PackedRingbuffer {
    static_assert(sizeof(AlignedT) % sizeof(T) == 0, "TF_PackedRingbuffer: Aligned type size must be divisible by target type size");
    static_assert((sizeof(T) & (sizeof(T) - 1)) == 0, "TF_PackedRingbuffer: Target type must have a size that is a power of two");
    static_assert((sizeof(AlignedT) & (sizeof(AlignedT) - 1)) == 0, "TF_PackedRingbuffer: Aligned type must have a size that is a power of two");
    static_assert(std::is_unsigned<AlignedT>::value, "TF_PackedRingbuffer: Aligned type must be unsigned");

public:
    TF_PackedRingbuffer() : start(0), end(0)
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


template <typename T, void*(*malloc_fn)(size_t), void(*free_fn)(void*)>
class TF_Ringbuffer {
public:
    TF_Ringbuffer() : start(0), end(0), buf_size(0)
    {

    }

    void setup(size_t size)
    {
        if ((size & (size - 1)) != 0)
            esp_system_abort("TF_Ringbuffer: size must be power of two");

        this->buf_size = size;
        buffer = (T *)malloc_fn(sizeof(T) * buf_size);
    }

    void clear()
    {
        start = 0;
        end = 0;
    }

    inline bool empty() {
        return start == end;
    }

    inline size_t size()
    {
        return buf_size - 1;
    }

    inline size_t used()
    {
        return mod_size(buf_size + end - start);
    }

    inline size_t free()
    {
        return size() - used();
    }

    void push(T val)
    {
        buffer[end] = val;
        end = mod_size(end + 1);

        // This is true if we've just overwritten the oldest item
        if (end == start) {
            start = mod_size(start + 1);
        }
    }

    bool pop(T *val)
    {
        // Silence Wmaybe-uninitialized in the _read_[type] functions.
        *val = 0;

        if (empty()) {
            return false;
        }

        *val = buffer[start];
        start = mod_size(start + 1);

        return true;
    }

    bool peek(T *val)
    {
        // Silence Wmaybe-uninitialized in the _read_[type] functions.
        *val = 0;

        if (empty()) {
            return false;
        }

        *val = buffer[start];
        return true;
    }

    bool peek_offset(T *val, size_t offset)
    {
        // Silence Wmaybe-uninitialized in the _read_[type] functions.
        *val = 0;

        if (used() <= offset) {
            return false;
        }

        size_t idx = mod_size(buf_size + start + offset);
        *val = buffer[idx];

        return true;
    }

    void push_n(const T *val, size_t n) {
        if (n > size())
            return;

        bool fits = n < free();

        while (end + n > buf_size) {
            size_t to_write = buf_size - end;
            memcpy(buffer + end, val, to_write);
            val += to_write;
            n -= to_write;
            end = 0;
        }

        memcpy(buffer + end, val, n);
        end = mod_size(end + n);

        if (!fits) {
            start = mod_size(end + 1);
        }
    }

    void pop_until(T needle)
    {
        while (!empty()) {
            bool found = buffer[start] == needle;
            start = mod_size(start + 1);

            if (found) {
                return;
            }
        }
        return;
    }

    void get_chunks(T **first, size_t *first_len, T** second, size_t *second_len) {
        bool wrap = end < start;
        if (!wrap) {
            *first = buffer + start;
            *first_len = end - start;
            *second = nullptr;
            *second_len = 0;
        } else {
            *first = buffer + start;
            *first_len = buf_size - start;
            *second = buffer;
            *second_len = end;
        }
    }

    // index of first valid elemnt
    size_t start;
    // index of first invalid element
    size_t end;
    size_t buf_size;
    T *buffer;

private:
    [[gnu::always_inline]]
    inline size_t mod_size(size_t x) {
        return x & (buf_size - 1);
    }
};
