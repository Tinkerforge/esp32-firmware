/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

#include "energy_manager.h"
#include "module_dependencies.h"

#include <Arduino.h>
#include <sys/time.h>
#include <time.h>

#include "event_log.h"

#define MAX_DATA_AGE 30000 // milliseconds
#define DATA_INTERVAL_5MIN 5 // minutes
#define MAX_PENDING_DATA_POINTS 250

//#define DEBUG_LOGGING

void EnergyManager::register_events()
{
    event.registerEvent("meter/state", {"state"}, [this](Config *config){
        history_meter_available = config->asUint() == 2;
    });

    event.registerEvent("meter/values", {"power"}, [this](Config *config){
        update_history_meter_power(config->asFloat());
    });
}

void EnergyManager::update_history_meter_power(float power /* W */)
{
    uint32_t now = millis();

    if (!isnan(history_meter_power_value)) {
        uint32_t duration_ms;

        if (now >= history_meter_power_timestamp) {
            duration_ms = now - history_meter_power_timestamp;
        } else {
            duration_ms = UINT32_MAX - history_meter_power_timestamp + now;
        }

        double duration_s = (double)duration_ms / 1000.0;
        double energy_ws = (double)history_meter_power_value * duration_s;

        history_meter_power_sum += energy_ws;
        history_meter_power_duration += duration_s;

        if (!persistent_data_loaded) {
            persistent_data_loaded = load_persistent_data();
        }

        if (persistent_data_loaded && energy_ws != 0) {
            double energy_dwh = energy_ws / 36000.0;

            if (energy_dwh >= 0) {
                history_meter_energy_import += energy_dwh;
            } else {
                history_meter_energy_export += -energy_dwh;
            }

            save_persistent_data();
        }
    }

    history_meter_power_value = power;
    history_meter_power_timestamp = now;
}

void EnergyManager::collect_data_points()
{
    struct timeval tv;
    struct tm utc;
    struct tm local;

    if (!clock_synced(&tv)) {
        return;
    }

    if (!persistent_data_loaded) {
        persistent_data_loaded = load_persistent_data();

        if (!persistent_data_loaded) {
            return;
        }
    }

    gmtime_r(&tv.tv_sec, &utc);
    localtime_r(&tv.tv_sec, &local);

    uint32_t current_5min_slot = ((utc.tm_year * 366 + utc.tm_yday) * 24 + utc.tm_hour) * 12 + utc.tm_min / 5;

    if (current_5min_slot != last_history_5min_slot) {
        // 5min data
        for (auto &charger : charge_manager.state.get("chargers")) {
            uint32_t last_update = charger.get("last_update")->asUint();

            if (!deadline_elapsed(last_update + MAX_DATA_AGE)) {
                uint8_t charger_state = charger.get("charger_state")->asUint();

                uint32_t uid = charger.get("uid")->asUint();
                uint8_t flags = charger_state; // bit 0-2 = charger state, bit 7 = no data (read only)
                uint16_t power = UINT16_MAX;

                if (charger.get("meter_supported")->asBool()) {
                    float power_total_sum = charger.get("power_total_sum")->asFloat();
                    uint32_t power_total_count = charger.get("power_total_count")->asUint();

                    charger.get("power_total_sum")->updateFloat(0);
                    charger.get("power_total_count")->updateUint(0);

                    if (power_total_count > 0) {
                        power = clamp<uint64_t>(0,
                                                roundf(power_total_sum / power_total_count),
                                                UINT16_MAX - 1); // W
                    }
                }

                if (pending_data_points.size() > MAX_PENDING_DATA_POINTS) {
                    logger.printfln("energy_manager: Data point queue is full, dropping new data point");
                }
                else {
                    pending_data_points.push_back([this, utc, local, uid, flags, power]{
                        return set_wallbox_5min_data_point(&utc, &local, uid, flags, power);
                    });
                }
            }
#ifdef DEBUG_LOGGING
            else {
                logger.printfln("collect_data_points: skipping 5min u%u, data too old %u",
                                charger.get("uid")->asUint(), last_update);
            }
#endif
        }

        if (all_data.is_valid && !deadline_elapsed(all_data.last_update + MAX_DATA_AGE)) {
            uint8_t flags = 0; // bit 0 = 1p/3p, bit 1-2 = input, bit 3 = relay, bit 7 = no data (read only)
            int32_t power_grid = INT32_MAX; // W
            int32_t power_general[6] = {INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX}; // W

            flags |= is_3phase         ? 0b0001 : 0;
            flags |= all_data.input[0] ? 0b0010 : 0;
            flags |= all_data.input[1] ? 0b0100 : 0;
            flags |= all_data.relay    ? 0b1000 : 0;

            // FIXME: how to tell if meter data is stale?
            if (history_meter_available) {
                update_history_meter_power(history_meter_power_value);

                if (history_meter_power_duration > 0) {
                    power_grid = clamp<int64_t>(INT32_MIN,
                                                roundf(history_meter_power_sum / history_meter_power_duration),
                                                INT32_MAX - 1); // W

                    history_meter_power_sum = 0;
                    history_meter_power_duration = 0;
                }
            }

            // FIXME: fill power_general

            if (pending_data_points.size() > MAX_PENDING_DATA_POINTS) {
                logger.printfln("energy_manager: Data point queue is full, dropping new data point");
            }
            else {
                pending_data_points.push_back([this, utc, local, flags, power_grid, power_general]{
                    return set_energy_manager_5min_data_point(&utc, &local, flags, power_grid, power_general);
                });
            }
        }

        // daily data
        for (const auto &charger : charge_manager.state.get("chargers")) {
            uint32_t last_update = charger.get("last_update")->asUint();

            if (!deadline_elapsed(last_update + MAX_DATA_AGE)) {
                bool have_data = false;
                uint32_t uid = charger.get("uid")->asUint();
                uint32_t energy = UINT32_MAX;

                if (charger.get("meter_supported")->asBool()) {
                    have_data = true;
                    energy = clamp<uint64_t>(0,
                                             roundf(charger.get("energy_abs")->asFloat() * 100.0),
                                             UINT32_MAX - 1); // kWh -> dWh
                }

                if (have_data) {
                    if (pending_data_points.size() > MAX_PENDING_DATA_POINTS) {
                        logger.printfln("energy_manager: Data point queue is full, dropping new data point");
                    }
                    else {
                        pending_data_points.push_back([this, local, uid, energy]{
                            return set_wallbox_daily_data_point(&local, uid, energy);
                        });
                    }
                }
#ifdef DEBUG_LOGGING
                else {
                    logger.printfln("collect_data_points: skipping daily u%u, no data",
                                    charger.get("uid")->asUint());
                }
#endif
            }
#ifdef DEBUG_LOGGING
            else {
                logger.printfln("collect_data_points: skipping daily u%u, data too old %u",
                                charger.get("uid")->asUint(), last_update);
            }
#endif
        }

        // FIXME: should we even look at all-data validity if we don't use any of it?
        if (all_data.is_valid && !deadline_elapsed(all_data.last_update + MAX_DATA_AGE)) {
            bool have_data = false;
            uint32_t energy_grid_in = UINT32_MAX; // dWh
            uint32_t energy_grid_out = UINT32_MAX; // dWh
            uint32_t energy_general_in[6] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX}; // dWh
            uint32_t energy_general_out[6] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX}; // dWh

            // FIXME: how to tell if meter data is stale?
            if (meter.state.get("state")->asUint() == 2) {
                have_data = true;

                float total_import = meter.all_values.get(METER_ALL_VALUES_TOTAL_IMPORT_KWH)->asFloat(); // kWh
                float total_export = meter.all_values.get(METER_ALL_VALUES_TOTAL_EXPORT_KWH)->asFloat(); // kWh

                if (api.hasFeature("meter_all_values") && !isnan(total_import)) {
                    energy_grid_in = clamp<uint64_t>(0,
                                                     roundf(total_import * 100.0), // kWh -> dWh
                                                     UINT32_MAX - 1);
                }
                else {
                    energy_grid_in = clamp<uint64_t>(0, roundf(history_meter_energy_import), UINT32_MAX - 1);
                }

                if (api.hasFeature("meter_all_values") && !isnan(total_export)) {
                    energy_grid_out = clamp<uint64_t>(0,
                                                      roundf(total_export * 100.0), // kWh -> dWh
                                                      UINT32_MAX - 1);
                }
                else {
                    energy_grid_out = clamp<uint64_t>(0, roundf(history_meter_energy_export), UINT32_MAX - 1);
                }
            }

            // FIXME: fill energy_general_in and energy_general_out

            if (have_data) {
                if (pending_data_points.size() > MAX_PENDING_DATA_POINTS) {
                    logger.printfln("energy_manager: Data point queue is full, dropping new data point");
                }
                else {
                    pending_data_points.push_back([this, local, energy_grid_in, energy_grid_out, energy_general_in, energy_general_out]{
                        return set_energy_manager_daily_data_point(&local, energy_grid_in, energy_grid_out, energy_general_in, energy_general_out);
                    });
                }
            }
        }

        last_history_5min_slot = current_5min_slot;

        save_persistent_data();
    }
}

