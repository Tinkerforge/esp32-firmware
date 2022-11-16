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

#include "rtc.h"
#include "build_timestamp.h"
#include "esp_sntp.h"
#include "modules.h"
#include <ctime>

extern API api;
extern TaskScheduler task_scheduler;
extern NTP ntp;

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
        {"sync_enabled", Config::Bool(true)},
    });
}

void Rtc::update_system_time()
{
    // We have to make sure, we don't try to update the system clock
    // while NTP also sets the clock.
    // To prevent this, we skip updating the system clock if NTP
    // did update it while we were fetching the current time from the RTC.

    uint32_t count;
    {
        std::lock_guard<std::mutex> lock{ntp.mtx};
        count = ntp.sync_counter;
    }

    struct timeval t = this->get_time();

    {
        std::lock_guard<std::mutex> lock{ntp.mtx};
        if (count != ntp.sync_counter)
            // NTP has just updated the system time. We assume that this time is more accurate the the RTC's.
            return;

        settimeofday(&t, nullptr);
    }
}

void Rtc::setup()
{
    setup_rtc();

    if (!device_found)
        return;

    logger.printfln("RTC Bricklet found");

    struct timeval time = get_time();
    if (time.tv_sec != 0) {
        settimeofday(&time, nullptr);

        auto now = millis();
        auto secs = now / 1000;
        auto ms = now % 1000;
        logger.printfln("Set system time from RTC at %lu,%03lu", secs, ms);
    } else {
        logger.printfln("RTC Bricklet not set!");
    }

    api.restorePersistentConfig("rtc/config", &config);
}

void Rtc::set_time(timeval time)
{
    struct tm date_time;
    gmtime_r(&time.tv_sec, &date_time);

    date_time.tm_year += 1900;
    auto ret = tf_real_time_clock_v2_set_date_time(&device,
                                                   date_time.tm_year,
                                                   date_time.tm_mon + 1,
                                                   date_time.tm_mday,
                                                   date_time.tm_hour,
                                                   date_time.tm_min,
                                                   date_time.tm_sec,
                                                   time.tv_usec / (1000 * 10),
                                                   date_time.tm_wday);
    if (ret)
        logger.printfln("Setting rtc failed with code %i", ret);

    ntp.set_synced();
}

struct timeval Rtc::get_time()
{
    int64_t ts;
    int ret = tf_real_time_clock_v2_get_timestamp(&device, &ts);
    if (ret)
    {
        logger.printfln("Reading RTC failed with code %i", ret);
        struct timeval tmp;
        tmp.tv_sec = 0;
        tmp.tv_usec = 0;
        return tmp;
    }

    struct timeval time;
    time.tv_usec = ts % 1000 * 1000;
    time.tv_sec = ts / 1000;

    time.tv_sec += 946684800;

    if (time.tv_sec < BUILD_TIMESTAMP)
    {
        struct timeval tmp;
        tmp.tv_sec = 0;
        tmp.tv_usec = 0;
        return tmp;
    }
    return time;
}

void Rtc::register_urls()
{
    if (!device_found)
        return;

    DeviceModule::register_urls();

    api.addPersistentConfig("rtc/config", &config, {}, 1000);

    api.addState("rtc/time", &time, {}, 100);
    api.addCommand("rtc/time_update", &time_update, {}, [this]() {
        auto ret = tf_real_time_clock_v2_set_date_time(&device,
                                                       time_update.get("year")->asUint(),
                                                       time_update.get("month")->asUint(),
                                                       time_update.get("day")->asUint(),
                                                       time_update.get("hour")->asUint(),
                                                       time_update.get("minute")->asUint(),
                                                       time_update.get("second")->asUint(),
                                                       time_update.get("centisecond")->asUint(),
                                                       time_update.get("weekday")->asUint());
        if (ret != TF_E_OK) {
            logger.printfln("Failed to update RTC via API. (rc %d)", ret);
            return;
        }

        ntp.set_synced();
        update_system_time();
    }, true);

    api.addFeature("rtc");

    task_scheduler.scheduleWithFixedDelay([this]() {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t centisecond;
        uint8_t weekday;

        auto ret = tf_real_time_clock_v2_get_date_time(&device,
                                                       &year,
                                                       &month,
                                                       &day,
                                                       &hour,
                                                       &minute,
                                                       &second,
                                                       &centisecond,
                                                       &weekday,
                                                       NULL);
        if (ret != TF_E_OK) {
            logger.printfln("Update time failed (rc %d)", ret);
            return;
        }
        time.get("year")->updateUint(year);
        time.get("month")->updateUint(month);
        time.get("day")->updateUint(day);
        time.get("hour")->updateUint(hour);
        time.get("minute")->updateUint(minute);
        time.get("second")->updateUint(second);
        time.get("weekday")->updateUint(weekday);
    }, 0, 200);

    task_scheduler.scheduleWithFixedDelay([this]() {
        update_system_time();
    }, 1000 * 60 * 10, 1000 * 60 * 10);
}

void Rtc::loop()
{
}

void Rtc::setup_rtc()
{
    if (!this->DeviceModule::setup_device())
        return;

    tf_real_time_clock_v2_set_response_expected(&device, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_DATE_TIME, true);

    initialized = true;
}
