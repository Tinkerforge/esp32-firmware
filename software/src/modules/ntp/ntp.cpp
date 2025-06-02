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
#include <esp_netif.h>
#include <esp_sntp.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "timezone_translation.h"

#include "gcc_warnings.h"

extern "C" void sntp_sync_time(struct timeval *tv)
{
#if MODULE_RTC_AVAILABLE()
    rtc.push_system_time(*tv, Rtc::Quality::High);
#elif
    settimeofday(&time, NULL);
#endif

    ntp.time_synced_NTPThread();
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

void NTP::setup()
{
    initialized = true;

    api.restorePersistentConfig("ntp/config", &config);

    const char *timezone = config.get("timezone")->asUnsafeCStr();
    const char *tzstring = lookup_timezone(timezone);

    if (!tzstring) {
        logger.printfln("Failed to look up timezone information for %s. Will not set timezone", timezone);
        return;
    }
    setenv("TZ", tzstring, 1);
    tzset();
    logger.printfln("Set timezone to %s", timezone);

    if (!config.get("enable")->asBool()) {
        return;
    }

    // Keep local copies of unsafe ConfStrings because the SNTP lib doesn't create its own copies and holds references to whatever is passed to it.
    ntp_server1 = config.get("server" )->asString();
    ntp_server2 = config.get("server2")->asString();

    const bool set_servers_from_dhcp = config.get("use_dhcp")->asBool();

    // Enable network stack before setting any SNTP options.
    // It should be safe to set SNTP options without the network stack
    // running, but it needs to be running to send any SNTP queries anyway.
    esp_netif_init();

    if (esp_sntp_enabled()) {
        esp_sntp_stop();
    }

    // As we use our own sntp_sync_time function, we do not need to register the cb function.
    // sntp_set_time_sync_notification_cb(ntp_sync_cb);

    esp_sntp_servermode_dhcp(set_servers_from_dhcp);
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);

    // Always set first two NTP server slots and don't leave any room for
    // servers received via DHCP. If NTP via DHCP is enabled and NTP
    // servers are received via DHCP, all previously set servers are removed.
    // Always set both servers, even when only one is configured, as the SNTP
    // client will query all servers round-robin, including unset ones.
    esp_sntp_setservername(0, ntp_server1.isEmpty() ? ntp_server2.c_str() : ntp_server1.c_str());
    esp_sntp_setservername(1, ntp_server2.isEmpty() ? ntp_server1.c_str() : ntp_server2.c_str());

#if MODULE_NETWORK_AVAILABLE()
    if (set_servers_from_dhcp) {
        // To hook into DHCP, SNTP must be initialized before starting Ethernet or WiFi.
        esp_sntp_init();
    }
#else
    esp_sntp_init();
#endif
}

void NTP::register_events()
{
#if MODULE_NETWORK_AVAILABLE()
    if (config.get("enable")->asBool() && !config.get("use_dhcp")->asBool()) {
        network.on_network_connected([this](const Config *connected) {
            if (connected->asBool()) {
                esp_sntp_init();

                return EventResult::Deregister;
            }

            return EventResult::OK;
        });
    }
#endif
}

void NTP::set_synced(bool synced)
{
    this->state.get("synced")->updateBool(synced);
}

void NTP::set_api_time(struct timeval time) {
    state.get("time")->updateUint(static_cast<uint32_t>(time.tv_sec / 60)); // This will overflow in 10135 CE, which seems safe enough.
}

void NTP::register_urls()
{
    api.addPersistentConfig("ntp/config", &config);
    api.addState("ntp/state", &state);
}

void NTP::time_synced_NTPThread() {
    micros_t now = now_us();

    if (sync_expires_at == 0_us) {
        uint32_t now_u32 = now.to<millis_t>().as<uint32_t>();

        task_scheduler.scheduleOnce([this, now_u32]() {
            this->set_synced(true);

            uint32_t secs = now_u32 / 1000;
            uint32_t ms   = now_u32 % 1000;
            // Don't log in TCP/IP task: Deadlocks the event lock
            logger.printfln("NTP synchronized at %lu,%03lu", secs, ms);
        });

        task_scheduler.scheduleWithFixedDelay([this]() {
            if (deadline_elapsed(this->sync_expires_at)) {
                this->set_synced(false);
            }
        }, 1_h, 1_h);
    }

    sync_expires_at = now + 25_h;
}
