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

#include "himem_buffer.h"

#include <stdint.h>
#include <string.h>

#include <esp_system.h>

#include "tools/himem_mapper.h"

#include "gcc_warnings.h"

HimemBuffer::HimemBuffer() : start(0),
                             end(0),
                             buf_size(0),
                             block_count(0),
                             buf_size_bits(0),
                             block_count_bits(0),
                             mh(nullptr)
    {}

void HimemBuffer::setup(size_t size)
{
    static_assert(std::has_single_bit(static_cast<uint32_t>(BLOCK_SIZE)));
    if (!std::has_single_bit(size))
        esp_system_abort("HimemBuffer: size must be power of two");

    if (size < 2 * BLOCK_SIZE)
        esp_system_abort("HimemBuffer: size must be at least BLOCK_SIZE * 2");

    if (size % BLOCK_SIZE != 0)
        esp_system_abort("HimemBuffer: size must be multiple of BLOCK_SIZE");

    this->buf_size = size;
    this->mh = HimemMapper::alloc(this->buf_size);

    this->block_count = this->buf_size / BLOCK_SIZE;

    if (!std::has_single_bit(this->block_count))
        esp_system_abortf<96>("HimemBuffer: block_count must be power of two; is %zu", this->block_count);

    this->buf_size_bits    = static_cast<size_t>(31 - __builtin_clz(this->buf_size));
    this->block_count_bits = static_cast<size_t>(31 - __builtin_clz(this->block_count));
}

void HimemBuffer::clear()
{
    start = 0;
    end = 0;
}

void HimemBuffer::push_n(const void *val, size_t n) {
    if (n > size())
        return;

    bool fits = n <= free();

    while (end + n > buf_size) {
        size_t to_write = buf_size - end;
        this->write(end, val, to_write);
        val = static_cast<const void *>(static_cast<const std::byte *>(val) + to_write);
        n -= to_write;
        end = 0;
    }

    this->write(end, val, n);
    end = mod_buf_size(end + n);

    if (!fits) {
        start = mod_buf_size(end + 1);
    }
}

size_t HimemBuffer::used_blocks() {
    if (empty())
        return 0;

    size_t start_block;
    size_t start_offset;
    block_and_offset(start, &start_block, &start_offset);

    size_t end_block;
    size_t end_offset;
    block_and_offset(end, &end_block, &end_offset);

    return end_block + (end >= start ? 0 : block_count) - start_block + (end_offset == 0 ? 0 : 1);
}

void *HimemBuffer::map_block(size_t block_idx, size_t *out_block_len) {
    size_t start_block;
    size_t start_offset;
    block_and_offset(start, &start_block, &start_offset);

    size_t end_block;
    size_t end_offset;
    block_and_offset(end, &end_block, &end_offset);

    size_t block = ((start_block + block_idx) % block_count);

    void *result = HimemMapper::map(this->mh, block << BLOCK_SIZE_BITS);

    size_t block_len = BLOCK_SIZE;
    // block_idx == 0 is almost identical to block == start_block
    // Only if used_blocks returned block_count + 1, i.e. when all blocks are in use
    // and start is behind end, block == start_block, but block_idx != 0
    // Don't apply start offset in that case.
    if (block_idx == 0) {
        result = static_cast<void *>(static_cast<std::byte *>(result) + start_offset);
        block_len -= start_offset;
    }

    if (block == end_block && (start < end || block_idx != 0)) {
        block_len -= BLOCK_SIZE - end_offset;
    }

    if (out_block_len != nullptr) {
        *out_block_len = block_len;
    }
    return result;
}

void HimemBuffer::unmap_block(void *block, bool madv_dontneed) {
    HimemMapper::unmap(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(block) & (~(BLOCK_SIZE - 1))),
                       madv_dontneed);
}

void HimemBuffer::write(size_t offset, const void *src, size_t len) {
    while(len > 0) {
        size_t offset_in_block = mod_block_size(offset);
        size_t block_offset = offset - offset_in_block;
        bool fills_block = BLOCK_SIZE - offset_in_block <= len;
        size_t to_write = fills_block ? BLOCK_SIZE - offset_in_block : len;

        void *block = HimemMapper::map(this->mh, block_offset);
        memcpy(static_cast<std::byte *>(block) + offset_in_block, src, to_write);

        len -= to_write;
        offset += to_write;
        src = static_cast<const void *>(static_cast<const std::byte *>(src) + to_write);

        HimemMapper::unmap(block, fills_block);
    }
}
