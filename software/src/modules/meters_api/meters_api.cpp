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

#include "meters_api.h"

#include "module_dependencies.h"
#include "meter_api.h"

#include "gcc_warnings.h"

void MetersAPI::pre_setup()
{
    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 32)},
        {"value_ids", Config::Array({},
            meters.get_config_uint_max_prototype(),
            0, METERS_MAX_VALUES_PER_METER, Config::type_id<Config::ConfUint>()
        )},
    });

    meters.register_meter_generator(get_class(), this);
}

[[gnu::const]]
MeterClassID MetersAPI::get_class() const
{
    return MeterClassID::API;
}

IMeter *MetersAPI::new_meter(uint32_t slot, Config * /*state*/, Config * /*errors*/)
{
    return new MeterAPI(slot);
}

[[gnu::const]]
const Config *MetersAPI::get_config_prototype()
{
    return &config_prototype;
}

[[gnu::const]]
const Config *MetersAPI::get_state_prototype()
{
    return Config::Null();
}

[[gnu::const]]
const Config *MetersAPI::get_errors_prototype()
{
    return Config::Null();
}
