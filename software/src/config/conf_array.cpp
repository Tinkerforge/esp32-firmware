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

#include "event_log_prefix.h"
#include "main_dependencies.h"

bool Config::ConfArray::slotEmpty(const Slot *slot)
{
    return slot->minElements > slot->maxElements;
}

Config::ConfArray::Slot *Config::ConfArray::allocSlotBuf(size_t elements)
{
    return new(std::nothrow) Config::ConfArray::Slot[elements]();
}

[[gnu::noinline]]
[[gnu::noreturn]]
static void abort_on_index_oob(size_t index, const std::vector<Config> *val, const Config::ConfArray *this_arr)
{
    char msg[96];
    auto slot = this_arr->getSlot();
    snprintf(msg, ARRAY_SIZE(msg), "Config index %zu out of bounds (vector size %zu, minElements %u, maxElements %u)!", index, val->size(), slot->minElements, slot->maxElements);
    esp_system_abort(msg);
}

Config *Config::ConfArray::get(size_t i)
{
    auto *val = this->getVal();
    if (i >= val->size()) {
        abort_on_index_oob(i, val, this);
    }
    return &(*val)[i];
}

const Config *Config::ConfArray::get(size_t i) const
{
    const auto *val = this->getVal();
    if (i >= val->size()) {
        abort_on_index_oob(i, val, this);
    }
    return &(*val)[i];
}

std::vector<Config> *Config::ConfArray::getVal() { return &get_slot<Config::ConfArray>(idx)->val; }
const std::vector<Config> *Config::ConfArray::getVal() const { return &get_slot<Config::ConfArray>(idx)->val; }

const Config::ConfArray::Slot *Config::ConfArray::getSlot() const { return get_slot<Config::ConfArray>(idx); }
Config::ConfArray::Slot *Config::ConfArray::getSlot() { return get_slot<Config::ConfArray>(idx); }
int8_t Config::ConfArray::getVariantType() const { return (int8_t) get_slot<Config::ConfArray>(idx)->prototype->value.tag; }

Config::ConfArray::ConfArray(std::initializer_list<Config> val, const Config *prototype, uint16_t minElements, uint16_t maxElements)
{
    if (minElements > maxElements) {
        esp_system_abort("ConfArray: Requested more minElements than maxElements!");
    }

    idx = nextSlot<Config::ConfArray>();
    auto *slot = this->getSlot();
    // Set min/max before val to be sure this slot is in use before copying val.
    slot->minElements = minElements;
    slot->maxElements = maxElements;

    slot->val = val;
    slot->prototype = prototype;
}

Config::ConfArray::ConfArray(const ConfArray &cpy)
{
    idx = nextSlot<Config::ConfArray>();
    // We have to mark this slot as in use here:
    // This array could contain a nested array that will be copied over
    // The inner array's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as in use first.
    this->getSlot()->minElements = 0;
    this->getSlot()->maxElements = 0;

    auto tmp = *cpy.getSlot();

    // Must call getSlot() again because any reference would be invalidated
    // if the copy triggers a move of the slots.
    *this->getSlot() = std::move(tmp);
}

Config::ConfArray::~ConfArray()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();
    // Mark slot as not in use
    this->getSlot()->minElements = 1;
    this->getSlot()->maxElements = 0;

    slot->val.clear();
    slot->prototype = nullptr;

    notify_free_slot<Config::ConfArray>(idx);
}

Config::ConfArray &Config::ConfArray::operator=(const ConfArray &cpy)
{
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config::ConfArray::ConfArray(ConfArray &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfArray &Config::ConfArray::operator=(ConfArray &&cpy) {
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}
