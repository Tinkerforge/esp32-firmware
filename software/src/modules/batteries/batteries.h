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

#include "options.h"
#include "config.h"
#include "ibattery.h"
#include "ibattery_generator.h"
#include "module.h"

class Batteries final : public IModule
{
public:
    enum class PathType {
        Base                     = 0,
        Config                   = 1,
        State                    = 2,
        Errors                   = 3,
        PermitGridCharge         = 4,
        RevokeGridChargeOverride = 5,
        ForbidDisharge           = 6,
        RevokeDishargeOverride   = 7,
        _max                     = 7,
    };

    Batteries()
    {
    }

    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;
    void pre_reboot() override;

    void register_battery_generator(BatteryClassID battery_class, IBatteryGenerator *generator);
    IBattery *get_battery(uint32_t slot);
    uint32_t get_batterys(BatteryClassID battery_class, IBattery **found_batterys, uint32_t found_batterys_capacity);
    BatteryClassID get_battery_class(uint32_t slot);

    String get_path(uint32_t slot, PathType path_type);

private:
    class BatterySlot final
    {
    public:
        IBattery *battery;

        ConfigRoot config_union;
        ConfigRoot state;
        ConfigRoot errors;
    };

    IBatteryGenerator *get_generator_for_class(BatteryClassID battery_class);
    IBattery *new_battery_of_class(BatteryClassID battery_class, uint32_t slot, Config *state, Config *errors);

    BatterySlot battery_slots[OPTIONS_BATTERIES_MAX_SLOTS()];

    std::vector<std::tuple<BatteryClassID, IBatteryGenerator *>> generators;
};
