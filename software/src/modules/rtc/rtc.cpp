/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include <time.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "musl_libc_timegm.h"

void IRtcBackend::set_time(const timeval &time)
{
    struct tm tm;
    gmtime_r(&time.tv_sec, &tm);
    set_time(tm);
}

void IRtcBackend::set_time(const tm &time)
{
    struct tm copy = time;
    struct timeval timeval;
    timeval.tv_sec = timegm(&copy);
    timeval.tv_usec = 0;
    set_time(timeval);
}

void Rtc::pre_setup()
{
    time = Config::Object({
        {"year", Config::Uint16(0)},
        {"month", Config::Uint8(0)},
        {"day", Config::Uint8(0)},
        {"hour", Config::Uint8(0)},
        {"minute", Config::Uint8(0)},
        {"second", Config::Uint8(0)},
        {"weekday", Config::Uint8(0)},
    });

    time_update = Config::Object({
        {"year", Config::Uint16(0)},
        {"month", Config::Uint8(0)},
        {"day", Config::Uint8(0)},
        {"hour", Config::Uint8(0)},
        {"minute", Config::Uint8(0)},
        {"second", Config::Uint8(0)},
        {"weekday", Config::Uint8(0)},
    });

    config = Config::Object({
        {"auto_sync", Config::Bool(true)},
    });
}

void Rtc::setup()
{
    api.restorePersistentConfig("rtc/config", &config);

    initialized = true;
}

void Rtc::register_backend(IRtcBackend *_backend)
{
    if (boot_stage < BootStage::REGISTER_URLS)
        esp_system_abort("Registering RTC backends before the register URLs stage is not allowed!");

    if (backend || !_backend)
        return;

    backend = _backend;

    api.addPersistentConfig("rtc/config", &config);

    api.addState("rtc/time", &time, {}, true);
    api.addCommand("rtc/time_update", &time_update, {}, [this]() {
        if (!backend)
            return;
        tm tm;
        tm.tm_year = time_update.get("year")->asUint() - 1900;
        tm.tm_mon = time_update.get("month")->asUint() - 1;
        tm.tm_mday = time_update.get("day")->asUint();
        tm.tm_hour = time_update.get("hour")->asUint();
        tm.tm_min = time_update.get("minute")->asUint();
        tm.tm_sec = time_update.get("second")->asUint();
        tm.tm_wday = time_update.get("weekday")->asUint();

        logger.printfln("Setting RTC time to %u-%02u-%02u %02u:%02u:%02u UTC",
                        tm.tm_year + 1900,
                        tm.tm_mon + 1,
                        tm.tm_mday,
                        tm.tm_hour,
                        tm.tm_min,
                        tm.tm_sec);

        backend->set_time(tm);
        update_system_time();
    }, true);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (!backend)
            return;

        timeval tv = backend->get_time();
        if (tv.tv_sec == 0)
            return;

        tm tm;
        gmtime_r(&tv.tv_sec, &tm);

        time.get("year")->updateUint(tm.tm_year + 1900);
        time.get("month")->updateUint(tm.tm_mon + 1);
        time.get("day")->updateUint(tm.tm_mday);
        time.get("hour")->updateUint(tm.tm_hour);
        time.get("minute")->updateUint(tm.tm_min);
        time.get("second")->updateUint(tm.tm_sec);
        time.get("weekday")->updateUint(tm.tm_wday);
    }, 0, 200);

    api.addFeature("rtc");

    task_scheduler.scheduleWithFixedDelay([this]() {
        update_system_time();
    }, 1000 * 60 * 10, 1000 * 60 * 10);

    struct timeval tv_now;
    if (clock_synced(&tv_now)) {
        // Got NTP sync before RTC init.
        set_time(tv_now);
    } else {
        if (update_system_time()) {
            auto now = millis();
            auto secs = now / 1000;
            auto ms = now % 1000;
            logger.printfln("Set system time from RTC at %lu,%03lu", secs, ms);
        } else {
            logger.printfln("RTC not set!");
        }
    }
}

void Rtc::reset()
{
    if (!backend)
        return;
    backend->reset();
}

void Rtc::set_time(const timeval &_time)
{
    if (!backend)
        return;
    backend->set_time(_time);
}

timeval Rtc::get_time()
{
    if (!backend)
        return timeval{0, 0};
    return backend->get_time();
}

bool Rtc::update_system_time()
{
    if (!backend)
        return false;

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
    if (t.tv_sec == 0 && t.tv_usec == 0)
        return false;

    {
        std::lock_guard<std::mutex> lock{ntp.mtx};
        if (count != ntp.sync_counter)
            // NTP has just updated the system time. We assume that this time is more accurate the the RTC's.
            return false;

        settimeofday(&t, nullptr);
        ntp.set_synced();
    }
    return true;
}
