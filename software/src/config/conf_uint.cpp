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

bool Config::ConfUint::slotEmpty(const Slot *slot) {
    return slot->min == std::numeric_limits<decltype(slot->min)>::max()
        && slot->max == 0;
}

void Config::ConfUint::slotDebugHook(const Slot *slot) {

}

Config::ConfUint::Slot *Config::ConfUint::allocSlotBuf(size_t elements)
{
    Config::ConfUint::Slot *block = static_cast<decltype(block)>(malloc_iram_or_psram_or_dram(elements * sizeof(*block)));
    if (block == nullptr)
        return nullptr;

    for (size_t i = 0; i < elements; i++) {
        Config::ConfUint::Slot *slot = block + i;

        slot->min = std::numeric_limits<decltype(slot->min)>::max();
        slot->max = 0;
    }

    return block;
}

uint32_t* Config::ConfUint::getVal() { return &get_slot<Config::ConfUint>(idx)->val; }
const uint32_t* Config::ConfUint::getVal() const { return &get_slot<Config::ConfUint>(idx)->val; }

const Config::ConfUint::Slot *Config::ConfUint::getSlot() const { return get_slot<Config::ConfUint>(idx); }
Config::ConfUint::Slot *Config::ConfUint::getSlot() { return get_slot<Config::ConfUint>(idx); }

Config::ConfUint::ConfUint(uint32_t val, uint32_t min, uint32_t max) : idx(nextSlot<Config::ConfUint>())
{
    if (min > max) {
        esp_system_abort("Invalid ConfUint limits: min > max");
    }

    auto *slot = this->getSlot();
    slot->val = val;
    slot->min = min;
    slot->max = max;
}

Config::ConfUint::ConfUint(const ConfUint &cpy) : idx(nextSlot<Config::ConfUint>())
{
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfUint::~ConfUint()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();
    slot->min = std::numeric_limits<decltype(slot->min)>::max();
    slot->max = 0;

    notify_free_slot<Config::ConfUint>(idx);
}

Config::ConfUint &Config::ConfUint::operator=(const ConfUint &cpy)
{
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config::ConfUint::ConfUint(ConfUint &&cpy) : idx(cpy.idx)
{
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfUint &Config::ConfUint::operator=(ConfUint &&cpy)
{
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}