void EnergyManager::set_pending_data_points()
{
    if (pending_data_points.empty()) {
        return;
    }

    if (!pending_data_points.front()()) {
        return;
    }

    pending_data_points.pop_front();
}

struct PersistentData {
    uint8_t version;
    uint8_t padding0[3];
    uint32_t last_history_5min_slot;
    uint32_t last_history_daily_slot; // unused
    double history_meter_energy_import;
    double history_meter_energy_export;
    uint16_t padding1;
    uint16_t checksum;
};

bool EnergyManager::load_persistent_data()
{
    uint8_t buf[63] = {0};
    if (tf_warp_energy_manager_get_data_storage(&device, 0, buf) != TF_E_OK) {
        return false;
    }

    uint8_t zero[63] = {0};
    if (memcmp(buf, zero, sizeof(buf)) == 0) {
        return true; // all zero, first start
    }

    if (internet_checksum(buf, sizeof(PersistentData)) != 0) {
        logger.printfln("energy_manager: Checksum mismatch while reading persistent data");
        return true;
    }

    PersistentData data;
    memcpy(&data, buf, sizeof(data));

    if (data.version != 1) {
        logger.printfln("energy_manager: Unexpected version %u while reading persistent data", data.version);
        return true;
    }

    last_history_5min_slot = data.last_history_5min_slot;
    history_meter_energy_import = data.history_meter_energy_import;
    history_meter_energy_export = data.history_meter_energy_export;

#ifdef DEBUG_LOGGING
    logger.printfln("load_persistent_data: slot %u, energy %f %f",
                    last_history_5min_slot,
                    history_meter_energy_import,
                    history_meter_energy_export);
#endif

    return true;
}

void EnergyManager::save_persistent_data()
{
    PersistentData data;
    memset(&data, 0, sizeof(data));

#ifdef DEBUG_LOGGING
    logger.printfln("save_persistent_data: slot %u, energy %f %f",
                    last_history_5min_slot,
                    history_meter_energy_import,
                    history_meter_energy_export);
#endif

    data.version = 1;
    data.last_history_5min_slot = last_history_5min_slot;
    data.last_history_daily_slot = 0; // unused
    data.history_meter_energy_import = history_meter_energy_import;
    data.history_meter_energy_export = history_meter_energy_export;
    data.checksum = internet_checksum((uint8_t *)&data, sizeof(data));

    uint8_t buf[63] = {0};
    memcpy(buf, &data, sizeof(data));
    tf_warp_energy_manager_set_data_storage(&device, 0, buf);
}

