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

bool Config::ConfUint::slotEmpty(size_t i) {
    return uint_buf[i].val == 0
        && uint_buf[i].min == 0
        && uint_buf[i].max == 0;
}

Config::ConfUint::Slot *Config::ConfUint::allocSlotBuf(size_t elements) {
    return (Config::ConfUint::Slot *) heap_caps_calloc(elements, sizeof(Config::ConfUint::Slot), MALLOC_CAP_32BIT);
}

void Config::ConfUint::freeSlotBuf(Config::ConfUint::Slot *buf)
{
    heap_caps_free(buf);
}

uint32_t* Config::ConfUint::getVal() { return &uint_buf[idx].val; }
const uint32_t* Config::ConfUint::getVal() const { return &uint_buf[idx].val; }

const Config::ConfUint::Slot *Config::ConfUint::getSlot() const { return &uint_buf[idx]; }
Config::ConfUint::Slot *Config::ConfUint::getSlot() { return &uint_buf[idx]; }

Config::ConfUint::ConfUint(uint32_t val, uint32_t min, uint32_t max)
{
    idx = nextSlot<Config::ConfUint>(uint_buf, uint_buf_size);
    auto *slot = this->getSlot();
    slot->val = val;
    slot->min = min;
    slot->max = max;
}

Config::ConfUint::ConfUint(const ConfUint &cpy)
{
    idx = nextSlot<Config::ConfUint>(uint_buf, uint_buf_size);
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfUint::~ConfUint()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();
    slot->val = 0;
    slot->min = 0;
    slot->max = 0;
}

Config::ConfUint &Config::ConfUint::operator=(const ConfUint &cpy)
{
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config::ConfUint::ConfUint(ConfUint &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfUint &Config::ConfUint::operator=(ConfUint &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}
