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

#include "api.h"
#include "task_scheduler.h"

#include "modules.h"

#include <time.h>
#include <sntp.h>
#include <lwip/inet.h>
#include <esp_netif.h>

#include "timezone_translation.h"
#include "build.h"

static bool first = true;

#define NTP_DESYNC_THRESHOLD_S 25 * 60 * 60

static void ntp_sync_cb(struct timeval *t)
{
    if (first) {
        first = false;
        auto now = millis();
        auto secs = now / 1000;
        auto ms = now % 1000;
        logger.printfln("NTP synchronized at %lu,%03lu!", secs, ms);

        task_scheduler.scheduleWithFixedDelay([](){
            ntp.state.get("time")->updateUint(timestamp_minutes());
        }, 0, 1000);
    }

    task_scheduler.scheduleOnce([]() {
        ntp.set_synced();
    }, 0);

#if MODULE_RTC_AVAILABLE()
    if (api.hasFeature("rtc"))
    {
        task_scheduler.scheduleOnce([]() {
            timeval time;
            gettimeofday(&time, nullptr);
            rtc.set_time(time);
        }, 0);
    }
#endif
}

// Because there is the risk of a race condition with the rtc module,
// we have to replace the sntp_sync_time function with a thread-safe implementation.
extern "C" void sntp_sync_time(struct timeval *tv)
{
    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_IMMED)
    {
        {
            std::lock_guard<std::mutex> lock{ntp.mtx};
            settimeofday(tv, NULL);
            ntp.sync_counter++;
        }
        sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
    }
    else
        logger.printfln("This sync mode is not supported.");
    ntp_sync_cb(tv);
}

void NTP::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(true)},
        {"use_dhcp", Config::Bool(true)},
        {"timezone", Config::Str("Europe/Berlin", 0, 32)}, // Longest is America/Argentina/ComodRivadavia = 32 chars
        {"server", Config::Str("ptbtime1.ptb.de", 0, 64)}, // We've applied for a vendor zone @ pool.ntp.org, however this seems to take quite a while. Use the ptb servers for now.
        {"server2", Config::Str("ptbtime4.ptb.de", 0, 64)},
    }), [](Config &conf) -> String {
        if (lookup_timezone(conf.get("timezone")->asEphemeralCStr()) == nullptr)
            return "Can't update config: Failed to look up timezone.";
        return "";
    }};

    state = Config::Object({
        {"synced", Config::Bool(false)},
        {"time", Config::Uint32(0)} // unix timestamp in minutes
    });
}

void NTP::setup()
{
    initialized = true;

    api.restorePersistentConfig("ntp/config", &config);

    // As we use our own sntp_sync_time function, we do not need to register the cb function.
    // sntp_set_time_sync_notification_cb(ntp_sync_cb);

    bool dhcp = config.get("use_dhcp")->asBool();
    sntp_servermode_dhcp(dhcp ? 1 : 0);

    esp_netif_init();
    if (sntp_enabled()) {
        sntp_stop();
    }

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    // Keep local copies of ephemeral conf Strings because the SNTP lib doesn't create its own copies and holds references to whatever we pass to it.
    ntp_server1 = config.get("server")->asString();
    ntp_server2 = config.get("server2")->asString();
    if (ntp_server1 != "") {
        sntp_setservername(dhcp ? 1 : 0, ntp_server1.c_str());
    }
    if (ntp_server2 != "") {
        sntp_setservername(dhcp ? 2 : 1, ntp_server2.c_str());
    }

    const char *tzstring = lookup_timezone(config.get("timezone")->asEphemeralCStr());

    if (tzstring == nullptr) {
        logger.printfln("Failed to look up timezone information for %s. Will not set timezone", config.get("timezone")->asEphemeralCStr());
        return;
    }
    setenv("TZ", tzstring, 1);
    tzset();
    logger.printfln("Set timezone to %s", config.get("timezone")->asEphemeralCStr());

    if (config.get("enable")->asBool())
         sntp_init();
}

void NTP::set_synced()
{
    gettimeofday(&last_sync, NULL);
    ntp.state.get("synced")->updateBool(true);
}

void NTP::register_urls()
{
    api.addPersistentConfig("ntp/config", &config, {}, 1000);
    api.addState("ntp/state", &state, {}, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        struct timeval time;
        gettimeofday(&time, NULL);
        if (time.tv_sec - this->last_sync.tv_sec >= NTP_DESYNC_THRESHOLD_S || time.tv_sec < build_timestamp())
            ntp.state.get("synced")->updateBool(false);
    }, 0, 30 * 1000);
}

void NTP::loop()
{
}