bool EnergyManager::set_wallbox_5min_data_point(const struct tm *utc, const struct tm *local, uint32_t uid, uint8_t flags, uint16_t power /* W */)
{
    uint8_t status;
    uint8_t utc_year = utc->tm_year - 100;
    uint8_t utc_month = utc->tm_mon + 1;
    uint8_t utc_day = utc->tm_mday;
    uint8_t utc_hour = utc->tm_hour;
    uint8_t utc_minute = (utc->tm_min / 5) * 5;
    int rc = tf_warp_energy_manager_set_sd_wallbox_data_point(&device,
                                                              uid,
                                                              utc_year,
                                                              utc_month,
                                                              utc_day,
                                                              utc_hour,
                                                              utc_minute,
                                                              flags,
                                                              power,
                                                              &status);

    check_bricklet_reachable(rc, "set_wallbox_5min_data_point");

#ifdef DEBUG_LOGGING
    logger.printfln("set_wallbox_5min_data_point: u%u %d-%02d-%02d %02d:%02d f%u p%u",
                    uid, 2000 + utc_year, utc_month, utc_day, utc_hour, utc_minute, flags, power);
#endif

    if (rc != TF_E_OK) {
        if (rc == TF_E_TIMEOUT || is_in_bootloader(rc)) {
            return false; // retry
        }
        else {
            logger.printfln("energy_manager: Failed to set wallbox 5min data point: error %d", rc);
            return true;
        }
    }
    else if (status != 0) {
        if (status == TF_WARP_ENERGY_MANAGER_DATA_STATUS_QUEUE_FULL) {
            return false; // retry
        }
        else {
            logger.printfln("energy_manager: Failed to set wallbox 5min data point: status %u", status);
            return true;
        }
    }
    else {
        char power_str[6] = "null";

        if (power != UINT16_MAX) {
            snprintf(power_str, sizeof(power_str), "%u", power);
        }

        uint8_t local_year = local->tm_year - 100;
        uint8_t local_month = local->tm_mon + 1;
        uint8_t local_day = local->tm_mday;
        uint8_t local_hour = local->tm_hour;
        uint8_t local_minute = (local->tm_min / 5) * 5;
        char *buf;
        int buf_written = asprintf(&buf,
                                   "{\"topic\":\"energy_manager/history_wallbox_5min_changed\","
                                    "\"payload\":{"
                                    "\"uid\":%u,"
                                    "\"year\":%u,"
                                    "\"month\":%u,"
                                    "\"day\":%u,"
                                    "\"hour\":%u,"
                                    "\"minute\":%u,"
                                    "\"flags\":%u,"
                                    "\"power\":%s}}\n",
                                   uid,
                                   2000U + local_year,
                                   local_month,
                                   local_day,
                                   local_hour,
                                   local_minute,
                                   flags,
                                   power_str);

        if (buf_written > 0) {
            ws.web_sockets.sendToAllOwned(buf, buf_written);
        }

        return true;
    }
}

bool EnergyManager::set_wallbox_daily_data_point(const struct tm *local, uint32_t uid, uint32_t energy /* dWh */)
{
    uint8_t status;
    uint8_t year = local->tm_year - 100;
    uint8_t month = local->tm_mon + 1;
    uint8_t day = local->tm_mday;
    int rc = tf_warp_energy_manager_set_sd_wallbox_daily_data_point(&device, uid, year, month, day, energy, &status);

    check_bricklet_reachable(rc, "set_wallbox_daily_data_point");

#ifdef DEBUG_LOGGING
    logger.printfln("set_wallbox_daily_data_point: u%u %d-%02d-%02d e%u",
                    uid, 2000 + year, month, day, energy);
#endif

    if (rc != TF_E_OK) {
        if (rc == TF_E_TIMEOUT || is_in_bootloader(rc)) {
            return false; // retry
        }
        else {
            logger.printfln("energy_manager: Failed to set wallbox daily data point: error %d", rc);
            return true;
        }
    }
    else if (status != 0) {
        if (status == TF_WARP_ENERGY_MANAGER_DATA_STATUS_QUEUE_FULL) {
            return false; // retry
        }
        else {
            logger.printfln("energy_manager: Failed to set wallbox daily data point: status %u", status);
            return true;
        }
    }
    else {
        char energy_str[12] = "null";

        if (energy != UINT32_MAX) {
            snprintf(energy_str, sizeof(energy_str), "%.2f", (double)energy / 100.0); // dWh -> kWh
        }

        char *buf;
        int buf_written = asprintf(&buf,
                                   "{\"topic\":\"energy_manager/history_wallbox_daily_changed\","
                                    "\"payload\":{"
                                    "\"uid\":%u,"
                                    "\"year\":%u,"
                                    "\"month\":%u,"
                                    "\"day\":%u,"
                                    "\"energy\":%s}}\n",
                                   uid,
                                   2000U + year,
                                   month,
                                   day,
                                   energy_str);

        if (buf_written > 0) {
            ws.web_sockets.sendToAllOwned(buf, buf_written);
        }

        return true;
    }
}

bool EnergyManager::set_energy_manager_5min_data_point(const struct tm *utc,
                                                       const struct tm *local,
                                                       uint8_t flags,
                                                       int32_t power_grid /* W */,
                                                       const int32_t power_general[6] /* W */)
{
    uint8_t status;
    uint8_t utc_year = utc->tm_year - 100;
    uint8_t utc_month = utc->tm_mon + 1;
    uint8_t utc_day = utc->tm_mday;
    uint8_t utc_hour = utc->tm_hour;
    uint8_t utc_minute = (utc->tm_min / 5) * 5;
    int rc = tf_warp_energy_manager_set_sd_energy_manager_data_point(&device,
                                                                     utc_year,
                                                                     utc_month,
                                                                     utc_day,
                                                                     utc_hour,
                                                                     utc_minute,
                                                                     flags,
                                                                     power_grid,
                                                                     power_general,
                                                                     &status);

    check_bricklet_reachable(rc, "set_energy_manager_5min_data_point");

#ifdef DEBUG_LOGGING
    logger.printfln("set_energy_manager_5min_data_point: %d-%02d-%02d %02d:%02d f%u gr%d ge%d,%d,%d,%d,%d,%d",
                    2000 + utc_year, utc_month, utc_day, utc_hour, utc_minute, flags, power_grid, power_general[0], power_general[1], power_general[2], power_general[3], power_general[4], power_general[5]);
#endif

    if (rc != TF_E_OK) {
        if (rc == TF_E_TIMEOUT || is_in_bootloader(rc)) {
            return false; // retry
        }
        else {
            logger.printfln("energy_manager: Failed to set energy manager 5min data point: error %d", rc);
            return true;
        }
    }
    else if (status != 0) {
        if (status == TF_WARP_ENERGY_MANAGER_DATA_STATUS_QUEUE_FULL) {
            return false; // retry
        }
        else {
            logger.printfln("energy_manager: Failed to set energy manager 5min data point: status %u", status);
            return true;
        }
    }
    else {
        char power_grid_str[12] = "null";
        char power_general_str[6][12] = {"null", "null", "null", "null", "null", "null"};

        if (power_grid != INT32_MAX) {
            snprintf(power_grid_str, sizeof(power_grid_str), "%d", power_grid);
        }

        for (int i = 0; i < 6; ++i) {
            if (power_general[i] != INT32_MAX) {
                snprintf(power_general_str[i], sizeof(power_general_str[i]), "%d", power_general[i]);
            }
        }

        uint8_t local_year = local->tm_year - 100;
        uint8_t local_month = local->tm_mon + 1;
        uint8_t local_day = local->tm_mday;
        uint8_t local_hour = local->tm_hour;
        uint8_t local_minute = (local->tm_min / 5) * 5;
        char *buf;
        int buf_written = asprintf(&buf,
                                   "{\"topic\":\"energy_manager/history_energy_manager_5min_changed\","
                                    "\"payload\":{"
                                    "\"year\":%u,"
                                    "\"month\":%u,"
                                    "\"day\":%u,"
                                    "\"hour\":%u,"
                                    "\"minute\":%u,"
                                    "\"flags\":%u,"
                                    "\"power_grid\":%s,"
                                    "\"power_general\":[%s,%s,%s,%s,%s,%s]}}\n",
                                   2000U + local_year,
                                   local_month,
                                   local_day,
                                   local_hour,
                                   local_minute,
                                   flags,
                                   power_grid_str,
                                   power_general_str[0],
                                   power_general_str[1],
                                   power_general_str[2],
                                   power_general_str[3],
                                   power_general_str[4],
                                   power_general_str[5]);

        if (buf_written > 0) {
            ws.web_sockets.sendToAllOwned(buf, buf_written);
        }

        return true;
    }
}

