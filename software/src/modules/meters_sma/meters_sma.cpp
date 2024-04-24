/* esp32-firmware
 * Copyright (C) 2023 Thomas Hein
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

#include "meter_sma.h"
#include "meters_sma.h"
#include "module_dependencies.h"
#include "modules/meters/meter_defs.h"

#include "gcc_warnings.h"

void MetersSMA::pre_setup()
{
    config_prototype = Config::Object({
        {"display_name", Config::Str("SMA Energy Meter 2.0", 0, 32)}
    });

    state_prototype = Config::Object({
        {"type",  Config::Uint(METER_TYPE_CUSTOM_ALL_VALUES)}
    });

    meters.register_meter_generator(get_class(), this);
}

_ATTRIBUTE((const))
MeterClassID MetersSMA::get_class() const
{
    return MeterClassID::SMA;
}

IMeter * MetersSMA::new_meter(uint32_t slot, Config * /*state*/, Config * /*errors*/)
{
    return new MeterSMA(slot);
}

_ATTRIBUTE((const))
const Config * MetersSMA::get_config_prototype()
{
    return &config_prototype;
}

_ATTRIBUTE((const))
const Config * MetersSMA::get_state_prototype()
{
    return &state_prototype;
}

const Config * MetersSMA::get_errors_prototype()
{
    return Config::Null();
}
