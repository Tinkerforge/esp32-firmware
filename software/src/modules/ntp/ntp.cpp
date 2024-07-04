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

#include <time.h>
#include <esp_sntp.h>
#include <lwip/inet.h>
#include <esp_netif.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "timezone_translation.h"
#include "build.h"

static bool first = true;

#define NTP_DESYNC_THRESHOLD_S 25 * 60 * 60

extern NTP ntp;

static void ntp_sync_cb(struct timeval *t)
{
    if (first) {
        first = false;
        auto now = millis();
        auto secs = now / 1000;
        auto ms = now % 1000;
        logger.printfln("NTP synchronized at %lu,%03lu", secs, ms);

        task_scheduler.scheduleWithFixedDelay([](){
            ntp.state.get("time")->updateUint(timestamp_minutes());
        }, 0, 1000);
    }

    task_scheduler.scheduleOnce([]() {
        ntp.set_synced();

#if MODULE_RTC_AVAILABLE()
        if (!api.hasFeature("rtc"))
            return;

        timeval time;
        gettimeofday(&time, nullptr);
        rtc.set_time(time);
#endif
    }, 0);
}

// Because there is the risk of a race condition with the rtc module,
// we have to replace the sntp_sync_time function with a thread-safe implementation.
extern "C" void sntp_sync_time(struct timeval *tv)
{
    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_IMMED) {
        {
            std::lock_guard<std::mutex> lock{ntp.mtx};
            settimeofday(tv, NULL);
            ntp.sync_counter++;
        }
        sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
    } else
        logger.printfln("This sync mode is not supported.");
    ntp_sync_cb(tv);
}

void NTP::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(true)},
        {"use_dhcp", Config::Bool(true)},
        {"timezone", Config::Str("Europe/Berlin", 0, 32)}, // Longest is America/Argentina/ComodRivadavia = 32 chars
        {"server", Config::Str("time.cloudflare.com", 0, 64)}, // We've applied for a vendor zone @ pool.ntp.org, however this seems to take quite a while. Use Cloudflare's public anycast servers for now.
        {"server2", Config::Str("time.google.com", 0, 64)}, // Google's public anycast servers as backup.
    }), [](Config &conf, ConfigSource source) -> String {
        if (lookup_timezone(conf.get("timezone")->asEphemeralCStr()) == nullptr)
            return "Can't update config: Failed to look up timezone.";
        return "";
    }};

    state = Config::Object({
        {"synced", Config::Bool(false)},
        {"time", Config::Uint32(0)} // unix timestamp in minutes
    });
}

struct sntp_opts {
    const String &server1;
    const String &server2;
    const bool set_servers_from_dhcp;
};

void NTP::setup()
{
    initialized = true;

    api.restorePersistentConfig("ntp/config", &config);

    const char *tzstring = lookup_timezone(config.get("timezone")->asEphemeralCStr());

    if (tzstring == nullptr) {
        logger.printfln("Failed to look up timezone information for %s. Will not set timezone", config.get("timezone")->asEphemeralCStr());
        return;
    }
    setenv("TZ", tzstring, 1);
    tzset();
    logger.printfln("Set timezone to %s", config.get("timezone")->asEphemeralCStr());

    if (config.get("enable")->asBool()) {
        bool set_servers_from_dhcp = config.get("use_dhcp")->asBool();

        // Keep local copies of ephemeral conf Strings because the SNTP lib doesn't create its own copies and holds references to whatever we pass to it.
        ntp_server1 = config.get("server")->asString();
        ntp_server2 = config.get("server2")->asString();

        sntp_opts sntp_opts = {ntp_server1, ntp_server2, set_servers_from_dhcp};

        // Enable network stack before setting any SNTP options.
        // Cannot be called via esp_netif_tcpip_exec() because that function
        // will fail if the network stack is started during its execution.
        // It should be safe to set SNTP options without the network stack
        // running, but it needs to be running to send any SNTP queries anyway.
        esp_netif_init();

        esp_netif_tcpip_exec([](void *ctx) -> esp_err_t {
            const struct sntp_opts *opts = static_cast<struct sntp_opts *>(ctx);

            // As we use our own sntp_sync_time function, we do not need to register the cb function.
            // sntp_set_time_sync_notification_cb(ntp_sync_cb);

            sntp_servermode_dhcp(opts->set_servers_from_dhcp);

            if (sntp_enabled()) {
                sntp_stop();
            }

            sntp_setoperatingmode(SNTP_OPMODE_POLL);
            sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

            // Always set first two NTP server slots and don't leave any room for
            // servers received via DHCP. If NTP via DHCP is enabled and NTP
            // servers are recieved via DHCP, all previously set servers are removed.
            // Always set both servers, even when only one is configured, as the SNTP
            // client will query all servers round-robin, including unset ones.
            sntp_setservername(0, opts->server1.isEmpty() ? opts->server2.c_str() : opts->server1.c_str());
            sntp_setservername(1, opts->server2.isEmpty() ? opts->server1.c_str() : opts->server2.c_str());

            sntp_init();

            return ESP_OK;
        }, &sntp_opts);
    }
}

void NTP::set_synced()
{
    gettimeofday(&last_sync, NULL);
    ntp.state.get("synced")->updateBool(last_sync.tv_sec > build_timestamp());
}

void NTP::register_urls()
{
    api.addPersistentConfig("ntp/config", &config);
    api.addState("ntp/state", &state);

    task_scheduler.scheduleWithFixedDelay([this]() {
        struct timeval time;
        gettimeofday(&time, NULL);
        if (time.tv_sec - this->last_sync.tv_sec >= NTP_DESYNC_THRESHOLD_S)
            ntp.state.get("synced")->updateBool(false);
    }, 0, 30 * 1000);
}
