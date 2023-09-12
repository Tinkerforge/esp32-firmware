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
#include "module_dependencies.h"

#include "build.h"
#include <ctime>

#include "gcc_warnings.h"

bool RtcBricklet::update_system_time()
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

void RtcBricklet::setup()
{
    setup_rtc();

    if (!device_found)
        return;
}

void RtcBricklet::register_urls() {
    if (!device_found)
        return;

    rtc.register_backend(this);
}

void RtcBricklet::set_time(const tm &date_time)
{
    uint16_t year = static_cast<uint16_t>(date_time.tm_year + 1900);
    uint8_t  mon  = static_cast<uint8_t >(date_time.tm_mon + 1);
    uint8_t  day  = static_cast<uint8_t >(date_time.tm_mday);
    uint8_t  hour = static_cast<uint8_t >(date_time.tm_hour);
    uint8_t  min  = static_cast<uint8_t >(date_time.tm_min);
    uint8_t  sec  = static_cast<uint8_t >(date_time.tm_sec);
    uint8_t  wday = static_cast<uint8_t >(date_time.tm_wday);

    // Bricklet expects Sunday to be 7, but tm_wday is specified to use 0 for Sunday.
    if (wday == 0)
        wday = 7;

    auto ret = tf_real_time_clock_v2_set_date_time(&device, year, mon, day, hour, min, sec, 0, wday);
    if (ret)
        logger.printfln("Setting RTC to %04u-%02u-%02u %02u:%02u:%02u (wd %i) failed with code %i", year, mon, day, hour, min, sec, wday, ret);
}

void RtcBricklet::set_time(const timeval &time)
{
    struct tm date_time;
    gmtime_r(&time.tv_sec, &date_time);

    set_time(date_time);
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
    time.tv_usec = static_cast<suseconds_t>(ts % 1000) * 1000;
    time.tv_sec  = static_cast<time_t>(ts / 1000);

    time.tv_sec += 946684800;

    // FIXME not Y2038-safe
    if (time.tv_sec < static_cast<time_t>(build_timestamp() - 14 * 3600))
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
