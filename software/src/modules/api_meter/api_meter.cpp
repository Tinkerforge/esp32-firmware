/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "api_meter.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"
#include "task_scheduler.h"
#include "web_server.h"
#include "modules.h"

extern EventLog logger;

extern WebServer server;
extern TaskScheduler task_scheduler;
extern Config modules;

extern API api;

APIMeter::APIMeter()
{
    state_update = Config::Object({
        {"state", Config::Uint8(0)}, // 0 - no energy meter, 1 - initialization error, 2 - meter available
        {"type", Config::Uint8(0)} // 0 - not available, 1 - sdm72, 2 - sdm630, 3 - sdm72v2
    });

    values_update = Config::Object({
        {"power", Config::Float(0.0)},
        {"energy_rel", Config::Float(0.0)},
        {"energy_abs", Config::Float(0.0)},
    });

    phases_update = Config::Object({
        {"phases_connected", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            new Config{Config::Bool(false)},
            3, 3, Config::type_id<Config::ConfBool>())},
        {"phases_active", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            new Config{Config::Bool(false)},
            3, 3, Config::type_id<Config::ConfBool>())}
    });

    all_values_update = Config::Array({},
        new Config{Config::Float(0)},
        0, METER_ALL_VALUES_COUNT, Config::type_id<Config::ConfFloat>());
}

void APIMeter::setup()
{
    initialized = true;
}

void APIMeter::register_urls()
{
    api.addCommand("meter/state_update", &state_update, {}, [this](){
        meter.updateMeterState(state_update.get("state")->asUint(), state_update.get("type")->asUint());
    }, false);

    api.addCommand("meter/values_update", &values_update, {}, [this](){
        meter.updateMeterValues(values_update.get("power")->asFloat(), values_update.get("energy_rel")->asFloat(), values_update.get("energy_abs")->asFloat());
    }, false);

    api.addCommand("meter/phases_update", &phases_update, {}, [this](){
        bool phases_active[3];
        bool phases_connected[3];

        for (int i = 0; i < 3; ++i)
            phases_active[i] = phases_update.get("phases_active")->get(i)->asBool();

        for (int i = 0; i < 3; ++i)
            phases_connected[i] = phases_update.get("phases_connected")->get(i)->asBool();

        meter.updateMeterPhases(phases_connected, phases_active);
    }, false);

    api.addCommand("meter/all_values_update", &all_values_update, {}, [this](){
        for(int i = 0; i < METER_ALL_VALUES_COUNT; ++i)
            meter.updateMeterAllValues(i, all_values_update.get(i)->asFloat());
    }, false);
}

void APIMeter::loop()
{
}
