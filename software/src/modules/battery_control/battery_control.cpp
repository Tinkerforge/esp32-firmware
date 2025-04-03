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

#include "battery_control.h"
#include "module_dependencies.h"

#include "event_log_prefix.h"
#include "tools.h"

void BatteryControl::pre_setup()
{
    config = Config::Object({
        {"block_discharge_during_fast_charge", Config::Bool(false)},
    });

    low_level_config = Config::Object({
        {"rewrite_period", Config::Uint(60, 5, 99999)}, // in seconds
    });

    state = Config::Object({
        {"grid_charge_allowed", Config::Bool(false)},
        {"discharge_blocked", Config::Bool(false)},
    });
}

void BatteryControl::setup()
{
    api.restorePersistentConfig("battery_control/config", &config);

    bool have_battery = false;

    for (size_t i = 0; i < BATTERIES_SLOTS; i++) {
        if (batteries.get_battery_class(i) != BatteryClassID::None) {
            have_battery = true;
            break;
        }
    }

    const bool block_discharge = config.get("block_discharge_during_fast_charge")->asBool();

    if (block_discharge && have_battery && charge_manager.get_charger_count() > 0) {
        task_scheduler.scheduleWithFixedDelay([this]() {
            bool want_blocked = charge_manager.fast_charger_in_c;
            bool changed = this->discharge_blocked != static_cast<TristateBool>(want_blocked);

            if (changed || deadline_elapsed(next_blocked_update)) {
                if (want_blocked) {
                    // Block
                    if (changed) {
                        logger.printfln("Blocking discharge");
                    }
                    batteries.start_action_all(IBattery::Action::ForbidDischarge);
                    this->discharge_blocked = TristateBool::True;
                } else {
                    // Unblock
                    if (changed) {
                        logger.printfln("Unblocking discharge");
                    }
                    batteries.start_action_all(IBattery::Action::RevokeDischargeOverride);
                    this->discharge_blocked = TristateBool::False;
                }

                this->state.get("discharge_blocked")->updateBool(static_cast<bool>(this->discharge_blocked));

                next_blocked_update = now_us() + seconds_t{this->low_level_config.get("rewrite_period")->asUint()};
            }
        }, 5_s, 1_s);
    }
}

void BatteryControl::register_urls()
{
    api.addPersistentConfig("battery_control/config", &config);
    api.addPersistentConfig("battery_control/low_level_config", &low_level_config);
    api.addState("battery_control/state", &state);
}
