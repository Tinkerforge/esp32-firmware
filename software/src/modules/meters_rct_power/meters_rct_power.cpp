/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include "meters_rct_power.h"

#include "module_dependencies.h"
#include "meter_rct_power.h"
#include "virtual_meter.enum.h"

#include "gcc_warnings.h"

void MetersRCTPower::pre_setup()
{
    config_prototype = Config::Object({
        {"display_name",   Config::Str("", 0, 32)},
        {"host",           Config::Str("", 0, 64)},
        {"port",           Config::Uint16(8899)},
        {"virtual_meter",  Config::Uint8(static_cast<uint8_t>(VirtualMeter::None))},
    });

    errors_prototype = Config::Object({
        {"timeout",           Config::Uint32(0)},
        {"checksum_mismatch", Config::Uint32(0)},
    });

    meters.register_meter_generator(get_class(), this);
}

void MetersRCTPower::loop()
{
    pool.tick();
}

MeterClassID MetersRCTPower::get_class() const
{
    return MeterClassID::RCTPower;
}

IMeter *MetersRCTPower::new_meter(uint32_t slot, Config *state, Config *errors)
{
    return new MeterRCTPower(slot, state, errors, &pool);
}

const Config *MetersRCTPower::get_config_prototype()
{
    return &config_prototype;
}

const Config *MetersRCTPower::get_state_prototype()
{
    return Config::Null();
}

const Config *MetersRCTPower::get_errors_prototype()
{
    return &errors_prototype;
}
