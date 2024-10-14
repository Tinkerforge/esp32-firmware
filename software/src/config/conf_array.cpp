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

#include "event_log_prefix.h"
#include "main_dependencies.h"

bool Config::ConfArray::slotEmpty(size_t i)
{
    return !array_buf[i].inUse;
}

Config::ConfArray::Slot *Config::ConfArray::allocSlotBuf(size_t elements)
{
    return new Config::ConfArray::Slot[elements];
}

void Config::ConfArray::freeSlotBuf(Config::ConfArray::Slot *buf)
{
    delete[] buf;
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

std::vector<Config> *Config::ConfArray::getVal() { return &array_buf[idx].val; }
const std::vector<Config> *Config::ConfArray::getVal() const { return &array_buf[idx].val; }

const Config::ConfArray::Slot *Config::ConfArray::getSlot() const { return &array_buf[idx]; }
Config::ConfArray::Slot *Config::ConfArray::getSlot() { return &array_buf[idx]; }

Config::ConfArray::ConfArray(std::vector<Config> val, const Config *prototype, uint16_t minElements, uint16_t maxElements, int8_t variantType)
{
    idx = nextSlot<Config::ConfArray>(array_buf, array_buf_size);
    auto *slot = this->getSlot();
    slot->inUse = true;

    slot->val = val;
    slot->prototype = prototype;
    slot->minElements = minElements;
    slot->variantType = variantType;

    if (maxElements < minElements) {
        slot->maxElements = minElements;
        logger.printfln("ConfArray of variantType %i: Requested maxElements of %u raised to fit minElements of %u.", variantType, maxElements, minElements);
    } else {
        slot->maxElements = maxElements;
    }
}

Config::ConfArray::ConfArray(const ConfArray &cpy)
{
    idx = nextSlot<Config::ConfArray>(array_buf, array_buf_size);
    // We have to mark this slot as in use here:
    // This array could contain a nested array that will be copied over
    // The inner array's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as inUse first.
    this->getSlot()->inUse = true;

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
    slot->inUse = false;

    slot->val.clear();
    slot->prototype = nullptr;
    slot->minElements = 0;
    slot->maxElements = 0;
    slot->variantType = 0;
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
