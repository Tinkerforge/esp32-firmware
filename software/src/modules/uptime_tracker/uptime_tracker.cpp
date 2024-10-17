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

#include "uptime_tracker.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

#define MAX_UPTIMES 10

RTC_NOINIT_ATTR uptime_data_t data;

static bool verify_data(uint8_t *data, uint16_t checksum)
{
    if (internet_checksum(data, sizeof(uint32_t)) == checksum)
        return true;
    return false;
}

void UptimeTracker::pre_setup()
{
    uptimes_prototype = Config::Object({
        {"reset_reason", Config::Uint8(0)},
        {"boot_count", Config::Uint32(0)},
        {"timestamp_min", Config::Uint32(0)},
        {"uptime", Config::Uint32(0)},
        {"uptime_overflows", Config::Uint32(0)},
    });

    uptimes = Config::Array(
        {},
        &uptimes_prototype,
        0, MAX_UPTIMES, Config::type_id<Config::ConfObject>()
    );
}

void UptimeTracker::setup()
{
    old_uptime = data;

    data.overflow_count = 0;
    data.uptime = millis();

    verified = verify_data((uint8_t *)&old_uptime.uptime, old_uptime.checksum);

    if (!verified)
        data.boot_count = 0;
    data.boot_count++;

    api.restorePersistentConfig("info/last_boots", &uptimes);

    initialized = true;

    task_scheduler.scheduleOnce([this]() {
        struct timeval timestamp;

        if (verified)
        {
            if (uptimes.count() >= MAX_UPTIMES)
                uptimes.remove(0);
            auto last_uptime = uptimes.add();

            //timestamp_min initialized with 0. 0 means not synced
            if (rtc.clock_synced(&timestamp))
                last_uptime->get("timestamp_min")->updateUint((timestamp.tv_sec - millis() / 1000) / 60);

            last_uptime->get("reset_reason")->updateUint(esp_reset_reason());
            last_uptime->get("uptime")->updateUint(old_uptime.uptime);
            last_uptime->get("uptime_overflows")->updateUint(old_uptime.overflow_count);
            last_uptime->get("boot_count")->updateUint(data.boot_count);

            api.writeConfig("info/last_boots", &uptimes);

            logger.printfln("Wrote last uptime to flash");
        }

    }, 5_m);

    task_scheduler.scheduleWithFixedDelay([this]() {
            uint32_t tmp = data.uptime;

            data.uptime = millis();
            data.checksum = internet_checksum((uint8_t *)&data.uptime, sizeof(uint32_t));
            if (tmp > data.uptime)
                data.overflow_count++;
        }, 10_s);
}

void UptimeTracker::register_urls()
{
    api.addState("info/last_boots", &uptimes);
}
