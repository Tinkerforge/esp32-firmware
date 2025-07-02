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
    float_uint max;
    max.u = slot->max;
    asm("" : : "r" (max.u)); // prevent slot->max access from being optimized into a float load
    return isnan(max.f);
}

Config::ConfFloat::Slot *Config::ConfFloat::allocSlotBuf(size_t elements)
{
    Config::ConfFloat::Slot *block = static_cast<decltype(block)>(malloc_iram_or_psram_or_dram(elements * sizeof(*block)));
    if (block == nullptr)
        return nullptr;

    float_uint v;
    v.f = NAN;

    for (size_t i = 0; i < elements; i++) {
        Config::ConfFloat::Slot *slot = block + i;

        slot->min = v.u;
        slot->max = v.u;
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
    get_slot<Config::ConfFloat>(idx)->val = v.u;
}

float Config::ConfFloat::getMin() const {
    float_uint result;
    result.u = get_slot<Config::ConfFloat>(idx)->min;
    return result.f;
}

void Config::ConfFloat::setSlot(float val, float min, float max) {
    Slot *slot = get_slot<Config::ConfFloat>(idx);

    float_uint va, mi, ma;
    va.f = val;
    mi.f = min;
    ma.f = max;
    slot->val = va.u;
    slot->min = mi.u;
    slot->max = ma.u;
}

float Config::ConfFloat::getMax() const {
    float_uint result;
    result.u = get_slot<Config::ConfFloat>(idx)->max;
    return result.f;
}

const Config::ConfFloat::Slot *Config::ConfFloat::getSlot() const { return get_slot<Config::ConfFloat>(idx); }
Config::ConfFloat::Slot *Config::ConfFloat::getSlot() { return get_slot<Config::ConfFloat>(idx); }

Config::ConfFloat::ConfFloat(float val, float min, float max)
{
    if (isnan(min) || isnan(max)) {
        esp_system_abort("Invalid ConfFloat limits: min and max cannot be NaN");
    }

    idx = nextSlot<Config::ConfFloat>();
    this->setSlot(val, min, max);
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
    v.f = NAN;

    auto *slot = this->getSlot();
    slot->min = v.u;
    slot->max = v.u;

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
