/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "config.h"

#include <limits>
#include <math.h>

static Config prototype_float_nan;
static Config prototype_int16_0;
static Config prototype_int32_0;
static Config prototype_uint8_0;
static Config prototype_uint16_0;
static Config prototype_uint32_0;
static Config prototype_bool_false;

const Config *Config::get_prototype_float_nan()
{
    if (prototype_float_nan.is_null()) {
        prototype_float_nan = Config::Float(NAN);
    }
    return &prototype_float_nan;
}

const Config *Config::get_prototype_int16_0()
{
    if (prototype_int16_0.is_null()) {
        prototype_int16_0 = Config::Int16(0);
    }
    return &prototype_int16_0;
}

const Config *Config::get_prototype_int32_0()
{
    if (prototype_int32_0.is_null()) {
        prototype_int32_0 = Config::Int(0);
    }
    return &prototype_int32_0;
}

const Config *Config::get_prototype_uint8_0()
{
    if (prototype_uint8_0.is_null()) {
        prototype_uint8_0 = Config::Uint8(0);
    }
    return &prototype_uint8_0;
}

const Config *Config::get_prototype_uint16_0()
{
    if (prototype_uint16_0.is_null()) {
        prototype_uint16_0 = Config::Uint16(0);
    }
    return &prototype_uint16_0;
}

const Config *Config::get_prototype_uint32_0()
{
    if (prototype_uint32_0.is_null()) {
        prototype_uint32_0 = Config::Uint(0);
    }
    return &prototype_uint32_0;
}

const Config *Config::get_prototype_bool_false()
{
    if (prototype_bool_false.is_null()) {
        prototype_bool_false = Config::Bool(false);
    }
    return &prototype_bool_false;
}
