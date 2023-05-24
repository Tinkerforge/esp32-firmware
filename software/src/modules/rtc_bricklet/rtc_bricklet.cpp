/* esp32-firmware
 * Copyright (C) 2022 - 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "rtc_bricklet.h"

#include "build.h"
#include "esp_sntp.h"
#include "api.h"
#include "modules.h"
#include "task_scheduler.h"
#include <ctime>

void RtcBricklet::update_system_time()
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
    if (t.tv_sec == 0 && t.tv_usec == 0)
        return;

    {
        std::lock_guard<std::mutex> lock{ntp.mtx};
        if (count != ntp.sync_counter)
            // NTP has just updated the system time. We assume that this time is more accurate the the RTC's.
            return;

        settimeofday(&t, nullptr);
        ntp.set_synced();
    }
}

void RtcBricklet::setup()
{
    setup_rtc();

    if (!device_found)
        return;

    rtc.register_backend(this);
}

void RtcBricklet::set_time(const tm &date_time)
{
    auto ret = tf_real_time_clock_v2_set_date_time(&device,
                                                   date_time.tm_year + 1900,
                                                   date_time.tm_mon + 1,
                                                   date_time.tm_mday,
                                                   date_time.tm_hour,
                                                   date_time.tm_min,
                                                   date_time.tm_sec,
                                                   0,
                                                   date_time.tm_wday);
    if (ret)
        logger.printfln("Setting rtc failed with code %i", ret);
}

void RtcBricklet::set_time(const timeval &time)
{
    struct tm date_time;
    gmtime_r(&time.tv_sec, &date_time);

    auto ret = tf_real_time_clock_v2_set_date_time(&device,
                                                   date_time.tm_year +1900,
                                                   date_time.tm_mon + 1,
                                                   date_time.tm_mday,
                                                   date_time.tm_hour,
                                                   date_time.tm_min,
                                                   date_time.tm_sec,
                                                   0,
                                                   date_time.tm_wday);
    if (ret)
        logger.printfln("Setting rtc failed with code %i", ret);
}

struct timeval RtcBricklet::get_time()
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

    if (time.tv_sec < build_timestamp())
    {
        struct timeval tmp;
        tmp.tv_sec = 0;
        tmp.tv_usec = 0;
        return tmp;
    }
    return time;
}

void RtcBricklet::setup_rtc()
{
    if (!this->DeviceModule::setup_device())
        return;

    tf_real_time_clock_v2_set_response_expected(&device, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_DATE_TIME, true);

    initialized = true;
}

void RtcBricklet::reset()
{
    DeviceModule::reset();
}
