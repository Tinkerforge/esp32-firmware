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
#include "modules.h"
#include "build.h"
#include "time.h"


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

    rtc_config = Config::Object({
        {"auto_sync", Config::Bool(true)},
    });
}

void Rtc::setup()
{
    api.restorePersistentConfig("rtc/config", &rtc_config);
}

void Rtc::register_urls()
{
}

void Rtc::loop() {}

void Rtc::register_backend(IRtcBackend *_backend)
{
    if (backend || !_backend)
        return;

    backend = _backend;

    api.addPersistentConfig("rtc/config", &rtc_config, {}, 1000);

    api.addState("rtc/time", &time, {}, 100);
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

        backend->set_time(tm);
        update_system_time();
    }, true);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (!backend)
            return;

        timeval tv = backend->get_time();

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
    initialized = true;

    task_scheduler.scheduleWithFixedDelay([this]() {
        update_system_time();
    }, 1000 * 60 * 10, 1000 * 60 * 10);


    timeval tv;
    if (clock_synced(&tv))
        return;

    struct timeval time = backend->get_time();
    if (time.tv_sec != 0) {
        settimeofday(&time, nullptr);

        auto now = millis();
        auto secs = now / 1000;
        auto ms = now % 1000;
        logger.printfln("Set system time from RTC at %lu,%03lu", secs, ms);
    } else {
        logger.printfln("RTC not set!");
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

void Rtc::update_system_time()
{
    if (!backend)
        return;
    backend->update_system_time();
}