bool EnergyManager::set_energy_manager_daily_data_point(const struct tm *local,
                                                        uint32_t energy_grid_in /* dWh */,
                                                        uint32_t energy_grid_out /* dWh */,
                                                        const uint32_t energy_general_in[6] /* dWh */,
                                                        const uint32_t energy_general_out[6] /* dWh */)
{
    uint8_t status;
    uint8_t year = local->tm_year - 100;
    uint8_t month = local->tm_mon + 1;
    uint8_t day = local->tm_mday;
    int rc = tf_warp_energy_manager_set_sd_energy_manager_daily_data_point(&device,
                                                                           year,
                                                                           month,
                                                                           day,
                                                                           energy_grid_in,
                                                                           energy_grid_out,
                                                                           energy_general_in,
                                                                           energy_general_out,
                                                                           &status);

    check_bricklet_reachable(rc, "set_energy_manager_daily_data_point");

#ifdef DEBUG_LOGGING
    logger.printfln("set_energy_manager_daily_data_point: %d-%02d-%02d gri%u gro%u gei%u,%u,%u,%u,%u,%u geo%u,%u,%u,%u,%u,%u",
                    2000 + year, month, day,
                    energy_grid_in, energy_grid_out,
                    energy_general_in[0], energy_general_in[1], energy_general_in[2], energy_general_in[3], energy_general_in[4], energy_general_in[5],
                    energy_general_out[0], energy_general_out[1], energy_general_out[2], energy_general_out[3], energy_general_out[4], energy_general_out[5]);
#endif

    if (rc != TF_E_OK) {
        if (rc == TF_E_TIMEOUT || is_in_bootloader(rc)) {
            return false; // retry
        }
        else {
            logger.printfln("energy_manager: Failed to set energy manager daily data point: error %i", rc);
            return true;
        }
    }
    else if (status != 0) {
        if (status == TF_WARP_ENERGY_MANAGER_DATA_STATUS_QUEUE_FULL) {
            return false; // retry
        }
        else {
            logger.printfln("energy_manager: Failed to set energy manager daily data point: status %u", status);
            return true;
        }
    }
    else {
        char energy_grid_in_str[13] = "null";
        char energy_grid_out_str[13] = "null";
        char energy_general_in_str[6][13] = {"null", "null", "null", "null", "null", "null"};
        char energy_general_out_str[6][13] = {"null", "null", "null", "null", "null", "null"};

        if (energy_grid_in != INT32_MAX) {
            snprintf(energy_grid_in_str, sizeof(energy_grid_in_str), "%.2f", (double)energy_grid_in / 100.0); // dWh -> kWh
        }

        if (energy_grid_out != INT32_MAX) {
            snprintf(energy_grid_out_str, sizeof(energy_grid_out_str), "%.2f", (double)energy_grid_out / 100.0); // dWh -> kWh
        }

        for (int i = 0; i < 6; ++i) {
            if (energy_general_in[i] != INT32_MAX) {
                snprintf(energy_general_in_str[i], sizeof(energy_general_in_str[i]), "%.2f", (double)energy_general_in[i] / 100.0); // dWh -> kWh
            }

            if (energy_general_out[i] != INT32_MAX) {
                snprintf(energy_general_out_str[i], sizeof(energy_general_out_str[i]), "%.2f", (double)energy_general_out[i] / 100.0); // dWh -> kWh
            }
        }

        char *buf;
        int buf_written = asprintf(&buf,
                                   "{\"topic\":\"energy_manager/history_energy_manager_daily_changed\","
                                    "\"payload\":{"
                                    "\"year\":%u,"
                                    "\"month\":%u,"
                                    "\"day\":%u,"
                                    "\"energy_grid_in\":%s,"
                                    "\"energy_grid_out\":%s,"
                                    "\"energy_general_in\":[%s,%s,%s,%s,%s,%s],"
                                    "\"energy_general_out\":[%s,%s,%s,%s,%s,%s]}}\n",
                                   2000U + year,
                                   month,
                                   day,
                                   energy_grid_in_str,
                                   energy_grid_out_str,
                                   energy_general_in_str[0],
                                   energy_general_in_str[1],
                                   energy_general_in_str[2],
                                   energy_general_in_str[3],
                                   energy_general_in_str[4],
                                   energy_general_in_str[5],
                                   energy_general_out_str[0],
                                   energy_general_out_str[1],
                                   energy_general_out_str[2],
                                   energy_general_out_str[3],
                                   energy_general_out_str[4],
                                   energy_general_out_str[5]);

        if (buf_written > 0) {
            ws.web_sockets.sendToAllOwned(buf, buf_written);
        }

        return true;
    }
}

typedef struct {
    IChunkedResponse *response;
    Ownership *response_ownership;
    uint32_t response_owner_id;
    bool call_begin;
    bool write_comma;
    uint16_t next_offset;
    uint32_t uid;
    uint8_t utc_end_year;
    uint8_t utc_end_month;
    uint8_t utc_end_day;
    uint16_t utc_end_slots;
} StreamMetadata;

static StreamMetadata metadata_array[4];

typedef struct {
    uint8_t flags; // bit 0-2 = charger state, bit 7 = no data (read only)
    uint16_t power; // W
} __attribute__((__packed__)) Wallbox5minData;

