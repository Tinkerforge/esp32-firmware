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

#include "network.h"

#include "api.h"
#include "build.h"
#include "task_scheduler.h"

#include <ESPmDNS.h>
#include "NetBIOS.h"

extern API api;
extern TaskScheduler task_scheduler;
extern char local_uid_str[32];

void Network::pre_setup()
{
    config = Config::Object({
        {"hostname", Config::Str("replaceme", 0, 32)},
        {"enable_mdns", Config::Bool(true)}
    });
}

void Network::setup()
{
    String default_hostname = String(BUILD_HOST_PREFIX) + String("-") + String(local_uid_str);

    if (!api.restorePersistentConfig("network/config", &config)) {
        config.get("hostname")->updateString(default_hostname);
    }

    initialized = true;
}

void Network::register_urls()
{
    api.addPersistentConfig("network/config", &config, {}, 1000);

    if (!config.get("enable_mdns")->asBool())
        return;

    if (!MDNS.begin(config.get("hostname")->asCStr())) {
        logger.printfln("Error setting up mDNS responder!");
    } else {
        logger.printfln("mDNS responder started");
    }
    MDNS.addService("http", "tcp", 80);
    NBNS.begin(config.get("hostname")->asCStr());
}

void Network::loop()
{
}
