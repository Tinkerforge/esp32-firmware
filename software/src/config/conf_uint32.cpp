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

bool Config::ConfUint32::slotEmpty(const Slot *slot, size_t slotIdx) {
    bool in_use = slot_in_use[slotIdx / 8] & (1 << slotIdx % 8);
    return !in_use;
}

static void mark_slot_in_use(uint16_t slotIdx) {
    slot_in_use[slotIdx / 8] |= static_cast<uint8_t>(1 << (slotIdx % 8));
}

static void mark_slot_free(uint16_t slotIdx) {
    slot_in_use[slotIdx / 8] &= static_cast<uint8_t>(~(1 << (slotIdx % 8)));
}

void Config::ConfUint32::slotDebugHook(const Slot *slot, size_t slotIdx) {

}

Config::ConfUint32::Slot *Config::ConfUint32::allocSlotBuf(size_t elements)
{
    Config::ConfUint32::Slot *block = static_cast<decltype(block)>(malloc_iram_or_psram_or_dram(elements * sizeof(*block)));
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

uint32_t* Config::ConfUint32::getVal() { return &get_slot<Config::ConfUint32>(idx)->val; }
const uint32_t* Config::ConfUint32::getVal() const { return &get_slot<Config::ConfUint32>(idx)->val; }

const Config::ConfUint32::Slot *Config::ConfUint32::getSlot() const { return get_slot<Config::ConfUint32>(idx); }
Config::ConfUint32::Slot *Config::ConfUint32::getSlot() { return get_slot<Config::ConfUint32>(idx); }

Config::ConfUint32::ConfUint32(uint32_t val) : idx(nextSlot<Config::ConfUint32>())
{
    auto *slot = this->getSlot();
    slot->val = val;
    mark_slot_in_use(idx);
}

Config::ConfUint32::ConfUint32(const ConfUint32 &cpy) : idx(nextSlot<Config::ConfUint32>())
{
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
    mark_slot_in_use(idx);
}

Config::ConfUint32::~ConfUint32()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    mark_slot_free(idx);

    notify_free_slot<Config::ConfUint32>(idx);
}

Config::ConfUint32 &Config::ConfUint32::operator=(const ConfUint32 &cpy)
{
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config::ConfUint32::ConfUint32(ConfUint32 &&cpy) : idx(cpy.idx)
{
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}