static void wallbox_5min_data_points_handler(TF_WARPEnergyManager *device, uint16_t data_length, uint16_t data_chunk_offset, uint8_t data_chunk_data[60], void *user_data)
{
    StreamMetadata *metadata = (StreamMetadata *)user_data;
    IChunkedResponse *response = metadata->response;
    bool write_success = true;
    OwnershipGuard ownership_guard(metadata->response_ownership, metadata->response_owner_id);

    if (!ownership_guard.have_ownership()) {
        tf_warp_energy_manager_register_sd_wallbox_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }

    if (metadata->call_begin) {
        metadata->call_begin = false;

        response->begin(true);

        if (write_success) {
            write_success = response->write("[");
        }
    }

    if (metadata->next_offset != data_chunk_offset) {
        logger.printfln("energy_manager: Failed to get wallbox 5min data point: stream out of sync (%u != %u)", metadata->next_offset, data_chunk_offset);

        if (write_success) {
            write_success = response->write("]");
        }

        write_success &= response->flush();
        response->end(write_success ? "" : "write error");

        tf_warp_energy_manager_register_sd_wallbox_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }

    uint16_t actual_length = data_length - data_chunk_offset;
    uint16_t i;
    Wallbox5minData *p;

    if (actual_length > 60) {
        actual_length = 60;
    }

    if (metadata->write_comma && write_success) {
        write_success = response->write(",");
    }

    for (i = 0; i < actual_length && write_success; i += sizeof(Wallbox5minData)) {
        p = (Wallbox5minData *)&data_chunk_data[i];

        if ((p->flags & 0x80 /* no data */) == 0) {
            write_success = response->writef("%u", p->flags);
        } else {
            p->power = UINT16_MAX;
            write_success = response->writef("null");
        }

        if (write_success) {
            if (p->power != UINT16_MAX) {
                write_success = response->writef(",%u", p->power);
            } else {
                write_success = response->writef(",null");
            }

            if (write_success && i < actual_length - sizeof(Wallbox5minData)) {
                write_success = response->write(",");
            }
        }
    }

    metadata->write_comma = true;
    metadata->next_offset += 60;

    if (metadata->next_offset >= data_length) {
        if (metadata->utc_end_slots > 0) {
            if (!write_success) {
                response->flush();
                response->end("write error");

                tf_warp_energy_manager_register_sd_wallbox_data_points_low_level_callback(device, nullptr, nullptr);
            } else {
                task_scheduler.scheduleOnce([device, metadata, response]{
                    uint8_t status;
                    int rc = tf_warp_energy_manager_get_sd_wallbox_data_points(device,
                                                                               metadata->uid,
                                                                               metadata->utc_end_year,
                                                                               metadata->utc_end_month,
                                                                               metadata->utc_end_day,
                                                                               0,
                                                                               0,
                                                                               metadata->utc_end_slots,
                                                                               &status);

                    metadata->next_offset = 0;
                    metadata->utc_end_slots = 0;

                    if (rc != TF_E_OK || status != 0) {
                        if (rc != TF_E_OK) {
                            logger.printfln("energy_manager: Failed to continue getting wallbox 5min data point: error %d", rc);
                        }
                        else if (status != 0) {
                            logger.printfln("energy_manager: Failed to continue getting wallbox 5min data point: status %u", status);
                        }

                        OwnershipGuard ownership_guard2(metadata->response_ownership, metadata->response_owner_id);

                        if (ownership_guard2.have_ownership()) {
                            response->flush();
                            response->end("continuation error");
                        }

                        tf_warp_energy_manager_register_sd_wallbox_data_points_low_level_callback(device, nullptr, nullptr);
                    }
                }, 0);
            }
        }
        else {
            if (write_success) {
                write_success = response->write("]");
            }

            write_success &= response->flush();
            response->end(write_success ? "" : "write error");

            tf_warp_energy_manager_register_sd_wallbox_data_points_low_level_callback(device, nullptr, nullptr);
        }

        return;
    }

    if (!write_success) {
        response->end("write error");

        tf_warp_energy_manager_register_sd_wallbox_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }
}

void EnergyManager::history_wallbox_5min_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id)
{
    uint32_t uid = history_wallbox_5min.get("uid")->asUint();

    // history is stored with date in UTC to avoid DST overlap problems.
    // API accepts date in localtime, convert from localtime to UTC
    uint8_t local_year = history_wallbox_5min.get("year")->asUint() - 2000;
    uint8_t local_month = history_wallbox_5min.get("month")->asUint();
    uint8_t local_day = history_wallbox_5min.get("day")->asUint();

    struct tm local_start;
    struct tm local_end;

    memset(&local_start, 0, sizeof(local_start));
    memset(&local_end, 0, sizeof(local_end));

    local_start.tm_year = local_year + 100;
    local_start.tm_mon = local_month - 1;
    local_start.tm_mday = local_day;
    local_start.tm_isdst = -1;

    local_end.tm_year = local_year + 100;
    local_end.tm_mon = local_month - 1;
    local_end.tm_mday = local_day + 1;
    local_end.tm_isdst = -1;

    time_t time_start = mktime(&local_start);
    time_t time_end = mktime(&local_end);
    struct tm utc_start;
    struct tm utc_end;

    gmtime_r(&time_start, &utc_start);
    gmtime_r(&time_end, &utc_end);

    uint8_t utc_start_year = utc_start.tm_year - 100;
    uint8_t utc_start_month = utc_start.tm_mon + 1;
    uint8_t utc_start_day = utc_start.tm_mday;
    uint8_t utc_start_hour = utc_start.tm_hour;
    uint8_t utc_start_minute = utc_start.tm_min;
    uint16_t utc_start_slots = ((23 - utc_start_hour) * 60 + (60 - utc_start_minute)) / 5; // till midnight

    uint8_t utc_end_year = utc_end.tm_year - 100;
    uint8_t utc_end_month = utc_end.tm_mon + 1;
    uint8_t utc_end_day = utc_end.tm_mday;
    uint8_t utc_end_hour = utc_end.tm_hour;
    uint8_t utc_end_minute = utc_end.tm_min;
    uint16_t utc_end_slots = (utc_end_hour * 60 + utc_end_minute) / 5; // since midnight

    uint8_t status;
    int rc;

    if (utc_start_slots > 0) {
        rc = tf_warp_energy_manager_get_sd_wallbox_data_points(&device,
                                                               uid,
                                                               utc_start_year,
                                                               utc_start_month,
                                                               utc_start_day,
                                                               utc_start_hour,
                                                               utc_start_minute,
                                                               utc_start_slots,
                                                               &status);
    } else {
        rc = tf_warp_energy_manager_get_sd_wallbox_data_points(&device,
                                                               uid,
                                                               utc_end_year,
                                                               utc_end_month,
                                                               utc_end_day,
                                                               utc_end_hour,
                                                               utc_end_minute,
                                                               utc_end_slots,
                                                               &status);
        utc_end_slots = 0;
    }

    //logger.printfln("history_wallbox_5min_response: u%u %d-%02d-%02d",
    //                uid, 2000 + year, month, day);

    if (rc != TF_E_OK || status != 0) {
        OwnershipGuard ownership_guard(response_ownership, response_owner_id);

        if (ownership_guard.have_ownership()) {
            response->begin(false);

            if (rc != TF_E_OK) {
                response->writef("Failed to get wallbox 5min data point: error %d", rc);
                logger.printfln("energy_manager: Failed to get wallbox 5min data point: error %d", rc);
            }
            else if (status != 0) {
                response->writef("Failed to get wallbox 5min data point: status %u", status);
                logger.printfln("energy_manager: Failed to get wallbox 5min data point: status %u", status);
            }

            response->flush();
            response->end("");
        }
    }
    else {
        StreamMetadata *metadata = &metadata_array[0];

        metadata->response = response;
        metadata->response_ownership = response_ownership;
        metadata->response_owner_id = response_owner_id;
        metadata->call_begin = true;
        metadata->write_comma = false;
        metadata->next_offset = 0;
        metadata->uid = uid;
        metadata->utc_end_year = utc_end_year;
        metadata->utc_end_month = utc_end_month;
        metadata->utc_end_day = utc_end_day;
        metadata->utc_end_slots = utc_end_slots;

        tf_warp_energy_manager_register_sd_wallbox_data_points_low_level_callback(&device, wallbox_5min_data_points_handler, metadata);
    }

    check_bricklet_reachable(rc, "history_wallbox_5min_response");
}

