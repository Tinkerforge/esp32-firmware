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

#include "rtc_update_api.h"

#include <time.h>
#include "musl_libc_timegm.h"

#include "module_dependencies.h"

void RtcUpdateApi::pre_setup()
{
    time_update = rtc.time;

    config = Config::Object({
        {"auto_sync", Config::Bool(true)},
    });
}

void RtcUpdateApi::setup()
{
    api.restorePersistentConfig("rtc/config", &config);

    initialized = true;
}

void RtcUpdateApi::register_urls() {
    api.addPersistentConfig("rtc/config", &config);

    api.addCommand("rtc/time_update", &time_update, {}, [this]() {
        struct tm tm;
        tm.tm_year = time_update.get("year")->asUint() - 1900;
        tm.tm_mon  = time_update.get("month")->asUint() - 1;
        tm.tm_mday = time_update.get("day")->asUint();
        tm.tm_hour = time_update.get("hour")->asUint();
        tm.tm_min  = time_update.get("minute")->asUint();
        tm.tm_sec  = time_update.get("second")->asUint();
        tm.tm_wday = time_update.get("weekday")->asUint();
        struct timeval timeval;
        timeval.tv_sec = timegm(&tm);

        rtc.push_system_time(timeval, Rtc::Quality::Low);
    }, true);
}
