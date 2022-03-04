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
#include "ntp.h"

#include "task_scheduler.h"

#include "modules.h"

#include "time.h"
#include "sntp.h"

extern TaskScheduler task_scheduler;

Config *ntp_state;
bool first = true;
void ntp_sync_cb(struct timeval *t)
{
    if (first) {
        first = false;
        auto now = millis();
        auto secs = now / 1000;
        auto ms = now % 1000;
        logger.printfln("NTP synchronized at %lu,%03lu!", secs, ms);
    }

    char buf[INET6_ADDRSTRLEN] = {0};
    ipaddr_ntoa_r(sntp_getserver(0), buf, sizeof(buf));

    ntp_state->get("synced")->updateBool(true);
    ntp_state->get("server")->updateString(buf);
}

NTP::NTP()
{
    config = Config::Object({
        {"enable", Config::Bool(true)},
        {"use_dhcp", Config::Bool(true)},
        {"timezone", Config::Str("Europe/Berlin", 0, 64)},
        {"server", Config::Str("", 0, 64)},
    });

    state = Config::Object({
        {"synced", Config::Bool(false)},
        {"server", Config::Str("", 0, INET6_ADDRSTRLEN)}
    });
}

void NTP::setup()
{
    initialized = true;

    api.restorePersistentConfig("ntp/config", &config);

    if (!config.get("enable")->asBool())
        return;

    ntp_state = &state;
    sntp_set_time_sync_notification_cb(ntp_sync_cb);

    sntp_servermode_dhcp(config.get("use_dhcp")->asBool() ? 1 : 0);

    esp_netif_init();
    if(sntp_enabled()){
        sntp_stop();
    }
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    if (config.get("server")->asString() != "")
        sntp_setservername(0, config.get("server")->asCStr());

    sntp_init();
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    logger.printfln("Set timezone to %s", config.get("timezone")->asCStr());
}

void NTP::register_urls()
{
    api.addPersistentConfig("ntp/config", &config, {}, 1000);
    api.addState("ntp/state", &state, {}, 1000);
}

void NTP::loop()
{

}
