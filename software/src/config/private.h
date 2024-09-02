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

#pragma once

#include "config.h"
#include "tools.h"

#define SLOT_HEADROOM 20

struct ConfStringSlot {
    CoolString val = "";
    uint16_t minChars = 0;
    uint16_t maxChars = 0;
};

struct ConfFloatSlot {
    uint32_t val = 0;
    uint32_t min = 0;
    uint32_t max = 0;
};

struct ConfIntSlot {
    int32_t val = 0;
    int32_t min = 0;
    int32_t max = 0;
};

struct ConfUintSlot {
    uint32_t val = 0;
    uint32_t min = 0;
    uint32_t max = 0;
};

struct ConfArraySlot {
    std::vector<Config> val;
    const Config *prototype;
    uint16_t minElements;
    uint16_t maxElements;
    int8_t variantType;
    bool inUse = false;
};

struct ConfObjectSchema {
    struct Key {
        size_t length;
        const char *val;
    };
    size_t length;
    Key keys[];
};

struct ConfObjectSlot {
    const ConfObjectSchema *schema = nullptr;
    Config *values;
};

struct ConfUnionSlot {
    uint8_t tag = 0;
    uint8_t prototypes_len = 0;
    Config val;
    const ConfUnionPrototypeInternal *prototypes = nullptr;
};

extern Config::ConfUint::Slot *uint_buf;
extern size_t uint_buf_size;

extern Config::ConfInt::Slot *int_buf;
extern size_t int_buf_size;

extern Config::ConfFloat::Slot *float_buf;
extern size_t float_buf_size;

extern Config::ConfString::Slot *string_buf;
extern size_t string_buf_size;

extern Config::ConfArray::Slot *array_buf;
extern size_t array_buf_size;

extern Config::ConfObject::Slot *object_buf;
extern size_t object_buf_size;

extern Config::ConfUnion::Slot *union_buf;
extern size_t union_buf_size;

template<typename T>
static size_t nextSlot(typename T::Slot *&buf, size_t &buf_size) {
    ASSERT_MAIN_THREAD();
    for (size_t i = 0; i < buf_size; i++)
    {
        if (!T::slotEmpty(i))
            continue;

        return i;
    }

    auto new_buf = T::allocSlotBuf(buf_size + SLOT_HEADROOM);

    for (size_t i = 0; i < buf_size; ++i)
        new_buf[i] = std::move(buf[i]);

    T::freeSlotBuf(buf);
    buf = new_buf;
    size_t result = buf_size;
    buf_size = buf_size + SLOT_HEADROOM;
    return result;
}
