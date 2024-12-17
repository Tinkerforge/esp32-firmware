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

#pragma once

#include <WString.h>
#include <stdint.h>

#include "battery_class_id.enum.h"
#include "config.h"

class IBattery
{
public:
    enum class Action {
        PermitGridCharge         = 0,
        RevokeGridChargeOverride = 1,
        ForbidDischarge          = 2,
        RevokeDischargeOverride  = 3,
        _max                     = 3,
    };

    virtual ~IBattery() = default;

    virtual BatteryClassID get_class() const = 0;
    virtual void setup(const Config &ephemeral_config) {}
    virtual void register_urls(const String &base_url) {}
    virtual void register_events()                     {}
    virtual void pre_reboot()                          {}

    virtual bool supports_action(Action action)     {return false;}
    virtual bool start_action(Action action)        {return false;}
    virtual bool get_current_action(Action *action) {return false;}
};
