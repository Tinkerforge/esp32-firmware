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

#pragma once

#include "config.h"


class Rtc
{
    private:
        struct timeval last_sync;
        time_t rtcunixtime;
        void tf_real_time_clock_v2_set_date_time(uint year, uint month, uint day, uint hour, uint minute, uint second);
    public:

        bool initialized = false;
        bool rtc_updated = false;

        void pre_setup();
        void setup();
        void loop();
        void set_synced();
        void register_urls();

        void setup_rtc();
        void set_time(timeval time);
        time_t get_time(bool reset_update);

        ConfigRoot time;
        ConfigRoot time_update;
        ConfigRoot config;
        ConfigRoot state;
};
