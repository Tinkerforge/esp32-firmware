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

#include "module.h"
#include "config.h"

#define CHARGE_TRACKER_MAX_REPAIR 200
#define CHARGE_RECORD_FOLDER "/charge-records"

class ChargeTracker final : public IModule
{
public:
    ChargeTracker(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    uint32_t first_charge_record;
    uint32_t last_charge_record;

    String chargeRecordFilename(uint32_t i);
    bool startCharge(uint32_t timestamp_minutes, float meter_start, uint8_t user_id, uint32_t evse_uptime, uint8_t auth_type, Config::ConfVariant auth_info);
    void endCharge(uint32_t charge_duration_seconds, float meter_end);
    void removeOldRecords();
    bool setupRecords();
    void updateState();
    bool is_user_tracked(uint8_t user_id);

    size_t completeRecordsInLastFile();
    bool currentlyCharging();

    void readNRecords(File *f, size_t records_to_read);

    ConfigRoot last_charges;
    ConfigRoot current_charge;
    ConfigRoot state;

    ConfigRoot config;

    std::mutex records_mutex;
    std::mutex pdf_mutex;

private:
    bool repair_last(float);
    void repair_charges();
};
