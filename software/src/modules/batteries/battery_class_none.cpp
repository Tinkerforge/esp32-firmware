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

#include "battery_class_none.h"

#include "battery_class_id.enum.h"

#include "gcc_warnings.h"

// for IBatteryGenerator
[[gnu::const]] IBattery *BatteryGeneratorNone::new_battery(uint32_t slot, Config */*state*/, Config */*errors*/)
{
    return this;
}

const Config *BatteryGeneratorNone::get_config_prototype()
{
    return Config::Null();
}

const Config *BatteryGeneratorNone::get_state_prototype()
{
    return Config::Null();
}

const Config *BatteryGeneratorNone::get_errors_prototype()
{
    return Config::Null();
}

// for both
[[gnu::const]] BatteryClassID BatteryGeneratorNone::get_class() const
{
    return BatteryClassID::None;
}
