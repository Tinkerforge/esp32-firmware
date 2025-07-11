/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "meters_meta.h"

#include "module_dependencies.h"
#include "options.h"
#include "modules/meters/meter_location.enum.h"

#include "gcc_warnings.h"

void MetersMeta::pre_setup()
{
    config_prototype = Config::Object({
        {"display_name",   Config::Str("", 0, 32)},
        {"location",       Config::Enum(MeterLocation::Unknown)},
        {"mode",           Config::Uint(0, 0, 4)},
        {"source_meter_a", Config::Uint(0, 0, OPTIONS_METERS_MAX_SLOTS() - 1)},
        {"source_meter_b", Config::Uint(1, 0, OPTIONS_METERS_MAX_SLOTS() - 1)},
        {"constant",       Config::Int32(0)},
    });

    meters.register_meter_generator(get_class(), this);
}

MeterClassID MetersMeta::get_class() const
{
    return MeterClassID::Meta;
}

IMeter *MetersMeta::new_meter(uint32_t slot, Config *state, Config *errors)
{
    return new MeterMeta(slot);
}

const Config *MetersMeta::get_config_prototype()
{
    return &config_prototype;
}

const Config *MetersMeta::get_state_prototype()
{
    return Config::Null();
}

const Config *MetersMeta::get_errors_prototype()
{
    return Config::Null();
}
