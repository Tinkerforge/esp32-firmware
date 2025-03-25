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

bool Config::ConfInt52::slotEmpty(const Slot *slot)
{
    return slot->val == std::numeric_limits<int64_t>::max();
}

Config::ConfInt52::Slot *Config::ConfInt52::allocSlotBuf(size_t elements)
{
    Config::ConfInt52::Slot *block = static_cast<decltype(block)>(malloc_32bit_addressed(elements * sizeof(*block)));

    for (size_t i = 0; i < elements; i++) {
        Config::ConfInt52::Slot *slot = block + i;

        slot->val = std::numeric_limits<int64_t>::max();
    }

    return block;
}

int64_t* Config::ConfInt52::getVal() { return &get_slot<Config::ConfInt52>(idx)->val; }
const int64_t* Config::ConfInt52::getVal() const { return &get_slot<Config::ConfInt52>(idx)->val; }

const Config::ConfInt52::Slot *Config::ConfInt52::getSlot() const { return get_slot<Config::ConfInt52>(idx); }
Config::ConfInt52::Slot *Config::ConfInt52::getSlot() { return get_slot<Config::ConfInt52>(idx); }

Config::ConfInt52::ConfInt52(int64_t val)
{
    idx = nextSlot<Config::ConfInt52>();
    auto *slot = this->getSlot();

    // TODO check limits
    slot->val = val;
}

Config::ConfInt52::ConfInt52(const ConfInt52 &cpy)
{
    idx = nextSlot<Config::ConfInt52>();
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfInt52::~ConfInt52()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();
    slot->val = std::numeric_limits<int64_t>::max();

    notify_free_slot<Config::ConfInt52>(idx);
}

Config::ConfInt52 &Config::ConfInt52::operator=(const ConfInt52 &cpy)
{
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config::ConfInt52::ConfInt52(ConfInt52 &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfInt52 &Config::ConfInt52::operator=(ConfInt52 &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}
