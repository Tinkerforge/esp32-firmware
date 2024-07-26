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

#include "meters_pv_faker.h"

#include "module_dependencies.h"
#include "meter_pv_faker.h"

#include "gcc_warnings.h"

void MetersPvFaker::pre_setup()
{
    config_prototype = Config::Object({
        {"display_name",  Config::Str("", 0, 32)},
        {"topic",         Config::Str("", 0, 128)},
        {"limiter_topic", Config::Str("", 0, 128)},
        {"peak_power",    Config::Uint32(30*1000)},  // watt
        {"zero_at_lux",   Config::Uint32(100)},      // lux
        {"peak_at_lux",   Config::Uint32(105*1000)}, // lux
    });

    meters.register_meter_generator(get_class(), this);
}

MeterClassID MetersPvFaker::get_class() const
{
    return MeterClassID::PvFaker;
}

IMeter *MetersPvFaker::new_meter(uint32_t slot, Config * /*state*/, Config * /*errors*/)
{
    return new MeterPvFaker(slot);
}

const Config *MetersPvFaker::get_config_prototype()
{
    return &config_prototype;
}

const Config *MetersPvFaker::get_state_prototype()
{
    return Config::Null();
}

const Config *MetersPvFaker::get_errors_prototype()
{
    return Config::Null();
}
