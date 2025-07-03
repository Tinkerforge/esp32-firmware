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

#include <limits>
#include <math.h>

#include "config.h"

struct ConfStringSlot {
    CoolString val = "";
    uint16_t minChars = 0;
    uint16_t maxChars = 0;
};

static constexpr const union {
    float f;
    uint32_t u;
} nan_uint = {NAN};

struct ConfFloatSlot {
    uint32_t val = 0;
    static constexpr float MARKER = nanf("0x0cafe");
};

struct ConfIntSlot {
    int32_t val = 0;
    int32_t min = 0;
    int32_t max = -1; // Use -1 because it fits into an int12 immediate when compiled. INT32_MIN and MAX do not.
};

struct ConfUintSlot {
    uint32_t val = 0;
    uint32_t min = std::numeric_limits<decltype(min)>::max();
    uint32_t max = 0;
};

struct ConfArraySlot {
    std::vector<Config> val;
    const Config *prototype;
    uint16_t minElements;
    uint16_t maxElements;
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

struct ConfInt52Slot {
    int64_t val = std::numeric_limits<int64_t>::max();
    static constexpr const int64_t min = -(1ll << 52ll) - 1;
    static constexpr const int64_t max =  (1ll << 52ll) - 1;
};

struct ConfUint53Slot {
    uint64_t val = std::numeric_limits<uint64_t>::max();
    static constexpr const int64_t min = 0;
    static constexpr const int64_t max = (1ll << 53ll) - 1;
};