static void wallbox_daily_data_points_handler(TF_WARPEnergyManager *device,
                                              uint16_t data_length,
                                              uint16_t data_chunk_offset,
                                              uint32_t data_chunk_data[15],
                                              void *user_data)
{
    StreamMetadata *metadata = (StreamMetadata *)user_data;
    IChunkedResponse *response = metadata->response;
    bool write_success = true;
    OwnershipGuard ownership_guard(metadata->response_ownership, metadata->response_owner_id);

    if (!ownership_guard.have_ownership()) {
        tf_warp_energy_manager_register_sd_wallbox_daily_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }

    if (metadata->call_begin) {
        metadata->call_begin = false;

        response->begin(true);

        if (write_success) {
            write_success = response->write("[");
        }
    }

    if (metadata->next_offset != data_chunk_offset) {
        logger.printfln("energy_manager: Failed to get wallbox daily data point: stream out of sync (%u != %u)",
                        metadata->next_offset, data_chunk_offset);

        if (write_success) {
            write_success = response->write("]");
        }

        write_success &= response->flush();
        response->end(write_success ? "" : "write error");

        tf_warp_energy_manager_register_sd_wallbox_daily_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }

    uint16_t actual_length = data_length - data_chunk_offset;
    uint16_t i;

    if (actual_length > 15) {
        actual_length = 15;
    }

    if (metadata->write_comma && write_success) {
        write_success = response->write(",");
    }

    for (i = 0; i < actual_length && write_success; ++i) {
        if (data_chunk_data[i] != UINT32_MAX) {
            write_success = response->writef("%.2f", (double)data_chunk_data[i] / 100.0); // dWh -> kWh
        } else {
            write_success = response->write("null");
        }

        if (write_success && i < actual_length - 1) {
            write_success = response->write(",");
        }
    }

    metadata->write_comma = true;
    metadata->next_offset += 15;

    if (metadata->next_offset >= data_length) {
        if (write_success) {
            write_success = response->write("]");
        }

        write_success &= response->flush();
        response->end(write_success ? "" : "write error");

        tf_warp_energy_manager_register_sd_wallbox_daily_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }

    if (!write_success) {
        response->end("write error");

        tf_warp_energy_manager_register_sd_wallbox_daily_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }
}

static int days_per_month(int year, int month)
{
    if (month == 2) {
        if ((year % 400) == 0 || ((year % 100) != 0 && (year % 4) == 0)) {
            return 29;
        }

        return 28;
    }

    if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }

    return 31;
}

void EnergyManager::history_wallbox_daily_response(IChunkedResponse *response,
                                                   Ownership *response_ownership,
                                                   uint32_t response_owner_id)
{
    uint32_t uid = history_wallbox_daily.get("uid")->asUint();

    // date in local time to have the days properly aligned
    uint8_t year = history_wallbox_daily.get("year")->asUint() - 2000;
    uint8_t month = history_wallbox_daily.get("month")->asUint();

    uint8_t status;
    int rc = tf_warp_energy_manager_get_sd_wallbox_daily_data_points(&device, uid, year, month, 1, days_per_month(2000 + year, month), &status);

    //logger.printfln("history_wallbox_daily_response: u%u %d-%02d",
    //                uid, 2000 + year, month);

    if (rc != TF_E_OK || status != 0) {
        OwnershipGuard ownership_guard(response_ownership, response_owner_id);

        if (ownership_guard.have_ownership()) {
            response->begin(false);

            if (rc != TF_E_OK) {
                response->writef("Failed to get wallbox daily data point: error %d", rc);
                logger.printfln("energy_manager: Failed to get wallbox daily data point: error %d", rc);
            }
            else if (status != 0) {
                response->writef("Failed to get wallbox daily data point: status %u", status);
                logger.printfln("energy_manager: Failed to get wallbox daily data point: status %u", status);
            }

            response->flush();
            response->end("");
        }
    }
    else {
        StreamMetadata *metadata = &metadata_array[1];

        metadata->response = response;
        metadata->response_ownership = response_ownership;
        metadata->response_owner_id = response_owner_id;
        metadata->call_begin = true;
        metadata->write_comma = false;
        metadata->next_offset = 0;

        tf_warp_energy_manager_register_sd_wallbox_daily_data_points_low_level_callback(&device, wallbox_daily_data_points_handler, metadata);
    }

    check_bricklet_reachable(rc, "history_wallbox_daily_response");
}

typedef struct {
    uint8_t flags; // bit 0 = 1p/3p, bit 1-2 = input, bit 3 = relay, bit 7 = no data
    int32_t power_grid; // W
    int32_t power_general[6]; // W
} __attribute__((__packed__)) EnergyManager5MinData;

