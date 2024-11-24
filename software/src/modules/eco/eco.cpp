/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
#include "eco.h"

#include <time.h>
#include <type_traits>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"


void Eco::pre_setup()
{
    this->trace_buffer_index = logger.alloc_trace_buffer("eco", 1 << 20);

    config = ConfigRoot{Config::Object({
        {"charge_plan_active", Config::Bool(false)},
        {"mode_after_charge_plan", Config::Uint(3, 0, 3)},
        {"charge_below_active", Config::Bool(false)},
        {"charge_below", Config::Int32(0)}, // in ct
        {"block_above_active", Config::Bool(false)},
        {"block_above", Config::Int32(20)} // in ct
    }), [this](Config &update, ConfigSource source) -> String {
        task_scheduler.scheduleOnce([this]() {
            this->update();
        });
        return "";
    }};

    charge_plan = Config::Object({
        {"enabled",Config::Bool(false)},
        {"day", Config::Uint(0, 0, 2)},
        {"time", Config::Int(8*60)}, // localtime in minutes since 00:00
        {"hours", Config::Uint(4, 1, 48)}
    });
    charge_plan_update = charge_plan;

    state = Config::Object({
        {"last_charge_plan_save", Config::Uint(0)},
    });
}

void Eco::setup()
{
    api.restorePersistentConfig("eco/config", &config);
    // TODO: Set user defined default charge_plan?

    initialized = true;
}

void Eco::register_urls()
{
    api.addPersistentConfig("eco/config", &config);
    api.addState("eco/state",             &state);

    api.addState("eco/charge_plan", &charge_plan);
    api.addCommand("eco/charge_plan_update", &charge_plan_update, {}, [this](String &/*errmsg*/) {
        charge_plan = charge_plan_update;
        state.get("last_charge_plan_save")->updateUint(rtc.timestamp_minutes());
        update();
    }, false);

    task_scheduler.scheduleWallClock([this]() {
        this->update();
    }, 1_m, 0_ms, true);
}

void Eco::update()
{
    current_decision = Decision::Normal;
}

Eco::Decision Eco::get_decision()
{
    if (!config.get("charge_planning_active")->asBool() &&
        !config.get("charge_below_active")->asBool() &&
        !config.get("block_above_active")->asBool()) {
        return Decision::Normal;
    }

    return current_decision;
}