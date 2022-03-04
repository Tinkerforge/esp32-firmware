/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include <LittleFS.h>

#include "config.h"

class ChargeTracker {
public:
    ChargeTracker();
    void setup();
    void register_urls();
    void loop();

    bool initialized = false;

    uint32_t first_charge_record;
    uint32_t last_charge_record;

    String chargeRecordFilename(uint32_t i);
    void startCharge(uint32_t timestamp_minutes, float meter_start, uint8_t user_id, uint32_t evse_uptime);
    void endCharge(uint32_t charge_duration_seconds, float meter_end);
    void removeOldRecords();
    bool setupRecords();
    void updateState();

    size_t completeRecordsInLastFile();
    bool currentlyCharging();

    void readNRecords(File *f, size_t records_to_read);

    ConfigRoot last_charges;
    ConfigRoot current_charge;
    ConfigRoot state;

    std::mutex records_mutex;
};