static void energy_manager_5min_data_points_handler(TF_WARPEnergyManager *device,
                                                    uint16_t data_length,
                                                    uint16_t data_chunk_offset,
                                                    uint8_t data_chunk_data[58],
                                                    void *user_data)
{
    StreamMetadata *metadata = (StreamMetadata *)user_data;
    IChunkedResponse *response = metadata->response;
    bool write_success = true;
    OwnershipGuard ownership_guard(metadata->response_ownership, metadata->response_owner_id);

    if (!ownership_guard.have_ownership()) {
        tf_warp_energy_manager_register_sd_energy_manager_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }

    if (metadata->call_begin) {
        metadata->call_begin = false;

        response->begin(true);

        if (write_success) {
            write_success = response->write("[");
        }
    }

    if (metadata->next_offset != data_chunk_offset) {
        logger.printfln("energy_manager: Failed to get energy manager 5min data point: stream out of sync (%u != %u)",
                        metadata->next_offset, data_chunk_offset);

        if (write_success) {
            write_success = response->write("]");
        }

        write_success &= response->flush();
        response->end(write_success ? "" : "write error");

        tf_warp_energy_manager_register_sd_energy_manager_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }

    uint16_t actual_length = data_length - data_chunk_offset;
    uint16_t i;
    EnergyManager5MinData *p;

    if (actual_length > 58) {
        actual_length = 58;
    }

    if (metadata->write_comma && write_success) {
        write_success = response->write(",", 1);
    }

    for (i = 0; i < actual_length && write_success; i += sizeof(EnergyManager5MinData)) {
        p = (EnergyManager5MinData *)&data_chunk_data[i];

        if ((p->flags & 0x80 /* no data */) == 0) {
            write_success = response->writef("%u", p->flags);
        } else {
            write_success = response->writef("null");
            p->power_grid = INT32_MAX;

            for (int k = 0; k < 6; ++k) {
                p->power_general[k] = INT32_MAX;
            }
        }

        if (write_success) {
            if (p->power_grid != INT32_MAX) {
                write_success = response->writef(",%d", p->power_grid);
            } else {
                write_success = response->writef(",null");
            }

            for (int k = 0; k < 6 && write_success; ++k) {
                if (p->power_general[k] != INT32_MAX) {
                    write_success = response->writef(",%d", p->power_general[k]);
                } else {
                    write_success = response->writef(",null");
                }
            }

            if (write_success && i < actual_length - sizeof(EnergyManager5MinData)) {
                write_success = response->write(",");
            }
        }
    }

    metadata->write_comma = true;
    metadata->next_offset += 58;

    if (metadata->next_offset >= data_length) {
        if (metadata->utc_end_slots > 0) {
            if (!write_success) {
                response->flush();
                response->end("write error");

                tf_warp_energy_manager_register_sd_energy_manager_data_points_low_level_callback(device, nullptr, nullptr);
            } else {
                task_scheduler.scheduleOnce([device, metadata, response]{
                    uint8_t status;
                    int rc = tf_warp_energy_manager_get_sd_energy_manager_data_points(device,
                                                                                      metadata->utc_end_year,
                                                                                      metadata->utc_end_month,
                                                                                      metadata->utc_end_day,
                                                                                      0,
                                                                                      0,
                                                                                      metadata->utc_end_slots,
                                                                                      &status);

                    metadata->next_offset = 0;
                    metadata->utc_end_slots = 0;

                    if (rc != TF_E_OK || status != 0) {
                        if (rc != TF_E_OK) {
                            logger.printfln("energy_manager: Failed to continue getting energy manager 5min data point: error %d", rc);
                        }
                        else if (status != 0) {
                            logger.printfln("energy_manager: Failed to continue getting energy manager 5min data point: status %u", status);
                        }

                        OwnershipGuard ownership_guard2(metadata->response_ownership, metadata->response_owner_id);

                        if (ownership_guard2.have_ownership()) {
                            response->flush();
                            response->end("continuation error");
                        }

                        tf_warp_energy_manager_register_sd_energy_manager_data_points_low_level_callback(device, nullptr, nullptr);
                    }
                }, 0);
            }
        }
        else {
            if (write_success) {
                write_success = response->write("]");
            }

            write_success &= response->flush();
            response->end(write_success ? "" : "write error");

            tf_warp_energy_manager_register_sd_energy_manager_data_points_low_level_callback(device, nullptr, nullptr);
        }

        return;
    }

    if (!write_success) {
        response->end("write error");

        tf_warp_energy_manager_register_sd_energy_manager_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }
}

