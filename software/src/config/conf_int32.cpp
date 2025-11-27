/* esp32-firmware
 * Copyright (C) 2020-2024 Erik Fleckstein <erik@tinkerforge.com>
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

#include "config/private.h"
#include "config/slot_allocator.h"

#include <limits>

#include "tools/malloc.h"

#include "gcc_warnings.h"

// http://www.gotw.ca/gotw/050.htm#:~:text=What+About+bool
// std::vector<bool> is even more cursed than I thought.
static std::vector<uint8_t> slot_in_use;

bool Config::ConfInt32::slotEmpty(const Slot *slot, size_t slotIdx) {
    bool in_use = slot_in_use[slotIdx / 8] & (1 << slotIdx % 8);
    return !in_use;
}

static void mark_slot_in_use(uint16_t slotIdx) {
    slot_in_use[slotIdx / 8] |= static_cast<uint8_t>(1 << (slotIdx % 8));
}

static void mark_slot_free(uint16_t slotIdx) {
    slot_in_use[slotIdx / 8] &= static_cast<uint8_t>(~(1 << (slotIdx % 8)));
}

void Config::ConfInt32::slotDebugHook(const Slot *slot, size_t slotIdx) {

}

Config::ConfInt32::Slot *Config::ConfInt32::allocSlotBuf(size_t elements)
{
    Config::ConfInt32::Slot *block = static_cast<decltype(block)>(malloc_iram_or_psram_or_dram(elements * sizeof(*block)));
    if (block == nullptr)
        return nullptr;

    // This assumes that allocSlotBuf is only called if the current slot buffer is full.
    auto size_before_alloc = slot_in_use.size();
    assert(elements % 8 == 0);
    slot_in_use.reserve(size_before_alloc + elements / 8);

    for (size_t i = 0; i < elements / 8; ++i)
        slot_in_use.push_back(0);

    return block;
}

int32_t* Config::ConfInt32::getVal() { return &get_slot<Config::ConfInt32>(idx)->val; }
const int32_t* Config::ConfInt32::getVal() const { return &get_slot<Config::ConfInt32>(idx)->val; }

const Config::ConfInt32::Slot *Config::ConfInt32::getSlot() const { return get_slot<Config::ConfInt32>(idx); }
Config::ConfInt32::Slot *Config::ConfInt32::getSlot() { return get_slot<Config::ConfInt32>(idx); }

Config::ConfInt32::ConfInt32(int32_t val) : idx(nextSlot<Config::ConfInt32>())
{
    auto *slot = this->getSlot();
    slot->val = val;
    mark_slot_in_use(idx);
}

Config::ConfInt32::ConfInt32(const ConfInt32 &cpy) : idx(nextSlot<Config::ConfInt32>())
{
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
    mark_slot_in_use(idx);
}

Config::ConfInt32::~ConfInt32()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    mark_slot_free(idx);

    notify_free_slot<Config::ConfInt32>(idx);
}

Config::ConfInt32 &Config::ConfInt32::operator=(const ConfInt32 &cpy)
{
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config::ConfInt32::ConfInt32(ConfInt32 &&cpy) : idx(cpy.idx)
{
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}
