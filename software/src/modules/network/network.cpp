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

#include "mdns.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "string_builder.h"
#include "build.h"

#if MODULE_ETHERNET_AVAILABLE()
#include "modules/ethernet/ethernet_state.enum.h"
#endif

#if MODULE_WIFI_AVAILABLE()
#include "modules/wifi/wifi_state.enum.h"
#endif

extern char local_uid_str[32];

void Network::pre_setup()
{
    config = Config::Object({
        {"hostname", Config::Str("replaceme", 0, 32)},
        {"enable_mdns", Config::Bool(true)},
        {"web_server_port", Config::Uint16(80)}
    });

    state = Config::Object({
        {"connected", Config::Bool(false)}
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
    api.addState("network/state", &state);

    if (!config.get("enable_mdns")->asBool()) {
        return;
    }

    if (mdns_init() != ESP_OK) {
        logger.printfln("Error initializing mDNS responder");
    } else {
        String hostname = config.get("hostname")->asString();

        if(mdns_hostname_set(hostname.c_str()) != ESP_OK) {
            logger.printfln("Error initializing mDNS hostname");
        } else {
            logger.printfln("mDNS responder started");
        }
    }

    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);

#if MODULE_DEBUG_AVAILABLE()
    debug.register_task("mdns", CONFIG_MDNS_TASK_STACK_SIZE);
#endif
}

void Network::register_events()
{
#if MODULE_ETHERNET_AVAILABLE()
    event.registerEvent("ethernet/state", {"connection_state"}, [this](const Config *connection_state) {
        ethernet_connected = connection_state->asEnum<EthernetState>() == EthernetState::Connected;
        update_connected();
        return EventResult::OK;
    });
#endif

#if MODULE_WIFI_AVAILABLE()
    event.registerEvent("wifi/state", {"connection_state"}, [this](const Config *connection_state) {
        wifi_sta_connected = connection_state->asEnum<WifiState>() == WifiState::Connected;
        update_connected();
        return EventResult::OK;
    });

    event.registerEvent("wifi/state", {"ap_sta_count"}, [this](const Config *ap_sta_count) {
        wifi_ap_sta_count = ap_sta_count->asUint();
        update_connected();
        return EventResult::OK;
    });
#endif
}

void Network::update_connected()
{
    connected = ethernet_connected || wifi_sta_connected || wifi_ap_sta_count > 0;

    if (state.get("connected")->updateBool(connected)) {
        if (connected) {
            char connections_str[24];
            StringWriter str{connections_str, ARRAY_SIZE(connections_str)};
            bool needs_comma = false;

            if (ethernet_connected) {
                str.puts("Ethernet");
                needs_comma = true;
            }
            if (wifi_sta_connected) {
                if (needs_comma) {
                    str.puts(", ");
                } else {
                    needs_comma = true;
                }
                str.puts("WiFi");
            }
            if (wifi_ap_sta_count > 0) {
                if (needs_comma) {
                    str.puts(", ");
                }
                str.puts("AP");
            }

            logger.printfln("Network connected (%s)", connections_str);
        } else {
            logger.printfln("Network disconnected");
        }
    }
}