void EnergyManager::history_energy_manager_5min_response(IChunkedResponse *response,
                                                         Ownership *response_ownership,
                                                         uint32_t response_owner_id)
{
    // history is stored with date in UTC to avoid DST overlap problems.
    // API accepts date in localtime, convert from localtime to UTC
    uint8_t local_year = history_energy_manager_5min.get("year")->asUint() - 2000;
    uint8_t local_month = history_energy_manager_5min.get("month")->asUint();
    uint8_t local_day = history_energy_manager_5min.get("day")->asUint();

    struct tm local_start;
    struct tm local_end;

    memset(&local_start, 0, sizeof(local_start));
    memset(&local_end, 0, sizeof(local_end));

    local_start.tm_year = local_year + 100;
    local_start.tm_mon = local_month - 1;
    local_start.tm_mday = local_day;
    local_start.tm_isdst = -1;

    local_end.tm_year = local_year + 100;
    local_end.tm_mon = local_month - 1;
    local_end.tm_mday = local_day + 1;
    local_end.tm_isdst = -1;

    time_t time_start = mktime(&local_start);
    time_t time_end = mktime(&local_end);
    struct tm utc_start;
    struct tm utc_end;

    gmtime_r(&time_start, &utc_start);
    gmtime_r(&time_end, &utc_end);

    uint8_t utc_start_year = utc_start.tm_year - 100;
    uint8_t utc_start_month = utc_start.tm_mon + 1;
    uint8_t utc_start_day = utc_start.tm_mday;
    uint8_t utc_start_hour = utc_start.tm_hour;
    uint8_t utc_start_minute = utc_start.tm_min;
    uint16_t utc_start_slots = ((23 - utc_start_hour) * 60 + (60 - utc_start_minute)) / 5; // till midnight

    uint8_t utc_end_year = utc_end.tm_year - 100;
    uint8_t utc_end_month = utc_end.tm_mon + 1;
    uint8_t utc_end_day = utc_end.tm_mday;
    uint8_t utc_end_hour = utc_end.tm_hour;
    uint8_t utc_end_minute = utc_end.tm_min;
    uint16_t utc_end_slots = (utc_end_hour * 60 + utc_end_minute) / 5; // since midnight

    uint8_t status;
    int rc;

    if (utc_start_slots > 0) {
        rc = tf_warp_energy_manager_get_sd_energy_manager_data_points(&device,
                                                                      utc_start_year,
                                                                      utc_start_month,
                                                                      utc_start_day,
                                                                      utc_start_hour,
                                                                      utc_start_minute,
                                                                      utc_start_slots,
                                                                      &status);
    }
    else {
        rc = tf_warp_energy_manager_get_sd_energy_manager_data_points(&device,
                                                                      utc_end_year,
                                                                      utc_end_month,
                                                                      utc_end_day,
                                                                      utc_start_hour,
                                                                      utc_start_minute,
                                                                      utc_start_slots,
                                                                      &status);
    }

    //logger.printfln("history_energy_manager_5min_response: %d-%02d-%02d",
    //                2000 + year, month, day);

    if (rc != TF_E_OK || status != 0) {
        OwnershipGuard ownership_guard(response_ownership, response_owner_id);

        if (ownership_guard.have_ownership()) {
            response->begin(false);

            if (rc != TF_E_OK) {
                response->writef("Failed to get energy manager 5min data point: error %d", rc);
                logger.printfln("energy_manager: Failed to get energy manager 5min data point: error %d", rc);
            }
            else if (status != 0) {
                response->writef("Failed to get energy manager 5min data point: status %u", status);
                logger.printfln("energy_manager: Failed to get energy manager 5min data point: status %u", status);
            }

            response->flush();
            response->end("");
        }
    }
    else {
        StreamMetadata *metadata = &metadata_array[2];

        metadata->response = response;
        metadata->response_ownership = response_ownership;
        metadata->response_owner_id = response_owner_id;
        metadata->call_begin = true;
        metadata->write_comma = false;
        metadata->next_offset = 0;
        metadata->utc_end_year = utc_end_year;
        metadata->utc_end_month = utc_end_month;
        metadata->utc_end_day = utc_end_day;
        metadata->utc_end_slots = utc_end_slots;

        tf_warp_energy_manager_register_sd_energy_manager_data_points_low_level_callback(&device, energy_manager_5min_data_points_handler, metadata);
    }

    check_bricklet_reachable(rc, "history_energy_manager_5min_response");
}

static void energy_manager_daily_data_points_handler(TF_WARPEnergyManager *device,
                                                     uint16_t data_length,
                                                     uint16_t data_chunk_offset,
                                                     uint32_t data_chunk_data[14],
                                                     void *user_data)
{
    StreamMetadata *metadata = (StreamMetadata *)user_data;
    IChunkedResponse *response = metadata->response;
    bool write_success = true;
    OwnershipGuard ownership_guard(metadata->response_ownership, metadata->response_owner_id);

    if (!ownership_guard.have_ownership()) {
        tf_warp_energy_manager_register_sd_energy_manager_daily_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }

    if (metadata->call_begin) {
        metadata->call_begin = false;

        response->begin(true);

        if (write_success) {
            write_success = response->write("[");
        }
    }

    if (metadata->next_offset != data_chunk_offset) {
        logger.printfln("energy_manager: Failed to get energy manager daily data point: stream out of sync (%u != %u)",
                        metadata->next_offset, data_chunk_offset);

        if (write_success) {
            write_success = response->write("]");
        }

        write_success &= response->flush();
        response->end(write_success ? "" : "write error");

        tf_warp_energy_manager_register_sd_energy_manager_daily_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }

    uint16_t actual_length = data_length - data_chunk_offset;
    uint16_t i;

    if (actual_length > 14) {
        actual_length = 14;
    }

    if (metadata->write_comma && write_success) {
        write_success = response->write(",");
    }

    for (i = 0; i < actual_length && write_success; ++i) {
        if (data_chunk_data[i] != UINT32_MAX) {
            write_success = response->writef("%.2f", (double)data_chunk_data[i] / 100.0); // dWh -> kWh
        } else {
            write_success = response->write("null");
        }

        if (write_success && i < actual_length - 1) {
            write_success = response->write(",");
        }
    }

    metadata->write_comma = true;
    metadata->next_offset += 14;

    if (metadata->next_offset >= data_length) {
        if (write_success) {
            write_success = response->write("]");
        }

        write_success &= response->flush();
        response->end(write_success ? "" : "write error");

        tf_warp_energy_manager_register_sd_energy_manager_daily_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }

    if (!write_success) {
        response->end("write error");

        tf_warp_energy_manager_register_sd_energy_manager_daily_data_points_low_level_callback(device, nullptr, nullptr);
        return;
    }
}

void EnergyManager::history_energy_manager_daily_response(IChunkedResponse *response,
                                                          Ownership *response_ownership,
                                                          uint32_t response_owner_id)
{
    // date in local time to have the days properly aligned
    uint8_t year = history_energy_manager_daily.get("year")->asUint() - 2000;
    uint8_t month = history_energy_manager_daily.get("month")->asUint();

    uint8_t status;
    int rc = tf_warp_energy_manager_get_sd_energy_manager_daily_data_points(&device, year, month, 1, days_per_month(2000 + year, month), &status);

    //logger.printfln("history_energy_manager_daily_response: %d-%02d",
    //                2000 + year, month);

    if (rc != TF_E_OK || status != 0) {
        OwnershipGuard ownership_guard(response_ownership, response_owner_id);

        if (ownership_guard.have_ownership()) {
            response->begin(false);

            if (rc != TF_E_OK) {
                response->writef("Failed to get energy manager daily data point: error %d", rc);
                logger.printfln("energy_manager: Failed to get energy manager daily data point: error %d", rc);
            }
            else if (status != 0) {
                response->writef("Failed to get energy manager daily data point: status %u", status);
                logger.printfln("energy_manager: Failed to get energy manager daily data point: status %u", status);
            }

            response->flush();
            response->end("");
        }
    }
    else {
        StreamMetadata *metadata = &metadata_array[3];

        metadata->response = response;
        metadata->response_ownership = response_ownership;
        metadata->response_owner_id = response_owner_id;
        metadata->call_begin = true;
        metadata->write_comma = false;
        metadata->next_offset = 0;

        tf_warp_energy_manager_register_sd_energy_manager_daily_data_points_low_level_callback(&device, energy_manager_daily_data_points_handler, metadata);
    }

    check_bricklet_reachable(rc, "history_energy_manager_daily_response");
}
