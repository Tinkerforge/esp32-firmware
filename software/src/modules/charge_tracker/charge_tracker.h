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
#include "../web_server/web_server.h"
#include "async_https_client.h"

#define CHARGE_TRACKER_MAX_REPAIR 200
#define CHARGE_RECORD_FOLDER "/charge-records"
#define MAX_RETRY_COUNT 10
#define BASE_RETRY_DELAY_MINUTES 5
struct SendChargeLogArgs;

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
    ConfigRoot pdf_letterhead_config;

    std::mutex records_mutex;
    std::mutex pdf_mutex;

    bool pdf_send_in_progress = false;

private:
    bool repair_last(float);
    void repair_charges();
    void generate_pdf(std::function<int(const void *data, size_t len, bool last_data)> &&callback, int user_filter, uint32_t start_timestamp_min, uint32_t end_timestamp_min, uint32_t current_timestamp_min, bool english, const char *letterhead, int letterhead_lines, WebServerRequest *request);
    void send_pdf(SendChargeLogArgs &&args);

    Config last_charges_prototype;
    Config charge_log_send_prototype;
};

struct SendChargeLogArgs {
    ChargeTracker *that = nullptr;
    int user_idx = 0;
    uint32_t last_month_start_min = 0;
    uint32_t last_month_end_min = 0;
    uint32_t upload_retry_count = 0;
    millis_t next_retry_delay = millis_t(0);
    std::shared_ptr<uint64_t> task_id = nullptr;
    std::shared_ptr<AsyncHTTPSClient> remote_client = nullptr;

    SendChargeLogArgs(){};
    SendChargeLogArgs(const SendChargeLogArgs &other) {
        that = other.that;
        user_idx = other.user_idx;
        last_month_start_min = other.last_month_start_min;
        last_month_end_min = other.last_month_end_min;
        upload_retry_count = other.upload_retry_count;
        next_retry_delay = other.next_retry_delay;
        task_id = other.task_id;
        remote_client = other.remote_client;
    };
};
