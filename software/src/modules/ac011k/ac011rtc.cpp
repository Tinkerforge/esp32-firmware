/* esp32-firmware
 * Copyright (C) 2022 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "ac011rtc.h"
#include "build.h"
#include "esp_sntp.h"
#include "modules.h"
#include <ctime>
#include "time.h"

extern API api;
extern TaskScheduler task_scheduler;
extern NTP ntp;

#define RTC_DESYNC_THRESHOLD_S 25 * 60 * 60

void Rtc::pre_setup()
{
    time = Config::Object({
        {"year", Config::Uint16(0)},
        {"month", Config::Uint8(0)},
        {"day", Config::Uint8(0)},
        {"hour", Config::Uint8(0)},
        {"minute", Config::Uint8(0)},
        {"second", Config::Uint8(0)},
        // Don't add the centiseconds here, this would send this API via Websockets/MQTT
        // 4 times per second (as the RTC bricklet polls the real time clock every 250 ms)
        //{"centisecond", Config::Uint8(0)},
        {"weekday", Config::Uint8(0)},
    });

    time_update = Config::Object({
        {"year", Config::Uint16(0)},
        {"month", Config::Uint8(0)},
        {"day", Config::Uint8(0)},
        {"hour", Config::Uint8(0)},
        {"minute", Config::Uint8(0)},
        {"second", Config::Uint8(0)},
        {"centisecond", Config::Uint8(0)},
        {"weekday", Config::Uint8(0)},
    });

    config = Config::Object({
        {"auto_sync", Config::Bool(true)},
    });

    state = Config::Object({
        {"synced", Config::Bool(false)},
        {"time", Config::Uint32(0)} // unix timestamp in minutes
    });
}

void Rtc::set_time(timeval time)
{
    struct tm date_time;
    gmtime_r(&time.tv_sec, &date_time);

    tf_real_time_clock_v2_set_date_time(
        date_time.tm_year + 1900,
        date_time.tm_mon + 1,
        date_time.tm_mday,
        date_time.tm_hour,
        date_time.tm_min,
        date_time.tm_sec);

    rtc_updated = true;
    logger.printfln("Setting the RTC clock...");
}

time_t Rtc::get_time(bool reset_update)
{
    rtc_updated = !reset_update;
    return rtcunixtime;
}

void Rtc::tf_real_time_clock_v2_set_date_time(uint year, uint month, uint day, uint hour, uint minute, uint second)
{
    tm tv;
    tv.tm_year  = year - 1900;
    tv.tm_mon   = month - 1;
    tv.tm_mday  = day;
    tv.tm_hour  = hour;
    tv.tm_min   = minute;
    tv.tm_sec   = second;
    tv.tm_isdst = -1;
    rtcunixtime = mktime(&tv);
}

void Rtc::set_synced()
{
    gettimeofday(&last_sync, NULL);
    state.get("synced")->updateBool(true);
    state.get("time")->updateUint(timestamp_minutes());
    ntp.set_synced();
}

void Rtc::register_urls()
{
    api.addPersistentConfig("rtc/config", &config, {}, 1000);

    api.addState("rtc/time", &time, {}, 100);
    api.addState("rtc/state", &state, {}, 1000);
    api.addCommand("rtc/time_update", &time_update, {}, [this]() {
        tf_real_time_clock_v2_set_date_time(time_update.get("year")->asUint(),
                                            time_update.get("month")->asUint(),
                                            time_update.get("day")->asUint(),
                                            time_update.get("hour")->asUint(),
                                            time_update.get("minute")->asUint(),
                                            time_update.get("second")->asUint());
        rtc_updated = true;
        set_synced();
    }, true);

    api.addFeature("rtc");

    task_scheduler.scheduleWithFixedDelay([this]() {
        struct timeval t;
        struct tm date_time;
        gettimeofday(&t, nullptr);
        gmtime_r(&t.tv_sec, &date_time);
        time.get("year")->updateUint(date_time.tm_year + 1900);
        time.get("month")->updateUint(date_time.tm_mon + 1);
        time.get("day")->updateUint(date_time.tm_mday);
        time.get("hour")->updateUint(date_time.tm_hour);
        time.get("minute")->updateUint(date_time.tm_min);
        time.get("second")->updateUint(date_time.tm_sec);
        time.get("weekday")->updateUint(date_time.tm_wday);
    }, 0, 200);

    task_scheduler.scheduleWithFixedDelay([this]() {
        struct timeval time;
        gettimeofday(&time, NULL);
        if (time.tv_sec - this->last_sync.tv_sec >= RTC_DESYNC_THRESHOLD_S || time.tv_sec < build_timestamp()) {
            ntp.state.get("synced")->updateBool(false);
            state.get("synced")->updateBool(false);
        }
    }, 0, 30 * 1000);
}

void Rtc::setup()
{
    api.restorePersistentConfig("rtc/config", &config);
    initialized = true;
}
