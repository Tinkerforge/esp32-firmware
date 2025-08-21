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

#include "gcc_warnings.h"

bool Config::ConfString::slotEmpty(const Slot *slot)
{
    return !slot->val;
}

void Config::ConfString::slotDebugHook(const Slot *slot) {

}

Config::ConfString::Slot *Config::ConfString::allocSlotBuf(size_t elements)
{
    auto *result = new(std::nothrow) Config::ConfString::Slot[elements];
    if (result == nullptr)
        return nullptr;

    // A slot is empty if the string is invalid.
    for(size_t i = 0; i < elements; ++i)
        result[i].val.make_invalid();

    return result;
}

CoolString* Config::ConfString::getVal() { return &get_slot<Config::ConfString>(idx)->val; }
const CoolString* Config::ConfString::getVal() const { return &get_slot<Config::ConfString>(idx)->val; }

const Config::ConfString::Slot* Config::ConfString::getSlot() const { return get_slot<Config::ConfString>(idx); }
Config::ConfString::Slot* Config::ConfString::getSlot() { return get_slot<Config::ConfString>(idx); }

Config::ConfString::ConfString(const char *val, uint16_t minChars, uint16_t maxChars) : idx(nextSlot<Config::ConfString>())
{
    auto *slot = this->getSlot();

    slot->val = val;
    slot->minChars = minChars;
    slot->maxChars = maxChars;
}

Config::ConfString::ConfString(const String &val, uint16_t minChars, uint16_t maxChars) : idx(nextSlot<Config::ConfString>())
{
    auto *slot = this->getSlot();

    slot->val = val;
    slot->minChars = minChars;
    slot->maxChars = maxChars;
}

Config::ConfString::ConfString(String &&val, uint16_t minChars, uint16_t maxChars) : idx(nextSlot<Config::ConfString>())
{
    auto *slot = this->getSlot();

    slot->val = std::move(val);
    slot->minChars = minChars;
    slot->maxChars = maxChars;
}

Config::ConfString::ConfString(const ConfString &cpy) : idx(nextSlot<Config::ConfString>())
{
    // this->getSlot() is evaluated before the RHS of the assignment is copied over.
    // This results in the LHS pointing to a deallocated array if copying the RHS
    // resizes the slot array. Copying into a temp value (which resizes the array if necessary)
    // and moving this value in the slot works.
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfString::~ConfString()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();

    slot->val.make_invalid();
    slot->minChars = 0;
    slot->maxChars = 0;

    notify_free_slot<Config::ConfString>(idx);
}

Config::ConfString &Config::ConfString::operator=(const ConfString &cpy)
{
    if (this == &cpy) {
        return *this;
    }

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config::ConfString::ConfString(ConfString &&cpy) : idx(cpy.idx)
{
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfString &Config::ConfString::operator=(ConfString &&cpy)
{
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}
