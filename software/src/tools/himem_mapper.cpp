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

#include "himem_mapper.h"

#include "gcc_warnings.h"

#include "tools/backtrace.h"

decltype(HimemMapper::handles) HimemMapper::handles{};
decltype(HimemMapper::handle_mutex) HimemMapper::handle_mutex{};
decltype(HimemMapper::handle_free_cv) HimemMapper::handle_free_cv{};
decltype(HimemMapper::cache_hits) HimemMapper::cache_hits = 0;
decltype(HimemMapper::cache_misses) HimemMapper::cache_misses = 0;
decltype(HimemMapper::lru_counter) HimemMapper::lru_counter = 0;

void HimemMapper::pre_init()
{
    std::lock_guard lock{handle_mutex};

    for (size_t i = 0; i < std::size(handles); ++i) {
        assert(esp_himem_alloc_map_range(ESP_HIMEM_BLKSZ, &handles[i].rh) == ESP_OK);
        handles[i].in_use = false;
        handles[i].mh = nullptr;
        handles[i].offset = 0;
        handles[i].ptr = nullptr;
        handles[i].last_used = 0;
    }
}

esp_himem_handle_t HimemMapper::alloc(size_t size)
{
    esp_himem_handle_t result;
    assert(esp_himem_alloc(size, &result) == ESP_OK);
    return result;
}

void *HimemMapper::map(esp_himem_handle_t mh, size_t offset)
{
    // Avoid heap alloc for capture list
    struct {
        size_t handle_idx;
        esp_himem_handle_t mh;
        size_t offset;
    } closure{SIZE_MAX, mh, offset};

    std::unique_lock lock{handle_mutex};

    handle_free_cv.wait(lock, [&closure]() {
        size_t lru = SIZE_MAX;

        for (size_t i = 0; i < std::size(handles); ++i) {
            auto &handle = handles[i];

            if (handle.mh == closure.mh && handle.offset == closure.offset) {
                closure.handle_idx = i;
                return true;
            }

            if (handle.in_use == 0 && (lru == SIZE_MAX || handle.last_used < handles[lru].last_used)) {
                lru = i;
            }
        }
        if (lru == SIZE_MAX)
            return false;

        closure.handle_idx = lru;
        return true;
    });

    auto &handle = handles[closure.handle_idx];

    if (handle.mh == closure.mh && handle.offset == closure.offset) {
        ++handle.in_use;
        handle.last_used = ++lru_counter;

        ++cache_hits;
        return handle.ptr;
    }

    ++cache_misses;

    if (handle.ptr != nullptr) {
        assert(esp_himem_unmap(handle.rh, handle.ptr, ESP_HIMEM_BLKSZ) == ESP_OK);
        handle.ptr = nullptr;
    }

    handle.in_use = 1;
    handle.mh = mh;
    handle.offset = offset;
    assert(esp_himem_map(handle.mh, handle.rh, handle.offset, 0, ESP_HIMEM_BLKSZ, 0, &handle.ptr) == ESP_OK);
    handle.last_used = ++lru_counter;

    return handle.ptr;
}

void HimemMapper::unmap(void *ptr, bool madv_dontneed)
{
    std::lock_guard lock{handle_mutex};
    for (size_t i = 0; i < std::size(handles); ++i) {
        auto &handle = handles[i];
        if (handle.ptr != ptr)
            continue;

        handle.last_used = ++lru_counter;

        assert(handle.in_use > 0);
        --handle.in_use;
        if (handle.in_use == 0) {
            if (madv_dontneed)
                handle.last_used = 0;
            handle_free_cv.notify_one();
        }
        return;
    }
    assert(false);
}
