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

#include <sdkconfig.h>
#include <ESPmDNS.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

extern char local_uid_str[32];

void Network::pre_setup()
{
    config = Config::Object({
        {"hostname", Config::Str("replaceme", 0, 32)},
        {"enable_mdns", Config::Bool(true)},
        {"web_server_port", Config::Uint16(80)}
    });
}

void Network::setup()
{
    if (!api.restorePersistentConfig("network/config", &config)) {
        config.get("hostname")->updateString(String(BUILD_HOST_PREFIX) + "-" + local_uid_str);
    }

    initialized = true;
}

void Network::register_urls()
{
    api.addPersistentConfig("network/config", &config);

    if (!config.get("enable_mdns")->asBool())
        return;

    if (!MDNS.begin(config.get("hostname")->asEphemeralCStr())) {
        logger.printfln("Error setting up mDNS responder!");
    } else {
        logger.printfln("mDNS responder started");
    }
    MDNS.addService("http", "tcp", 80);

#if MODULE_DEBUG_AVAILABLE()
    debug.register_task("mdns", CONFIG_MDNS_TASK_STACK_SIZE);
#endif
}
