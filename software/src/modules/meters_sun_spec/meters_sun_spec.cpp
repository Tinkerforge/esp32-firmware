/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

#include "meter_sun_spec.h"
#include "meters_sun_spec.h"
#include "module_dependencies.h"

#include "gcc_warnings.h"

void MetersSunSpec::pre_setup()
{
    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 32)},
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)}, // 0 == auto discover
        {"device_address", Config::Uint(0, 1, 247)}, // 0 == auto discover
        {"model_id", Config::Uint16(0)}, // 0 == invalid
    });

    meters.register_meter_generator(get_class(), this);
}

_ATTRIBUTE((const))
MeterClassID MetersSunSpec::get_class() const
{
    return MeterClassID::SunSpec;
}

IMeter * MetersSunSpec::new_meter(uint32_t slot, Config * /*state*/, Config *config, Config * /*errors*/)
{
    return new MeterSunSpec(slot, config);
}

_ATTRIBUTE((const))
const Config * MetersSunSpec::get_config_prototype()
{
    return &config_prototype;
}

_ATTRIBUTE((const))
const Config * MetersSunSpec::get_state_prototype()
{
    return Config::Null();
}

_ATTRIBUTE((const))
const Config * MetersSunSpec::get_errors_prototype()
{
    return Config::Null();
}
