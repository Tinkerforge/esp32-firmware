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

bool Config::ConfUnion::slotEmpty(size_t i)
{
    return union_buf[i].prototypes == nullptr;
}

Config::ConfUnion::Slot *Config::ConfUnion::allocSlotBuf(size_t elements)
{
    return new Config::ConfUnion::Slot[elements];
}

void Config::ConfUnion::freeSlotBuf(Config::ConfUnion::Slot *buf)
{
    delete[] buf;
}

bool Config::ConfUnion::changeUnionVariant(uint8_t tag)
{
    auto &slot = union_buf[idx];
    for (int i = 0; i < slot.prototypes_len; ++i) {
        if (slot.prototypes[i].tag == tag) {
            union_buf[idx].tag = tag;
            slot.val = slot.prototypes[i].config;
            slot.val.set_updated(0xFF);
            return true;
        }
    }

    return false;
}
uint8_t Config::ConfUnion::getTag() const { return union_buf[idx].tag; }

Config* Config::ConfUnion::getVal() { return &union_buf[idx].val; }
const Config* Config::ConfUnion::getVal() const { return &union_buf[idx].val; }

const Config::ConfUnion::Slot* Config::ConfUnion::getSlot() const { return &union_buf[idx]; }
Config::ConfUnion::Slot* Config::ConfUnion::getSlot() { return &union_buf[idx]; }

Config::ConfUnion::ConfUnion(const Config &val, uint8_t tag, uint8_t prototypes_len, const ConfUnionPrototypeInternal prototypes[])
{
    idx = nextSlot<Config::ConfUnion>(union_buf, union_buf_size);

    auto *slot = this->getSlot();
    slot->tag = tag;
    slot->prototypes_len = prototypes_len;
    slot->prototypes = prototypes;
    slot->val = val;
}

Config::ConfUnion::ConfUnion(const ConfUnion &cpy)
{
    idx = nextSlot<Config::ConfUnion>(union_buf, union_buf_size);

    // We have to mark this slot as in use here:
    // This union could contain a nested union that will be copied over
    // The inner union's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as inUse first.
    this->getSlot()->prototypes = cpy.getSlot()->prototypes;


    // this->getSlot() is evaluated before the RHS of the assignment is copied over.
    // This results in the LHS pointing to a deallocated array if copying the RHS
    // resizes the slot array. Copying into a temp value (which resizes the array if necessary)
    // and moving this value in the slot works.
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfUnion::~ConfUnion()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();
    slot->val = *Config::Null();
    slot->tag = 0;
    slot->prototypes_len = 0;
    slot->prototypes = nullptr;
}

Config::ConfUnion &Config::ConfUnion::operator=(const ConfUnion &cpy)
{
    if (this == &cpy) {
        return *this;
    }

    *this->getSlot() = *cpy.getSlot();


    return *this;
}


Config::ConfUnion::ConfUnion(ConfUnion &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfUnion &Config::ConfUnion::operator=(ConfUnion &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}
