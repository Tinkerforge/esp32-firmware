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

#include <limits>

#include "config/private.h"
#include "config/slot_allocator.h"
#include "tools/malloc.h"

bool Config::ConfUint53::slotEmpty(const Slot *slot) {
    return slot->val == std::numeric_limits<uint64_t>::max();
}

Config::ConfUint53::Slot *Config::ConfUint53::allocSlotBuf(size_t elements)
{
    Config::ConfUint53::Slot *block = static_cast<decltype(block)>(malloc_32bit_addressed(elements * sizeof(*block)));

    for (size_t i = 0; i < elements; i++) {
        Config::ConfUint53::Slot *slot = block + i;

        slot->val = std::numeric_limits<uint64_t>::max();
    }

    return block;
}

uint64_t* Config::ConfUint53::getVal() { return &get_slot<Config::ConfUint53>(idx)->val; }
const uint64_t* Config::ConfUint53::getVal() const { return &get_slot<Config::ConfUint53>(idx)->val; }

const Config::ConfUint53::Slot *Config::ConfUint53::getSlot() const { return get_slot<Config::ConfUint53>(idx); }
Config::ConfUint53::Slot *Config::ConfUint53::getSlot() { return get_slot<Config::ConfUint53>(idx); }

Config::ConfUint53::ConfUint53(uint64_t val)
{
    idx = nextSlot<Config::ConfUint53>();
    auto *slot = this->getSlot();

    // TODO check val against limits here
    slot->val = val;
}

Config::ConfUint53::ConfUint53(const ConfUint53 &cpy)
{
    idx = nextSlot<Config::ConfUint53>();
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfUint53::~ConfUint53()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();
    slot->val = std::numeric_limits<uint64_t>::max();

    notify_free_slot<Config::ConfUint53>(idx);
}

Config::ConfUint53 &Config::ConfUint53::operator=(const ConfUint53 &cpy)
{
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config::ConfUint53::ConfUint53(ConfUint53 &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfUint53 &Config::ConfUint53::operator=(ConfUint53 &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}
