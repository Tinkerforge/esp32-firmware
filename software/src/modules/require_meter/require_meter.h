/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "module.h"
#include "config.h"

#define METER_TIMEOUT_US 24ll * 60 * 60 * 1000 * 1000
//#define METER_TIMEOUT_US  10 * 1000 * 1000

#define METER_BOOTUP_ENERGY_TIMEOUT_US 90ll * 1000 * 1000

#define WARP_SMART 0
#define WARP_PRO_DISABLED 1
#define WARP_PRO_ENABLED 2


class RequireMeter final : public IModule {
private:
    ConfigRoot config;

public:
    void pre_setup();
    void setup();
    void register_urls();

    void start_task();
    void meter_found();
    void set_require_meter_enabled(bool enabled);
    void set_require_meter_blocking(bool block);

    bool get_require_meter_enabled();
    bool get_require_meter_blocking();
    bool allow_charging(float meter_value);

    bool initialized = false;
};