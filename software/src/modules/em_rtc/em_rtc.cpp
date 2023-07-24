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

#include "em_rtc.h"
#include "module_dependencies.h"

void EmRtc::setup()
{
    if (!energy_manager.initialized)
        return;

    rtc.register_backend(this);

    initialized = true;
}

void EmRtc::set_time(const tm &tm)
{
    energy_manager.set_time(tm);
}

void EmRtc::set_time(const timeval &tv)
{
    tm tm;
    gmtime_r(&tv.tv_sec, &tm);
    energy_manager.set_time(tm);
}

bool EmRtc::update_system_time()
{
    // We have to make sure, we don't try to update the system clock
    // while NTP also sets the clock.
    // To prevent this, we skip updating the system clock if NTP
    // did update it while we were fetching the current time from the EM.

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
            // NTP has just updated the system time. We assume that this time is more accurate the the EMs.
            return false;

        settimeofday(&t, nullptr);
        ntp.set_synced();
    }
    return true;
}

struct timeval EmRtc::get_time()
{
    return energy_manager.get_time();
}
