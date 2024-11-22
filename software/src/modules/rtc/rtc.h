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

#pragma once

#include "module.h"
#include "config.h"

// Only implement this interface for RTC-style hardware, not for pushing time sources such as NTP or the API!
class IRtcBackend
{
public:
    IRtcBackend() {}
    virtual ~IRtcBackend() {}

    // Override exactly one of the set_time functions!
    virtual void set_time(const timeval &time);
    virtual void set_time(const tm &time, int32_t microseconds = 0);

    virtual struct timeval get_time() = 0;
    virtual void reset() = 0;
};

/*
Handles two kinds of time sources:
    - RTC-style hardware that can be set to the current time and then keep the time with (hopefully) low drift
    - Sources that push the current time but can't be queried at will. For example NTP, the API and OCPP
When the current time is pushed, this module sets the system time and writes all RTCs.
The RTC with the best quality will periodically be queried to set the system time to reduce drift.
A pushed time is only accepted if it is of better quality than the last pushed time,
or if the max age of the last push was reached. This could happen if for example the NTP server is offline.
After some time we should then accept a time set via the API.
*/
class Rtc final : public IModule
{
public:
    Rtc() {}

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void register_backend(IRtcBackend *_backend);

    void reset();

    bool clock_synced(struct timeval *out_tv_now);

    uint32_t timestamp_minutes()
    {
        struct timeval tv_now;

        if (!clock_synced(&tv_now))
            return 0;

        return tv_now.tv_sec / 60;
    }

    // Also edit get_quality_name() in rtc.cpp when modifying this!
    enum class Quality : uint8_t {
        None = 0, // Never sycned.
        RTC = 1, // Use for any RTC: Will not write RTCs. Will be ignored if the time was set for example via NTP or the API not too long ago.
        Low = 2, // Use for example for the API: Will be ignored if the time was set for example via NTP not too long ago.
        High = 3, // Use for example for NTP: A time received via NTP is more trustworthy than a time received via the API.
        Force = 255 // Use for example for OCPP: we have to accept the server's time to be sure that reported meter values etc. match the server time.
    };

    bool push_system_time(const timeval &time, Quality quality);

private:
    ConfigRoot time;
    ConfigRoot time_update;
    ConfigRoot config;

    void update_system_time_from_rtc();
    void update_rtc_from_system_time(int attempt);

    std::vector<IRtcBackend *> backends;

    Quality last_sync_quality = Quality::None;
    micros_t last_sync = -1000_h;
    micros_t last_sync_ok_deadline = -1000_h;

    std::recursive_mutex push_system_time_mutex;

    size_t trace_buf_index;
};
