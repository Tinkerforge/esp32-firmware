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
#include "tools/malloc.h"

bool Config::ConfInt::slotEmpty(const Slot *slot)
{
    return slot->min == 0
        && slot->max == -1;
}

Config::ConfInt::Slot *Config::ConfInt::allocSlotBuf(size_t elements)
{
    Config::ConfInt::Slot *block = static_cast<decltype(block)>(malloc_iram_or_psram_or_dram(elements * sizeof(*block)));
    if (block == nullptr)
        return nullptr;

    for (size_t i = 0; i < elements; i++) {
        Config::ConfInt::Slot *slot = block + i;

        slot->min = 0;
        slot->max = -1;
    }

    return block;
}

int32_t* Config::ConfInt::getVal() { return &get_slot<Config::ConfInt>(idx)->val; }
const int32_t* Config::ConfInt::getVal() const { return &get_slot<Config::ConfInt>(idx)->val; }

const Config::ConfInt::Slot *Config::ConfInt::getSlot() const { return get_slot<Config::ConfInt>(idx); }
Config::ConfInt::Slot *Config::ConfInt::getSlot() { return get_slot<Config::ConfInt>(idx); }

Config::ConfInt::ConfInt(int32_t val, int32_t min, int32_t max)
{
    if (min > max) {
        esp_system_abort("Invalid ConfInt limits: min > max");
    }

    idx = nextSlot<Config::ConfInt>();
    auto *slot = this->getSlot();
    slot->val = val;
    slot->min = min;
    slot->max = max;
}

Config::ConfInt::ConfInt(const ConfInt &cpy)
{
    idx = nextSlot<Config::ConfInt>();
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfInt::~ConfInt()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();
    slot->val = 0;
    slot->min = 0;
    slot->max = -1;

    notify_free_slot<Config::ConfInt>(idx);
}

Config::ConfInt &Config::ConfInt::operator=(const ConfInt &cpy)
{
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config::ConfInt::ConfInt(ConfInt &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfInt &Config::ConfInt::operator=(ConfInt &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}
