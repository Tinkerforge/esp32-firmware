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

#include <time.h>
#include <sntp.h>
#include <lwip/inet.h>
#include <esp_netif.h>

#include "timezone_translation.h"

extern TaskScheduler task_scheduler;
extern API api;

static Config *ntp_state;
static bool first = true;

void ntp_sync_cb(struct timeval *t)
{
    if (first) {
        first = false;
        auto now = millis();
        auto secs = now / 1000;
        auto ms = now % 1000;
        logger.printfln("NTP synchronized at %lu,%03lu!", secs, ms);
    }

    ntp_state->get("synced")->updateBool(true);
}

NTP::NTP()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(true)},
        {"use_dhcp", Config::Bool(true)},
        {"timezone", Config::Str("Europe/Berlin", 0, 32)}, // Longest is America/Argentina/ComodRivadavia = 32 chars
        {"server", Config::Str("ptbtime1.ptb.de", 0, 64)}, // We've applied for a vendor zone @ pool.ntp.org, however this seems to take quite a while. Use the ptb servers for now.
        {"server2", Config::Str("ptbtime2.ptb.de", 0, 64)},
    }), [](Config &conf) -> String {
        if (lookup_timezone(conf.get("timezone")->asCStr()) == nullptr)
            return "Can't update config: Failed to look up timezone.";
        return "";
    }};

    state = Config::Object({
        {"synced", Config::Bool(false)}
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

    bool dhcp = config.get("use_dhcp")->asBool();
    sntp_servermode_dhcp(dhcp ? 1 : 0);

    esp_netif_init();
    if (sntp_enabled()) {
        sntp_stop();
    }

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    if (config.get("server")->asString() != "")
        sntp_setservername(dhcp ? 1 : 0, config.get("server")->asCStr());
    if (config.get("server2")->asString() != "")
        sntp_setservername(dhcp ? 2 : 1, config.get("server2")->asCStr());

    sntp_init();
    const char *tzstring = lookup_timezone(config.get("timezone")->asCStr());
    if (tzstring == nullptr) {
        logger.printfln("Failed to look up timezone information for %s. Will not set timezone", config.get("timezone")->asCStr());
        return;
    }
    setenv("TZ", tzstring, 1);
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
