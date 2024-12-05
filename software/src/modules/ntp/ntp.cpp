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

extern "C" void sntp_sync_time(struct timeval *tv)
{
    if (sntp_get_sync_mode() != SNTP_SYNC_MODE_IMMED) {
        logger.printfln("This sync mode is not supported.");
        return;
    }

#if MODULE_RTC_AVAILABLE()
    rtc.push_system_time(*tv, Rtc::Quality::High);
#elif
    settimeofday(&time, NULL);
#endif
    sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
    ntp.last_sync = now_us();

    if (first) {
        first = false;
        auto now = millis();

        task_scheduler.scheduleOnce([now](){
            ntp.set_synced(true);

            auto secs = now / 1000;
            auto ms = now % 1000;
            // Don't log in TCP/IP task: Deadlocks the event lock
            logger.printfln("NTP synchronized at %lu,%03lu", secs, ms);
        });

        task_scheduler.scheduleWithFixedDelay([](){
            if (deadline_elapsed(ntp.last_sync + 25_h))
                ntp.set_synced(false);
        }, 1_h, 1_h);
    }
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

void NTP::set_synced(bool synced)
{
    this->state.get("synced")->updateBool(synced);
}

void NTP::set_api_time(struct timeval time) {
    state.get("time")->updateUint(time.tv_sec / 60);
}

void NTP::register_urls()
{
    api.addPersistentConfig("ntp/config", &config);
    api.addState("ntp/state", &state);
}
