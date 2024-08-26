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

#include "meters_sma_speedwire.h"

#include "module_dependencies.h"
#include "meter_sma_speedwire.h"

#include "gcc_warnings.h"

void MetersSMASpeedwire::pre_setup()
{
    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 32)}
    });

    meters.register_meter_generator(get_class(), this);
}

MeterClassID MetersSMASpeedwire::get_class() const
{
    return MeterClassID::SMASpeedwire;
}

IMeter *MetersSMASpeedwire::new_meter(uint32_t slot, Config * /*state*/, Config * /*errors*/)
{
    return new MeterSMASpeedwire(slot);
}

const Config *MetersSMASpeedwire::get_config_prototype()
{
    return &config_prototype;
}

const Config *MetersSMASpeedwire::get_state_prototype()
{
    return Config::Null();
}

const Config *MetersSMASpeedwire::get_errors_prototype()
{
    return Config::Null();
}
