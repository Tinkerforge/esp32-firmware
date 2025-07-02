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

#include <math.h>

#include "config/private.h"
#include "config/slot_allocator.h"
#include "tools/malloc.h"

typedef union {
    float f;
    uint32_t u;
} float_uint;

bool Config::ConfFloat::slotEmpty(const Slot *slot) {
    float_uint v;
    v.f = Config::ConfFloat::Slot::MARKER;
    return slot->val == v.u;
}

Config::ConfFloat::Slot *Config::ConfFloat::allocSlotBuf(size_t elements)
{
    Config::ConfFloat::Slot *block = static_cast<decltype(block)>(malloc_iram_or_psram_or_dram(elements * sizeof(*block)));
    if (block == nullptr)
        return nullptr;

    float_uint v;
    v.f = Config::ConfFloat::Slot::MARKER;

    for (size_t i = 0; i < elements; i++) {
        Config::ConfFloat::Slot *slot = block + i;
        slot->val = v.u;
    }

    return block;
}

float Config::ConfFloat::getVal() const {
    float_uint result;
    result.u = get_slot<Config::ConfFloat>(idx)->val;
    return result.f;
}

void Config::ConfFloat::setVal(float f) {
    float_uint v;
    v.f = f;

    float_uint v2;
    v2.f = Config::ConfFloat::Slot::MARKER;

    if (v.u == v2.u)
        v.f = NAN;

    get_slot<Config::ConfFloat>(idx)->val = v.u;
}

const Config::ConfFloat::Slot *Config::ConfFloat::getSlot() const { return get_slot<Config::ConfFloat>(idx); }
Config::ConfFloat::Slot *Config::ConfFloat::getSlot() { return get_slot<Config::ConfFloat>(idx); }

Config::ConfFloat::ConfFloat(float val)
{
    idx = nextSlot<Config::ConfFloat>();
    this->setVal(val);
}

Config::ConfFloat::ConfFloat(const ConfFloat &cpy)
{
    idx = nextSlot<Config::ConfFloat>();
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfFloat::~ConfFloat()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    float_uint v;
    v.f = Config::ConfFloat::Slot::MARKER;

    auto *slot = this->getSlot();
    slot->val = v.u;

    notify_free_slot<Config::ConfFloat>(idx);
}

Config::ConfFloat &Config::ConfFloat::operator=(const ConfFloat &cpy)
{
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config::ConfFloat::ConfFloat(ConfFloat &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfFloat &Config::ConfFloat::operator=(ConfFloat &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}
