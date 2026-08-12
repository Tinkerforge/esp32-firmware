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

#include <stdbool.h>
#include <stddef.h>
#include <bit>
#include <condition_variable>
#include <mutex>

#include <esp32/himem.h>

#include "tools.h"

class HimemBuffer {
public:
    HimemBuffer();

    void setup(size_t size);

    [[gnu::always_inline]] inline bool  empty() { return start == end;}
    [[gnu::always_inline]] inline size_t size() { return buf_size - 1; }
    [[gnu::always_inline]] inline size_t used() { return mod_buf_size(buf_size + end - start); }
    [[gnu::always_inline]] inline size_t free() { return size() - used(); }

    // write
    void clear();
    void push_n(const void *val, size_t n);
    template<typename T> void pop_until(T needle) {
        static_assert(std::has_single_bit(sizeof(T)));
        static_assert(BLOCK_SIZE % sizeof(T) == 0);

        size_t blocks = this->used_blocks();

        for (size_t block_idx = 0; block_idx < blocks; ++block_idx) {
            size_t block_len;
            T *block = reinterpret_cast<T *>(this->map_block(block_idx, &block_len));
            defer {this->unmap_block(block, block_idx != (blocks - 1)); };

            for (size_t i = 0; i < block_len / sizeof(T); ++i) {
                if (block[i] == needle) {
                    start = mod_buf_size(start + block_idx * BLOCK_SIZE + i * sizeof(T) + 1);
                    return;
                }
            }
        }

        // If we don't find the needle, drop everything.
        clear();
    }

    // read
    size_t used_blocks();
    void *map_block(size_t block_idx, size_t *out_block_len);
    void unmap_block(void *block, bool madv_dontneed=false);

    static constexpr size_t BLOCK_SIZE = ESP_HIMEM_BLKSZ;
    static constexpr size_t BLOCK_SIZE_BITS = 31 - __builtin_clz(BLOCK_SIZE);

    static constexpr size_t MIN_BUFFER_SIZE = 2 * BLOCK_SIZE;

private:
    // index of first valid byte
    size_t start;
    // index of first invalid byte
    size_t end;
    size_t buf_size;
    size_t block_count;

    size_t buf_size_bits;
    size_t block_count_bits;

    esp_himem_handle_t mh;

    [[gnu::always_inline]]
    void block_and_offset(size_t index, size_t *out_block, size_t *out_offset) {
        *out_offset = mod_block_size(index);
        *out_block  = div_block_size(index);
    }

    void write(size_t offset, const void *src, size_t len);

    //void read(size_t offset, void *dst, size_t len);

    [[gnu::always_inline]] inline size_t mod_buf_size(size_t x) { return x & (buf_size - 1); }

    [[gnu::always_inline]] inline size_t mod_block_size(size_t x) { return x & (BLOCK_SIZE - 1); }
    [[gnu::always_inline]] inline size_t div_block_size(size_t x) { return x >> BLOCK_SIZE_BITS; }
};
