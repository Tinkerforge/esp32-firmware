/* esp32-firmware
 * Copyright (C) 2022 Erik Fleckstein <erik@tinkerforge.com>
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

#include "ocpp.h"

#include "build.h"
#include "modules.h"
#include "api.h"
#include "task_scheduler.h"

extern API api;
extern TaskScheduler task_scheduler;

extern char local_uid_str[7];

void Ocpp::pre_setup()
{
    config = Config::Object({
        {"enable", Config::Bool(false)},
        {"url", Config::Str("", 0, 128)}
    });
}

void Ocpp::setup()
{
    initialized = true;
    api.restorePersistentConfig("ocpp/config", &config);

    config_in_use = config;

    if (!config.get("enable")->asBool() || config.get("url")->asString().length() == 0)
        return;

    cp.start(config.get("url")->asCStr(), (String(BUILD_HOST_PREFIX) + '-' + local_uid_str).c_str());

    task_scheduler.scheduleWithFixedDelay([this](){
        cp.tick();
    }, 100, 100);
}

void Ocpp::register_urls()
{
    api.addPersistentConfig("ocpp/config", &config, {}, 1000);
}

void Ocpp::loop()
{

}
