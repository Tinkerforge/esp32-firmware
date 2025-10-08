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

#include "gcc_warnings.h"

bool Config::ConfTuple::slotEmpty(const Slot *slot)
{
    return slot->length == std::numeric_limits<size_t>::max();
}

void Config::ConfTuple::slotDebugHook(const Slot *slot) {

}

Config::ConfTuple::Slot *Config::ConfTuple::allocSlotBuf(size_t elements)
{
    return new(std::nothrow) Config::ConfTuple::Slot[elements]();
}

[[gnu::noinline]]
[[gnu::noreturn]]
static void abort_on_index_oob(size_t index, size_t len)
{
    char msg[96];
    snprintf(msg, ARRAY_SIZE(msg), "Config index %zu out of bounds (tuple size %zu)!", index, len);
    esp_system_abort(msg);
}

Config *Config::ConfTuple::get(size_t i)
{
    auto *slot = this->getSlot();
    if (i >= slot->length) {
        abort_on_index_oob(i, slot->length);
    }
    return &slot->values[i];
}

Config *Config::ConfTuple::get_or_null(size_t i)
{
    auto *slot = this->getSlot();
    if (i >= slot->length) {
        return nullptr;
    }
    return &slot->values[i];
}

const Config *Config::ConfTuple::get(size_t i) const
{
    const auto *slot = this->getSlot();
    if (i >= slot->length) {
        abort_on_index_oob(i, slot->length);
    }
    return &slot->values[i];
}

const Config::ConfTuple::Slot *Config::ConfTuple::getSlot() const { return get_slot<Config::ConfTuple>(idx); }
Config::ConfTuple::Slot *Config::ConfTuple::getSlot() { return get_slot<Config::ConfTuple>(idx); }

Config::ConfTuple::ConfTuple(std::initializer_list<Config> tup) : idx(nextSlot<Config::ConfTuple>())
{
    auto *slot = this->getSlot();
    const size_t len = tup.size();

    if (len == 0) {
        slot->length = 0;
        return;
    }

    Config *vals = new Config[len];
    slot->values.reset(vals);
    slot->length = len;

    size_t i = 0;
    for (auto &t : tup) {
        vals[i] = std::move(t);
        ++i;
    }
}

Config::ConfTuple::ConfTuple(size_t length, Config &&cfg) : idx(nextSlot<Config::ConfTuple>())
{
    auto *slot = this->getSlot();

    if (length == 0) {
        slot->length = 0;
        return;
    }

    Config *vals = new Config[length];
    slot->values.reset(vals);
    slot->length = length;

    vals[0] = std::move(cfg);

    for (size_t i = 1; i < length; ++i) {
        vals[i] = vals[0];
    }
}

Config::ConfTuple::ConfTuple(const ConfTuple &cpy) : idx(nextSlot<Config::ConfTuple>())
{
    const size_t len = cpy.getSlot()->length;
    Config *cpy_vals = cpy.getSlot()->values.get();

    Config *vals = new Config[len];

    auto *slot = this->getSlot();
    slot->values.reset(vals);
    slot->length = len;

    for (size_t i = 0; i < len; ++i) {
        vals[i] = cpy_vals[i];
    }
}

Config::ConfTuple::~ConfTuple()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();
    slot->length = std::numeric_limits<size_t>::max();;
    slot->values = nullptr;

    notify_free_slot<Config::ConfTuple>(idx);
}

Config::ConfTuple &Config::ConfTuple::operator=(const ConfTuple &cpy)
{
    if (this == &cpy)
        return *this;

    const size_t len = cpy.getSlot()->length;
    Config *cpy_vals = cpy.getSlot()->values.get();

    Config *vals = new Config[len];

    auto *slot = this->getSlot();
    slot->values.reset(vals);
    slot->length = len;

    for (size_t i = 0; i < len; ++i) {
        vals[i] = cpy_vals[i];
    }

    return *this;
}

Config::ConfTuple::ConfTuple(ConfTuple &&cpy) : idx(cpy.idx)
{
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfTuple &Config::ConfTuple::operator=(ConfTuple &&cpy)
{
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}

size_t Config::ConfTuple::getSize() const {
    return this->getSlot()->length;
}
