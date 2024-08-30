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

bool Config::ConfFloat::slotEmpty(size_t i) {
    return float_buf[i].val == 0
        && float_buf[i].min == 0
        && float_buf[i].max == 0;
}
Config::ConfFloat::Slot *Config::ConfFloat::allocSlotBuf(size_t elements)
{
    return (Config::ConfFloat::Slot *) heap_caps_calloc(elements, sizeof(Config::ConfFloat::Slot), MALLOC_CAP_32BIT);
}

void Config::ConfFloat::freeSlotBuf(Config::ConfFloat::Slot *buf)
{
    heap_caps_free(buf);
}

typedef union {
    float f;
    uint32_t u;
} float_uint;

float Config::ConfFloat::getVal() const {
    float_uint result;
    result.u = float_buf[idx].val;
    return result.f;
}

void Config::ConfFloat::setVal(float f) {
    float_uint v;
    v.f = f;
    float_buf[idx].val = v.u;
}

float Config::ConfFloat::getMin() const {
    float_uint result;
    result.u = float_buf[idx].min;
    return result.f;
}

void Config::ConfFloat::setSlot(float val, float min, float max) {
    float_uint va, mi, ma;
    va.f = val;
    mi.f = min;
    ma.f = max;
    float_buf[idx].val = va.u;
    float_buf[idx].min = mi.u;
    float_buf[idx].max = ma.u;
}

float Config::ConfFloat::getMax() const {
    float_uint result;
    result.u = float_buf[idx].max;
    return result.f;
}

const Config::ConfFloat::Slot *Config::ConfFloat::getSlot() const { return &float_buf[idx]; }
Config::ConfFloat::Slot *Config::ConfFloat::getSlot() { return &float_buf[idx]; }

Config::ConfFloat::ConfFloat(float val, float min, float max)
{
    idx = nextSlot<Config::ConfFloat>(float_buf, float_buf_size);
    this->setSlot(val, min, max);
}

Config::ConfFloat::ConfFloat(const ConfFloat &cpy)
{
    idx = nextSlot<Config::ConfFloat>(float_buf, float_buf_size);
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfFloat::~ConfFloat()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();
    slot->val = 0;
    slot->min = 0;
    slot->max = 0;
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
