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
#include "tools/memory.h"
#include "tools/malloc.h"

#include "gcc_warnings.h"

bool Config::ConfObject::slotEmpty(const Slot *slot)
{
    return slot->schema == nullptr;
}

void Config::ConfObject::slotDebugHook(const Slot *slot) {

}

Config::ConfObject::Slot *Config::ConfObject::allocSlotBuf(size_t elements)
{
    return static_cast<Config::ConfObject::Slot *>(calloc_32bit_addressed(elements, sizeof(Config::ConfObject::Slot)));
}

[[gnu::noinline]]
[[gnu::noreturn]]
static void abort_on_key_not_found(const char *needle)
{
    char msg[64];
    snprintf(msg, ARRAY_SIZE(msg), "Config key %s not found!", needle);
    esp_system_abort(msg);
}

Config *Config::ConfObject::get(const char *needle, size_t needle_len)
{
    const auto *slot = this->getSlot();
    const auto schema = slot->schema;
    const auto size = schema->length;
    const auto keys = schema->keys;

    if (address_is_in_rodata(needle)) {
        for (size_t i = 0; i < size; ++i) {
            if (keys[i].val == needle) { // Address comparison, not string comparison
                return &slot->values[i];
            }
        }
#ifdef DEBUG_FS_ENABLE
        logger.printfln("Key '%s' in rodata but not in keys.", needle);
#endif
    }

    if (!needle_len) {
        needle_len = strlen(needle);
    }

    for (size_t i = 0; i < size; ++i) {
        if (keys[i].length != needle_len)
            continue;

        if (memcmp(keys[i].val, needle, needle_len) == 0)
            return &slot->values[i];
    }

    abort_on_key_not_found(needle);
}

Config *Config::ConfObject::get_or_null(const char *needle, size_t needle_len)
{
    const auto *slot = this->getSlot();
    const auto schema = slot->schema;
    const auto size = schema->length;
    const auto keys = schema->keys;

    if (address_is_in_rodata(needle)) {
        for (size_t i = 0; i < size; ++i) {
            if (keys[i].val == needle) { // Address comparison, not string comparison
                return &slot->values[i];
            }
        }
#ifdef DEBUG_FS_ENABLE
        logger.printfln("Key '%s' in rodata but not in keys.", needle);
#endif
    }

    if (!needle_len) {
        needle_len = strlen(needle);
    }

    for (size_t i = 0; i < size; ++i) {
        if (keys[i].length != needle_len)
            continue;

        if (memcmp(keys[i].val, needle, needle_len) == 0)
            return &slot->values[i];
    }

    return nullptr;
}

const Config *Config::ConfObject::get(const char *needle, size_t needle_len) const
{
    const auto *slot = this->getSlot();
    const auto schema = slot->schema;
    const auto size = schema->length;
    const auto keys = schema->keys;

    if (address_is_in_rodata(needle)) {
        for (size_t i = 0; i < size; ++i) {
            if (keys[i].val == needle) { // Address comparison, not string comparison
                return &slot->values[i];
            }
        }
#ifdef DEBUG_FS_ENABLE
        logger.printfln("Key '%s' in rodata but not in keys.", needle);
#endif
    }

    if (!needle_len) {
        needle_len = strlen(needle);
    }

    for (size_t i = 0; i < size; ++i) {
        if (keys[i].length != needle_len)
            continue;

        if (memcmp(keys[i].val, needle, needle_len) == 0)
            return &slot->values[i];
    }

    abort_on_key_not_found(needle);
}

const Config::ConfObject::Slot *Config::ConfObject::getSlot() const { return get_slot<Config::ConfObject>(idx); }
Config::ConfObject::Slot *Config::ConfObject::getSlot() { return get_slot<Config::ConfObject>(idx); }

Config::ConfObject::ConfObject(std::vector<std::pair<const char *, Config>> &&val) : idx(nextSlot<Config::ConfObject>())
{
    const size_t len = val.size();

    // No need to use leak_aligned_prefer here: 4 + n * 8 always has 4 as smallest power of two
    // -> leak_prefer will use 4 byte alignment.
    static_assert(sizeof(ConfObjectSchema) % 8 == 4);
    static_assert(sizeof(ConfObjectSchema::Key) % 8 == 0);
    auto schema = static_cast<ConfObjectSchema *>(perm_alloc_prefer(sizeof(ConfObjectSchema) + len * sizeof(ConfObjectSchema::Key), IRAM, PSRAM, DRAM));
    schema->length = len;

    for (size_t i = 0; i < len; ++i) {
        const char *key = val[i].first;

        if (!address_is_in_rodata(key))
            esp_system_abort("ConfObject key not in flash! Please pass a string literal!");

        for (size_t j = i + 1; j < len; ++j) {
            if (key == val[j].first)
                esp_system_abort("ConfObject key not unique in this ConfObject! Remove duplicates!");
        }

        schema->keys[i].val = key;
        schema->keys[i].length = strlen(key);
    }

    auto *slot = this->getSlot();
    slot->schema = schema;

    slot->values = new Config[len]();

    for (size_t i = 0; i < len; ++i) {
        this->getSlot()->values[i] = std::move(val[i].second); //TODO: move here?
    }
}

Config::ConfObject::ConfObject(const ConfObject &cpy) : idx(nextSlot<Config::ConfObject>())
{
    // TODO: could we just use *this = cpy here?

    // We have to mark this slot as in use here:
    // (by setting the schema pointer first)
    // This object could contain a nested object that will be copied over
    // The inner object's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as in use first.
    this->getSlot()->schema = cpy.getSlot()->schema;

    const size_t len = cpy.getSlot()->schema->length;
    auto values = new Config[len]();
    for (size_t i = 0; i < len; ++i)
        values[i] = cpy.getSlot()->values[i];

    // Must call getSlot() again because any reference would be invalidated
    // if the copy triggers a move of the slots.
    this->getSlot()->values = values;
}

Config::ConfObject::~ConfObject()
{
    if (idx == std::numeric_limits<decltype(idx)>::max())
        return;

    auto *slot = this->getSlot();
    slot->schema = nullptr;

    if (slot->values != nullptr)
        delete[] slot->values;

    slot->values = nullptr;

    notify_free_slot<Config::ConfObject>(idx);
}

Config::ConfObject &Config::ConfObject::operator=(const ConfObject &cpy)
{
    if (this == &cpy)
        return *this;

    // We have to mark this slot as in use here:
    // (by setting the schema pointer first)
    // This object could contain a nested object that will be copied over
    // The inner object's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as in use first.
    this->getSlot()->schema = cpy.getSlot()->schema;

    const size_t len = cpy.getSlot()->schema->length;
    auto values = new Config[len]();
    for (size_t i = 0; i < len; ++i)
        values[i] = cpy.getSlot()->values[i];

    // Must call getSlot() again because any reference would be invalidated
    // if the copy triggers a move of the slots.
    if (this->getSlot()->values != nullptr)
        delete[] this->getSlot()->values;
    this->getSlot()->values = values;

    return *this;
}

Config::ConfObject::ConfObject(ConfObject &&cpy)  : idx(cpy.idx)
{
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
}

Config::ConfObject &Config::ConfObject::operator=(ConfObject &&cpy)
{
    this->idx = cpy.idx;
    cpy.idx = std::numeric_limits<decltype(idx)>::max();
    return *this;
}
