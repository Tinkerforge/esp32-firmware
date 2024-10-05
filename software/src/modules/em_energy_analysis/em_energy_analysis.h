/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <list>
#include <stdint.h>
#include <time.h>

#include "chunked_response.h"
#include "config.h"
#include "module.h"
#include "modules/em_common/structs.h"

class EMEnergyAnalysis final : public IModule
{
public:
    EMEnergyAnalysis() {}

    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

private:
    void update_history_meter_power(uint32_t slot, float power /* W */);
    void collect_data_points();
    void set_pending_data_points();
    bool load_persistent_data();
    void load_persistent_data_v1(uint8_t *buf);
    void load_persistent_data_v2(uint8_t *buf);
    void save_persistent_data();
    void history_wallbox_5min_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    void history_wallbox_daily_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    void history_energy_manager_5min_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    void history_energy_manager_daily_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);
    bool set_wallbox_5min_data_point(const struct tm *utc, const struct tm *local, uint32_t uid, uint16_t flags, uint16_t power /* W */);
    bool set_wallbox_daily_data_point(const struct tm *local, uint32_t uid, uint32_t energy /* daWh */);
    bool set_energy_manager_5min_data_point(const struct tm *utc, const struct tm *local, uint16_t flags, const int32_t power[7] /* W */,
                                            const int32_t price /* mct/kWh */);
    bool set_energy_manager_daily_data_point(const struct tm *local, const uint32_t energy_import[7] /* daWh */, const uint32_t energy_export[7] /* daWh */,
                                             int32_t price_min /* ct/kWh */, int32_t price_avg /* ct/kWh */, int32_t price_max /* ct/kWh */);

    std::list<std::function<bool(void)>> pending_data_points;
    bool persistent_data_loaded = false;
    bool show_blank_value_id_update_warnings = false;
    uint32_t last_history_5min_slot = 0;
    ConfigRoot history_wallbox_5min;
    ConfigRoot history_wallbox_daily;
    ConfigRoot history_energy_manager_5min;
    ConfigRoot history_energy_manager_daily;
    bool history_meter_setup_done[METERS_SLOTS];
    float history_meter_power_value[METERS_SLOTS]; // W
    uint32_t history_meter_power_timestamp[METERS_SLOTS];
    double history_meter_power_sum[METERS_SLOTS] = {0}; // watt seconds
    double history_meter_power_duration[METERS_SLOTS] = {0}; // seconds
    double history_meter_energy_import[METERS_SLOTS] = {0}; // daWh
    double history_meter_energy_export[METERS_SLOTS] = {0}; // daWh
    uint32_t history_request_seqnum = 0;

    // Cached EM data
    const EMAllDataCommon *all_data_common;
};
