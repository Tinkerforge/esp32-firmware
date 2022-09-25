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

#include <Arduino.h>

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "time.h"

#define MAX_UPTIMES 10

typedef struct uptime_data_s {
    uint16_t overflow_count;
    uint16_t checksum;
    uint32_t boot_count;
    uint32_t uptime;
} uptime_data_t;

static_assert(sizeof(uptime_data_s) == 12, "Unexpected size of uptime_data_s");

class UptimeTracker
{
    public:
        UptimeTracker() {}
        void pre_setup();
        void setup();
        void register_urls();
        void loop();

        bool initialized = false;
        bool verified = false;
        uptime_data_t old_uptime;

        ConfigRoot uptimes;
};
