/* esp32-firmware
 * Copyright (C) 2025 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "config.h"
#include "module.h"
#include "TFTools/Micros.h"
#include "tools/tristate_bool.h"

class BatteryControl final : public IModule
{
public:
    BatteryControl() {}

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

private:
    ConfigRoot config;
    ConfigRoot low_level_config;
    ConfigRoot state;

    micros_t next_blocked_update = 0_us;
    TristateBool discharge_blocked = TristateBool::Undefined;
};
