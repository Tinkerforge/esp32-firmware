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

#define EVENT_LOG_PREFIX "em_energy_analys"

#include "em_energy_analysis.h"

#include <sys/time.h>

#include "modules/em_common/bricklet_bindings_constants.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modules/charge_manager/charge_manager_private.h"

#define MAX_DATA_AGE 30000 // milliseconds
#define DATA_INTERVAL_5MIN 5 // minutes
#define MAX_PENDING_DATA_POINTS 250

#if MODULE_EM_V1_AVAILABLE()
#define FLAGS_NO_DATA 0x80
#elif MODULE_EM_V2_AVAILABLE()
#define FLAGS_NO_DATA 0x8000
#endif

//#define DEBUG_LOGGING

static_assert(METERS_SLOTS <= 7, "Too many meters slots");

static const char *get_data_status_string(uint8_t status)
{
    switch (status) {
        case WEM_DATA_STATUS_OK:         return "OK";
        case WEM_DATA_SDATUS_SD_ERROR:   return "SD error";
        case WEM_DATA_SDATUS_LFS_ERROR:  return "LFS error";
        case WEM_DATA_SDATUS_QUEUE_FULL: return "queue full";
    }

    return "<unknown>";
}

void EMEnergyAnalysis::pre_setup()
{
    history_wallbox_5min = Config::Object({
        {"uid", Config::Uint32(0)},
        // date in UTC to avoid DST overlap problems
        {"year", Config::Uint(0, 2000, 2255)},
        {"month", Config::Uint(0, 1, 12)},
        {"day", Config::Uint(0, 1, 31)},
    });

    history_wallbox_daily = Config::Object({
        {"uid", Config::Uint32(0)},
        // date in local time to have the days properly aligned
        {"year", Config::Uint(0, 2000, 2255)},
        {"month", Config::Uint(0, 1, 12)},
    });

    history_energy_manager_5min = Config::Object({
        // date in UTC to avoid DST overlap problems
        {"year", Config::Uint(0, 2000, 2255)},
        {"month", Config::Uint(0, 1, 12)},
        {"day", Config::Uint(0, 1, 31)},
    });

    history_energy_manager_daily = Config::Object({
        // date in local time to have the days properly aligned
        {"year", Config::Uint(0, 2000, 2255)},
        {"month", Config::Uint(0, 1, 12)},
    });

    for (uint32_t slot = 0; slot < METERS_SLOTS; ++slot) {
        history_meter_setup_done[slot] = false;
        history_meter_power_value[slot] = NAN;
    }
}

void EMEnergyAnalysis::setup()
{
    if (!em_common.initialized)
        return;

    all_data_common = em_common.get_all_data_common();

    task_scheduler.scheduleWallClock([this]() {collect_data_points();}, 5_m, 100_ms, true);
    task_scheduler.scheduleWithFixedDelay([this]() {set_pending_data_points();}, 15_s, 100_ms);
    task_scheduler.scheduleOnce([this]() {this->show_blank_value_id_update_warnings = true;}, 250_ms);
}

void EMEnergyAnalysis::register_urls()
{
    if (!em_common.initialized)
        return;

    api.addResponse("energy_manager/history_wallbox_5min",         &history_wallbox_5min,         {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_wallbox_5min_response(response, ownership, owner_id);});
    api.addResponse("energy_manager/history_wallbox_daily",        &history_wallbox_daily,        {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_wallbox_daily_response(response, ownership, owner_id);});
    api.addResponse("energy_manager/history_energy_manager_5min",  &history_energy_manager_5min,  {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_energy_manager_5min_response(response, ownership, owner_id);});
    api.addResponse("energy_manager/history_energy_manager_daily", &history_energy_manager_daily, {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id){history_energy_manager_daily_response(response, ownership, owner_id);});
}

void EMEnergyAnalysis::register_events()
{
    for (uint32_t slot = 0; slot < METERS_SLOTS; ++slot) {
        if (meters.get_meter_class(slot) == MeterClassID::None) {
            continue;
        }

        // Passing no values will register on the ConfigRoot.
        event.registerEvent(meters.get_path(slot, Meters::PathType::ValueIDs), {}, [this, slot](const Config *config) {
            size_t count = config->count();

            if (count == 0) {
                if (show_blank_value_id_update_warnings) {
                    logger.printfln("Ignoring blank value IDs update from meter in slot %u.", slot);
                }
                return EventResult::OK;
            }

            history_meter_setup_done[slot] = true;

            uint32_t power_index;
            if (meters.get_cached_power_index(slot, &power_index)) {
                event.registerEvent(meters.get_path(slot, Meters::PathType::Values), {power_index}, [this, slot](const Config *config_power) {
                    update_history_meter_power(slot, config_power->asFloat());
                    return EventResult::OK;
                });
            } else {
                logger.printfln("Meter in slot %u doesn't provide power.", slot);
            }

            return EventResult::Deregister;
        });
    }
}

void EMEnergyAnalysis::update_history_meter_power(uint32_t slot, float power /* W, must not be NaN */)
{
    uint32_t now = millis();

    if (!isnan(history_meter_power_value[slot])) {
        uint32_t duration_ms;

        if (now >= history_meter_power_timestamp[slot]) {
            duration_ms = now - history_meter_power_timestamp[slot];
        } else {
            duration_ms = UINT32_MAX - history_meter_power_timestamp[slot] + now + 1;
        }

        double power_last_interval_w = (double)history_meter_power_value[slot];
        double duration_s = (double)duration_ms / 1000.0;
        double energy_ws = power_last_interval_w * duration_s;

        history_meter_power_sum[slot] += energy_ws;
        history_meter_power_duration[slot] += duration_s;

        if (!persistent_data_loaded) {
            persistent_data_loaded = load_persistent_data();
        }

        if (persistent_data_loaded && energy_ws != 0) {
            double energy_dawh = energy_ws / 36000.0;

            if (energy_dawh >= 0) {
                history_meter_energy_import[slot] += energy_dawh;
            } else {
                history_meter_energy_export[slot] += -energy_dawh;
            }

            save_persistent_data();
        }
    }

    history_meter_power_value[slot] = power;
    history_meter_power_timestamp[slot] = now;
}

void EMEnergyAnalysis::collect_data_points()
{
    struct timeval tv;
    struct tm utc;
    struct tm local;

    if (!rtc.clock_synced(&tv)) {
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

    // Even with scheduleWallClock still need to check if the slot has not already be written before the last boot
    uint32_t current_5min_slot = ((utc.tm_year * 366 + utc.tm_yday) * 24 + utc.tm_hour) * 12 + utc.tm_min / 5;

    if (current_5min_slot != last_history_5min_slot) {
        // 5min data
        for (size_t i = 0; i < charge_manager.charger_count; ++i) {
            auto &charger = charge_manager.charger_state[i];
            uint32_t last_update = charger.last_update;

            if (!deadline_elapsed(last_update + MAX_DATA_AGE)) {
                uint32_t uid = charger.uid;
#if MODULE_EM_V1_AVAILABLE()
                uint16_t flags = charger.charger_state; // v1: bit 0-2 = charger state, bit 7 = no data (read only)
#elif MODULE_EM_V2_AVAILABLE()
                uint16_t flags = charger.charger_state | (charger.phases << 3); // v2: bit 0-2 = charger state, bit 3-4 = phases, bit 15 = no data (read only)
#endif
                uint16_t power = UINT16_MAX;

                if (charger.meter_supported) {
                    float power_total_sum = charger.power_total_sum;
                    uint32_t power_total_count = charger.power_total_count;

                    charger.power_total_sum = 0;
                    charger.power_total_count = 0;

                    if (power_total_count > 0) {
                        power = clamp<uint64_t>(0,
                                                roundf(power_total_sum / power_total_count),
                                                UINT16_MAX - 1); // W
                    }
                }

                if (pending_data_points.size() > MAX_PENDING_DATA_POINTS) {
                    logger.printfln("Data point queue is full, dropping new data point");
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
                                charger.uid, last_update);
            }
#endif
        }

        if (all_data_common->is_valid && !deadline_elapsed(all_data_common->last_update + MAX_DATA_AGE)) {
            uint8_t flags = 0; // v1: bit 0 = 1p/3p, bit 1-2 = inputs, bit 3 = relay, bit 7 = no data (read only)
                               // v2: bit 0-3 = inputs, bit 4-5 = SG ready, bit 6-7 = relays, bit 15 = no data (read only)
            int32_t power[7] = {INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX}; // W
            int32_t price = INT32_MAX; // mct/kWh

            bool inputs[4];
            bool outputs[4];
            size_t input_len  = ARRAY_SIZE(inputs);
            size_t output_len = ARRAY_SIZE(outputs);

            em_common.get_input_output_states(inputs, &input_len, outputs, &output_len);
            // input_len and output_len now contain the actual value count

#if MODULE_EM_V1_AVAILABLE()
            flags |= power_manager.get_phases() > 1 ? 0b0001 : 0;
            flags |= inputs[0]                      ? 0b0010 : 0;
            flags |= inputs[1]                      ? 0b0100 : 0;
            flags |= outputs[0]                     ? 0b1000 : 0;
#elif MODULE_EM_V2_AVAILABLE()
            flags |= inputs[0]  ? (1 << 0) : 0;
            flags |= inputs[1]  ? (1 << 1) : 0;
            flags |= inputs[2]  ? (1 << 2) : 0;
            flags |= inputs[3]  ? (1 << 3) : 0;
            flags |= outputs[0] ? (1 << 4) : 0;
            flags |= outputs[1] ? (1 << 5) : 0;
            flags |= outputs[2] ? (1 << 6) : 0;
            flags |= outputs[3] ? (1 << 7) : 0;
#endif

            for (uint32_t slot = 0; slot < METERS_SLOTS; ++slot) {
                // FIXME: how to tell if meter data is stale?
                if (!isnan(history_meter_power_value[slot])) {
                    update_history_meter_power(slot, history_meter_power_value[slot]);

                    if (history_meter_power_duration[slot] > 0) {
                        power[slot] = clamp<int64_t>(INT32_MIN,
                                                     roundf(history_meter_power_sum[slot] / history_meter_power_duration[slot]),
                                                     INT32_MAX - 1); // W

                        history_meter_power_sum[slot] = 0;
                        history_meter_power_duration[slot] = 0;
                    }
                }
            }

#if MODULE_DAY_AHEAD_PRICES_AVAILABLE()
            {
                int32_t current_price_data;

                if (day_ahead_prices.get_current_price().try_unwrap(&current_price_data)) {
                    price = current_price_data + day_ahead_prices.get_grid_cost_plus_tax_plus_markup();
                }
            }
#endif

            if (pending_data_points.size() > MAX_PENDING_DATA_POINTS) {
                logger.printfln("Data point queue is full, dropping new data point");
            }
            else {
                pending_data_points.push_back([this, utc, local, flags, power, price] {
                    return set_energy_manager_5min_data_point(&utc, &local, flags, power, price);
                });
            }
        }

        // daily data
        for (size_t i = 0; i < charge_manager.charger_count; ++i) {
            const auto &charger = charge_manager.charger_state[i];
            uint32_t last_update = charger.last_update;

            if (!deadline_elapsed(last_update + MAX_DATA_AGE)) {
                bool have_data = false;
                uint32_t uid = charger.uid;
                uint32_t energy = UINT32_MAX;

                if (charger.meter_supported) {
                    have_data = true;
                    energy = clamp<uint64_t>(0,
                                             roundf(charger.energy_abs * 100.0),
                                             UINT32_MAX - 1); // kWh -> daWh
                }

                if (have_data) {
                    if (pending_data_points.size() > MAX_PENDING_DATA_POINTS) {
                        logger.printfln("Data point queue is full, dropping new data point");
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
                                    charger.uid);
                }
#endif
            }
#ifdef DEBUG_LOGGING
            else {
                logger.printfln("collect_data_points: skipping daily u%u, data too old %u",
                                charger.uid, last_update);
            }
#endif
        }

        bool have_data = false;
        uint32_t energy_import[7] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX}; // daWh
        uint32_t energy_export[7] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX}; // daWh
        int32_t price_min = INT32_MAX; // ct/kWh
        int32_t price_avg = INT32_MAX; // ct/kWh
        int32_t price_max = INT32_MAX; // ct/kWh

        micros_t max_age = micros_t{5 * 60 * 1000 * 1000};
        MeterValueAvailability availability;

        for (uint32_t slot = 0; slot < METERS_SLOTS; ++slot) {
            if (meters.get_meter_class(slot) == MeterClassID::None) {
                continue; // don't record integrated energy values for unconfigured meters
            }

            float total_import; // kWh
            availability = meters.get_energy_import(slot, &total_import, max_age);
            if (availability == MeterValueAvailability::Fresh) {
                if (isnan(total_import)) {
                    logger.printfln("Meter claims fresh 'import' value but returned NaN.");
                } else {
                    have_data = true;
                    energy_import[slot] = clamp<uint64_t>(0,
                                                          roundf(total_import * 100.0), // kWh -> daWh
                                                          UINT32_MAX - 1);
                }
            } else if (availability == MeterValueAvailability::Unavailable) {
                have_data = true;
                energy_import[slot] = clamp<uint64_t>(0, roundf(history_meter_energy_import[slot]), UINT32_MAX - 1);
            } else {
                // Value availability currently unknown or value is stale.
            }

            float total_export; // kWh
            availability = meters.get_energy_export(slot, &total_export, max_age);
            if (availability == MeterValueAvailability::Fresh) {
                if (isnan(total_export)) {
                    logger.printfln("Meter claims fresh 'export' value but returned NaN.");
                } else {
                    have_data = true;
                    energy_export[slot] = clamp<uint64_t>(0,
                                                          roundf(total_export * 100.0), // kWh -> daWh
                                                          UINT32_MAX - 1);
                }
            } else if (availability == MeterValueAvailability::Unavailable) {
                have_data = true;
                energy_export[slot] = clamp<uint64_t>(0, roundf(history_meter_energy_export[slot]), UINT32_MAX - 1);
            } else {
                // Value availability currently unknown or value is stale.
            }
        }

#if MODULE_DAY_AHEAD_PRICES_AVAILABLE()
        {
            int32_t minimum_price_today_data;
            int32_t average_price_today_data;
            int32_t maximum_price_today_data;
            int32_t grid_cost_plus_tax_plus_markup = day_ahead_prices.get_grid_cost_plus_tax_plus_markup();

            if (day_ahead_prices.get_minimum_price_today().try_unwrap(&minimum_price_today_data)) {
                have_data = true;
                price_min = (minimum_price_today_data + grid_cost_plus_tax_plus_markup) / 1000; // mct/kWh -> ct/kWh (floor)
            }

            if (day_ahead_prices.get_average_price_today().try_unwrap(&average_price_today_data)) {
                have_data = true;
                price_avg = ((average_price_today_data + grid_cost_plus_tax_plus_markup) + 500) / 1000; // mct/kWh -> ct/kWh (round)
            }

            if (day_ahead_prices.get_maximum_price_today().try_unwrap(&maximum_price_today_data)) {
                have_data = true;
                price_max = ((maximum_price_today_data + grid_cost_plus_tax_plus_markup) + 999) / 1000; // mct/kWh -> ct/kWh (ceil)
            }
        }
#endif

        if (have_data) {
            if (pending_data_points.size() > MAX_PENDING_DATA_POINTS) {
                logger.printfln("Data point queue is full, dropping new data point");
            }
            else {
                pending_data_points.push_back([this, local, energy_import, energy_export, price_min, price_avg, price_max] {
                    return set_energy_manager_daily_data_point(&local, energy_import, energy_export, price_min, price_avg, price_max);
                });
            }
        }

        last_history_5min_slot = current_5min_slot;

        save_persistent_data();
    }
}

void EMEnergyAnalysis::set_pending_data_points()
{
    if (pending_data_points.empty()) {
        return;
    }

    if (!pending_data_points.front()()) {
        return;
    }

    pending_data_points.pop_front();
}

#define DATA_STORAGE_PAGE_SIZE 63
#define DATA_STORAGE_PAGE_COUNT 3

// this struct is 8 byte larger than initially expected, because the 8 byte
// alignment requirement for the double values was missed. but explicitly
// adding the extra padding now and making the struct packed results in
// "packed attribute causes inefficient alignment" warnings for every struct
// member after padding0. therefore, keeping the struct as is, because it
// has been like this in released versions and the memory layout of the
// struct cannot be changed anymore.
struct PersistentDataV1 {
    uint8_t version;
    uint8_t padding0[3];
    uint32_t last_history_5min_slot;
    uint32_t last_history_daily_slot; // unused
    // 4 byte of padding here for 8 byte alignment of the double values
    double history_meter_energy_import;
    double history_meter_energy_export;
    uint16_t padding1;
    uint16_t checksum;
    // 4 byte of padding here for 8 byte alignment of the double values
};

struct PersistentDataV2 {
    double history_meter_energy_import[6];
    double history_meter_energy_export[6];
    uint8_t version;
    uint8_t padding0;
    uint16_t checksum;
    uint32_t padding1;
};

static_assert(sizeof(PersistentDataV1) == 40);
static_assert(sizeof(PersistentDataV2) == 104);

bool EMEnergyAnalysis::load_persistent_data()
{
    uint8_t buf[DATA_STORAGE_PAGE_SIZE * DATA_STORAGE_PAGE_COUNT] = {0};

    for (uint8_t page = 0; page < DATA_STORAGE_PAGE_COUNT; ++page) {
        uint8_t status = WEM_DATA_STORAGE_STATUS_BUSY;

        if (em_common.wem_get_data_storage(page, &status, buf + (DATA_STORAGE_PAGE_SIZE * page)) != TF_E_OK) {
            return false;
        }

        switch (status) {
        case WEM_DATA_STORAGE_STATUS_BUSY:
            logger.printfln("Persistent data not available yet, trying again later");
            return false;

        case WEM_DATA_STORAGE_STATUS_NOT_FOUND:
            logger.printfln("Persistent data not found, first boot?");
            return true;

        case WEM_DATA_STORAGE_STATUS_OK:
            break;

        default:
            logger.printfln("Persistent data has unknown status, trying again later: %d", status);
            return false;
        }
    }

    load_persistent_data_v1(buf);
    load_persistent_data_v2(buf + sizeof(PersistentDataV1));

#ifdef DEBUG_LOGGING
    logger.printfln("load_persistent_data: 5min slot %u, energy %f/%f %f/%f %f/%f %f/%f %f/%f %f/%f %f/%f",
                    last_history_5min_slot,
                    history_meter_energy_import[0],
                    history_meter_energy_export[0],
                    history_meter_energy_import[1],
                    history_meter_energy_export[1],
                    history_meter_energy_import[2],
                    history_meter_energy_export[2],
                    history_meter_energy_import[3],
                    history_meter_energy_export[3],
                    history_meter_energy_import[4],
                    history_meter_energy_export[4],
                    history_meter_energy_import[5],
                    history_meter_energy_export[5],
                    history_meter_energy_import[6],
                    history_meter_energy_export[6]);
#endif

    return true;
}

void EMEnergyAnalysis::load_persistent_data_v1(uint8_t *buf)
{
    PersistentDataV1 data_v1;
    memcpy(&data_v1, buf, sizeof(data_v1));

    PersistentDataV1 zero_v1;
    memset(&zero_v1, 0, sizeof(zero_v1));

    if (memcmp(&data_v1, &zero_v1, sizeof(data_v1)) == 0) {
        logger.printfln("Persistent data v1 all zero, first boot?");
        return;
    }

    if (internet_checksum((uint8_t *)&data_v1, sizeof(data_v1)) != 0) {
        logger.printfln("Checksum mismatch for persistent data v1");
        return;
    }

    if (data_v1.version != 1) {
        logger.printfln("Unexpected version %u for persistent data v1", data_v1.version);
        return;
    }

    last_history_5min_slot = data_v1.last_history_5min_slot;
    history_meter_energy_import[0] = data_v1.history_meter_energy_import;
    history_meter_energy_export[0] = data_v1.history_meter_energy_export;
}

void EMEnergyAnalysis::load_persistent_data_v2(uint8_t *buf)
{
    PersistentDataV2 data_v2;
    memcpy(&data_v2, buf, sizeof(data_v2));

    PersistentDataV2 zero_v2;
    memset(&zero_v2, 0, sizeof(zero_v2));

    if (memcmp(&data_v2, &zero_v2, sizeof(data_v2)) == 0) {
        logger.printfln("Persistent data v2 all zero, first boot?");
        return;
    }

    if (internet_checksum((uint8_t *)&data_v2, sizeof(data_v2)) != 0) {
        logger.printfln("Checksum mismatch for persistent data v2");
        return;
    }

    if (data_v2.version != 2) {
        logger.printfln("Unexpected version %u for persistent data v2", data_v2.version);
        return;
    }

    for (uint32_t slot = 1; slot < METERS_SLOTS; ++slot) {
        history_meter_energy_import[slot] = data_v2.history_meter_energy_import[slot - 1];
        history_meter_energy_export[slot] = data_v2.history_meter_energy_export[slot - 1];
    }
}

void EMEnergyAnalysis::save_persistent_data()
{
    PersistentDataV1 data_v1;
    memset(&data_v1, 0, sizeof(data_v1));

    PersistentDataV2 data_v2;
    memset(&data_v2, 0, sizeof(data_v2));

#ifdef DEBUG_LOGGING
    logger.printfln("save_persistent_data: 5min slot %u, energy %f/%f %f/%f %f/%f %f/%f %f/%f %f/%f %f/%f",
                    last_history_5min_slot,
                    history_meter_energy_import[0],
                    history_meter_energy_export[0],
                    history_meter_energy_import[1],
                    history_meter_energy_export[1],
                    history_meter_energy_import[2],
                    history_meter_energy_export[2],
                    history_meter_energy_import[3],
                    history_meter_energy_export[3],
                    history_meter_energy_import[4],
                    history_meter_energy_export[4],
                    history_meter_energy_import[5],
                    history_meter_energy_export[5],
                    history_meter_energy_import[6],
                    history_meter_energy_export[6]);
#endif

    data_v1.version = 1;
    data_v1.last_history_5min_slot = last_history_5min_slot;
    data_v1.last_history_daily_slot = 0; // unused
    data_v1.history_meter_energy_import = history_meter_energy_import[0];
    data_v1.history_meter_energy_export = history_meter_energy_export[0];
    data_v1.checksum = internet_checksum((uint8_t *)&data_v1, sizeof(data_v1));

    for (uint32_t slot = 1; slot < METERS_SLOTS; ++slot) {
        data_v2.history_meter_energy_import[slot - 1] = history_meter_energy_import[slot];
        data_v2.history_meter_energy_export[slot - 1] = history_meter_energy_export[slot];
    }

    data_v2.version = 2;
    data_v2.checksum = internet_checksum((uint8_t *)&data_v2, sizeof(data_v2));

    uint8_t buf[DATA_STORAGE_PAGE_SIZE * DATA_STORAGE_PAGE_COUNT] = {0};
    memcpy(buf, &data_v1, sizeof(data_v1));
    memcpy(buf + sizeof(data_v1), &data_v2, sizeof(data_v2));

    for (uint8_t page = 0; page < DATA_STORAGE_PAGE_COUNT; ++page) {
        em_common.wem_set_data_storage(page, buf + (DATA_STORAGE_PAGE_SIZE * page));
    }
}

bool EMEnergyAnalysis::set_wallbox_5min_data_point(const struct tm *utc, const struct tm *local, uint32_t uid, uint16_t flags, uint16_t power /* W */)
{
    uint8_t status;
    uint8_t utc_year = utc->tm_year - 100;
    uint8_t utc_month = utc->tm_mon + 1;
    uint8_t utc_day = utc->tm_mday;
    uint8_t utc_hour = utc->tm_hour;
    uint8_t utc_minute = (utc->tm_min / 5) * 5;
    int rc = em_common.wem_set_sd_wallbox_data_point(uid,
                                                     utc_year,
                                                     utc_month,
                                                     utc_day,
                                                     utc_hour,
                                                     utc_minute,
                                                     flags,
                                                     power,
                                                     &status);

#ifdef DEBUG_LOGGING
    logger.printfln("set_wallbox_5min_data_point: u%u %d-%02d-%02d %02d:%02d f%u p%u",
                    uid, 2000 + utc_year, utc_month, utc_day, utc_hour, utc_minute, flags, power);
#endif

    if (rc != TF_E_OK) {
        if (rc == TF_E_TIMEOUT || em_common.device_module_is_in_bootloader(rc)) {
            return false; // retry
        }
        else {
            logger.printfln("Failed to set wallbox 5min data point: error %d", rc);
            return true;
        }
    }
    else if (status != 0) {
        if (status == WEM_DATA_SDATUS_QUEUE_FULL) {
            return false; // retry
        }
        else {
            logger.printfln("Failed to set wallbox 5min data point: status (%s, %u)", get_data_status_string(status), status);
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

bool EMEnergyAnalysis::set_wallbox_daily_data_point(const struct tm *local, uint32_t uid, uint32_t energy /* daWh */)
{
    uint8_t status;
    uint8_t year = local->tm_year - 100;
    uint8_t month = local->tm_mon + 1;
    uint8_t day = local->tm_mday;
    int rc = em_common.wem_set_sd_wallbox_daily_data_point(uid, year, month, day, energy, &status);

#ifdef DEBUG_LOGGING
    logger.printfln("set_wallbox_daily_data_point: u%u %d-%02d-%02d e%u",
                    uid, 2000 + year, month, day, energy);
#endif

    if (rc != TF_E_OK) {
        if (rc == TF_E_TIMEOUT || em_common.device_module_is_in_bootloader(rc)) {
            return false; // retry
        }
        else {
            logger.printfln("Failed to set wallbox daily data point: error %d", rc);
            return true;
        }
    }
    else if (status != 0) {
        if (status == WEM_DATA_SDATUS_QUEUE_FULL) {
            return false; // retry
        }
        else {
            logger.printfln("Failed to set wallbox daily data point: status (%s, %u)", get_data_status_string(status), status);
            return true;
        }
    }
    else {
        char energy_str[12] = "null";

        if (energy != UINT32_MAX) {
            snprintf(energy_str, sizeof(energy_str), "%.2f", (double)energy / 100.0); // daWh -> kWh
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

bool EMEnergyAnalysis::set_energy_manager_5min_data_point(const struct tm *utc,
                                                          const struct tm *local,
                                                          const uint16_t flags,
                                                          const int32_t power[7] /* W */,
                                                          const int32_t price /* mct/kWh */)
{
    uint32_t price_bits = UINT32_MAX;

    if (price != INT32_MAX) {
        price_bits = (uint32_t)price;
    }

    uint8_t status;
    uint8_t utc_year = utc->tm_year - 100;
    uint8_t utc_month = utc->tm_mon + 1;
    uint8_t utc_day = utc->tm_mday;
    uint8_t utc_hour = utc->tm_hour;
    uint8_t utc_minute = (utc->tm_min / 5) * 5;
    int rc = em_common.wem_set_sd_energy_manager_data_point(utc_year,
                                                            utc_month,
                                                            utc_day,
                                                            utc_hour,
                                                            utc_minute,
                                                            flags,
                                                            power[0],
                                                            &power[1],
                                                            price_bits,
                                                            &status);

#ifdef DEBUG_LOGGING
    logger.printfln("set_energy_manager_5min_data_point: %d-%02d-%02d %02d:%02d f%u po%d,%d,%d,%d,%d,%d,%d pr%d",
                    2000 + utc_year, utc_month, utc_day, utc_hour, utc_minute, flags, power[0], power[1], power[2], power[3], power[4], power[5], power[6], price);
#endif

    if (rc != TF_E_OK) {
        if (rc == TF_E_TIMEOUT || em_common.device_module_is_in_bootloader(rc)) {
            return false; // retry
        }
        else {
            logger.printfln("Failed to set energy manager 5min data point: error %d", rc);
            return true;
        }
    }
    else if (status != 0) {
        if (status == WEM_DATA_SDATUS_QUEUE_FULL) {
            return false; // retry
        }
        else {
            logger.printfln("Failed to set energy manager 5min data point: status (%s, %u)", get_data_status_string(status), status);
            return true;
        }
    }
    else {
        char power_str[7][12] = {"null", "null", "null", "null", "null", "null", "null"};
        char price_str[12] = "null";

        for (int i = 0; i < 7; ++i) {
            if (power[i] != INT32_MAX) {
                snprintf(power_str[i], sizeof(power_str[i]), "%d", power[i]);
            }
        }

        if (price != INT32_MAX) {
            snprintf(price_str, sizeof(price_str), "%.3f", (double)price / 1000.0); // mct/kWh -> ct/kWh
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
                                    "\"power\":[%s,%s,%s,%s,%s,%s,%s],"
                                    "\"price\":%s}}\n",
                                   2000U + local_year,
                                   local_month,
                                   local_day,
                                   local_hour,
                                   local_minute,
                                   flags,
                                   power_str[0],
                                   power_str[1],
                                   power_str[2],
                                   power_str[3],
                                   power_str[4],
                                   power_str[5],
                                   power_str[6],
                                   price_str);

        if (buf_written > 0) {
            ws.web_sockets.sendToAllOwned(buf, buf_written);
        }

        return true;
    }
}

#define PRICE_UINT10_MAX 1023
#define PRICE_INT10_MAX  911
#define PRICE_INT10_MIN  (PRICE_INT10_MAX - PRICE_UINT10_MAX)

static uint32_t price_to_10bit(int32_t price)
{
    if (price == INT32_MAX) {
        return PRICE_UINT10_MAX;
    }

    return clamp(PRICE_INT10_MIN, price, PRICE_INT10_MAX) - PRICE_INT10_MIN;
}

static int32_t price_from_10bit(uint32_t price)
{
    if (price >= PRICE_UINT10_MAX) {
        return INT32_MAX;
    }

    return price + PRICE_INT10_MIN;
}

bool EMEnergyAnalysis::set_energy_manager_daily_data_point(const struct tm *local,
                                                           const uint32_t energy_import[7] /* daWh */,
                                                           const uint32_t energy_export[7] /* daWh */,
                                                           int32_t price_min /* ct/kWh */,
                                                           int32_t price_avg /* ct/kWh */,
                                                           int32_t price_max /* ct/kWh */)
{
    uint8_t status;
    uint8_t year = local->tm_year - 100;
    uint8_t month = local->tm_mon + 1;
    uint8_t day = local->tm_mday;
    uint32_t price_bits = (price_to_10bit(price_min) << 20) | (price_to_10bit(price_avg) << 10) | price_to_10bit(price_max);
    int rc = em_common.wem_set_sd_energy_manager_daily_data_point(year,
                                                                  month,
                                                                  day,
                                                                  energy_import[0],
                                                                  energy_export[0],
                                                                  &energy_import[1],
                                                                  &energy_export[1],
                                                                  price_bits,
                                                                  &status);

#ifdef DEBUG_LOGGING
    logger.printfln("set_energy_manager_daily_data_point: %d-%02d-%02d ei%u,%u,%u,%u,%u,%u,%u ee%u,%u,%u,%u,%u,%u,%u p%d,%d,%d",
                    2000 + year, month, day,
                    energy_import[0], energy_import[1], energy_import[2], energy_import[3], energy_import[4], energy_import[5], energy_import[6],
                    energy_export[0], energy_export[1], energy_export[2], energy_export[3], energy_export[4], energy_export[5], energy_export[6],
                    price_min, price_avg, price_max);
#endif

    if (rc != TF_E_OK) {
        if (rc == TF_E_TIMEOUT || em_common.device_module_is_in_bootloader(rc)) {
            return false; // retry
        }
        else {
            logger.printfln("Failed to set energy manager daily data point: error %i", rc);
            return true;
        }
    }
    else if (status != 0) {
        if (status == WEM_DATA_SDATUS_QUEUE_FULL) {
            return false; // retry
        }
        else {
            logger.printfln("Failed to set energy manager daily data point: status (%s, %u)", get_data_status_string(status), status);
            return true;
        }
    }
    else {
        char energy_import_str[7][13] = {"null", "null", "null", "null", "null", "null", "null"};
        char energy_export_str[7][13] = {"null", "null", "null", "null", "null", "null", "null"};
        char price_min_str[12] = "null";
        char price_avg_str[12] = "null";
        char price_max_str[12] = "null";

        for (int i = 0; i < 7; ++i) {
            if (energy_import[i] != UINT32_MAX) {
                snprintf(energy_import_str[i], sizeof(energy_import_str[i]), "%.2f", (double)energy_import[i] / 100.0); // daWh -> kWh
            }

            if (energy_export[i] != UINT32_MAX) {
                snprintf(energy_export_str[i], sizeof(energy_export_str[i]), "%.2f", (double)energy_export[i] / 100.0); // daWh -> kWh
            }
        }

        if (price_min != INT32_MAX) {
            snprintf(price_min_str, sizeof(price_min_str), "%d", price_min);
        }

        if (price_avg != INT32_MAX) {
            snprintf(price_avg_str, sizeof(price_avg_str), "%d", price_avg);
        }

        if (price_max != INT32_MAX) {
            snprintf(price_max_str, sizeof(price_max_str), "%d", price_max);
        }

        char *buf;
        int buf_written = asprintf(&buf,
                                   "{\"topic\":\"energy_manager/history_energy_manager_daily_changed\","
                                    "\"payload\":{"
                                    "\"year\":%u,"
                                    "\"month\":%u,"
                                    "\"day\":%u,"
                                    "\"energy_import\":[%s,%s,%s,%s,%s,%s,%s],"
                                    "\"energy_export\":[%s,%s,%s,%s,%s,%s,%s],"
                                    "\"price_min\":%s,"
                                    "\"price_avg\":%s,"
                                    "\"price_max\":%s}}\n",
                                   2000U + year,
                                   month,
                                   day,
                                   energy_import_str[0],
                                   energy_import_str[1],
                                   energy_import_str[2],
                                   energy_import_str[3],
                                   energy_import_str[4],
                                   energy_import_str[5],
                                   energy_import_str[6],
                                   energy_export_str[0],
                                   energy_export_str[1],
                                   energy_export_str[2],
                                   energy_export_str[3],
                                   energy_export_str[4],
                                   energy_export_str[5],
                                   energy_export_str[6],
                                   price_min_str,
                                   price_avg_str,
                                   price_max_str);

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
    uint32_t seqnum;
    uint32_t uid;
    uint8_t utc_end_year;
    uint8_t utc_end_month;
    uint8_t utc_end_day;
    uint16_t utc_end_slots;
} StreamMetadata;

static StreamMetadata metadata_array[4];

struct [[gnu::packed]] Wallbox5minData {
#if MODULE_EM_V1_AVAILABLE()
    uint8_t flags;  // v1: bit 0-2 = charger state, bit 7 = no data (read only)
#elif MODULE_EM_V2_AVAILABLE()
    uint16_t flags; // v2: bit 0-2 = charger state, bit 3-4 = phases, bit 15 = no data (read only)
#endif
    uint16_t power; // W
};

static void wallbox_5min_data_points_handler(void *do_not_use, uint16_t data_length, uint16_t data_chunk_offset, uint8_t data_chunk_data[60], void *user_data)
{
    StreamMetadata *metadata = (StreamMetadata *)user_data;
    IChunkedResponse *response = metadata->response;
    bool write_success = true;
    OwnershipGuard ownership_guard(metadata->response_ownership, metadata->response_owner_id);

    if (!ownership_guard.have_ownership()) {
        em_common.wem_register_sd_wallbox_data_points_low_level_callback(nullptr, nullptr);
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
        logger.printfln("Failed to get wallbox 5min data point: seqnum %u, stream out of sync (%u != %u)", metadata->seqnum, metadata->next_offset, data_chunk_offset);

        if (write_success) {
            write_success = response->write("]");
        }

        write_success &= response->flush();
        response->end(write_success ? "" : "write error");

        em_common.wem_register_sd_wallbox_data_points_low_level_callback(nullptr, nullptr);
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

        if ((p->flags & FLAGS_NO_DATA) == 0) {
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

                em_common.wem_register_sd_wallbox_data_points_low_level_callback(nullptr, nullptr);
            } else {
                task_scheduler.scheduleOnce([metadata, response]{
                    uint8_t status;
                    int rc = em_common.wem_get_sd_wallbox_data_points(metadata->uid,
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
                            logger.printfln("Failed to continue getting wallbox 5min data point: seqnum %u, error %d", metadata->seqnum, rc);
                        }
                        else if (status != 0) {
                            logger.printfln("Failed to continue getting wallbox 5min data point: seqnum %u, status (%s, %u)", metadata->seqnum, get_data_status_string(status), status);
                        }

                        OwnershipGuard ownership_guard2(metadata->response_ownership, metadata->response_owner_id);

                        if (ownership_guard2.have_ownership()) {
                            response->flush();
                            response->end("continuation error");
                        }

                        em_common.wem_register_sd_wallbox_data_points_low_level_callback(nullptr, nullptr);
                    }
                });
            }
        }
        else {
            if (write_success) {
                write_success = response->write("]");
            }

            write_success &= response->flush();
            response->end(write_success ? "" : "write error");

            em_common.wem_register_sd_wallbox_data_points_low_level_callback(nullptr, nullptr);
        }

        return;
    }

    if (!write_success) {
        response->end("write error");

        em_common.wem_register_sd_wallbox_data_points_low_level_callback(nullptr, nullptr);
        return;
    }
}

void EMEnergyAnalysis::history_wallbox_5min_response(IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id)
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

    uint32_t seqnum = history_request_seqnum++;
    uint8_t status;
    int rc;

    if (utc_start_slots > 0) {
        rc = em_common.wem_get_sd_wallbox_data_points(uid,
                                                      utc_start_year,
                                                      utc_start_month,
                                                      utc_start_day,
                                                      utc_start_hour,
                                                      utc_start_minute,
                                                      utc_start_slots,
                                                      &status);
    } else {
        rc = em_common.wem_get_sd_wallbox_data_points(uid,
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
                response->writef("Failed to get wallbox 5min data point: seqnum %u, error %d", seqnum, rc);
                logger.printfln("Failed to get wallbox 5min data point: seqnum %u, error %d", seqnum, rc);
            }
            else if (status != 0) {
                response->writef("Failed to get wallbox 5min data point: seqnum %u, status (%s, %u)", seqnum, get_data_status_string(status), status);
                logger.printfln("Failed to get wallbox 5min data point: seqnum %u, status (%s, %u)", seqnum, get_data_status_string(status), status);
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
        metadata->seqnum = seqnum;
        metadata->uid = uid;
        metadata->utc_end_year = utc_end_year;
        metadata->utc_end_month = utc_end_month;
        metadata->utc_end_day = utc_end_day;
        metadata->utc_end_slots = utc_end_slots;

        em_common.wem_register_sd_wallbox_data_points_low_level_callback(wallbox_5min_data_points_handler, metadata);
    }
}

static void wallbox_daily_data_points_handler(void *do_not_use,
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
        em_common.wem_register_sd_wallbox_daily_data_points_low_level_callback(nullptr, nullptr);
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
        logger.printfln("Failed to get wallbox daily data point: seqnum %u, stream out of sync (%u != %u)",
                        metadata->seqnum, metadata->next_offset, data_chunk_offset);

        if (write_success) {
            write_success = response->write("]");
        }

        write_success &= response->flush();
        response->end(write_success ? "" : "write error");

        em_common.wem_register_sd_wallbox_daily_data_points_low_level_callback(nullptr, nullptr);
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
            write_success = response->writef("%.2f", (double)data_chunk_data[i] / 100.0); // daWh -> kWh
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

        em_common.wem_register_sd_wallbox_daily_data_points_low_level_callback(nullptr, nullptr);
        return;
    }

    if (!write_success) {
        response->end("write error");

        em_common.wem_register_sd_wallbox_daily_data_points_low_level_callback(nullptr, nullptr);
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

void EMEnergyAnalysis::history_wallbox_daily_response(IChunkedResponse *response,
                                                      Ownership *response_ownership,
                                                      uint32_t response_owner_id)
{
    uint32_t uid = history_wallbox_daily.get("uid")->asUint();

    // date in local time to have the days properly aligned
    uint8_t year = history_wallbox_daily.get("year")->asUint() - 2000;
    uint8_t month = history_wallbox_daily.get("month")->asUint();

    uint32_t seqnum = history_request_seqnum++;
    uint8_t status;
    int rc = em_common.wem_get_sd_wallbox_daily_data_points(uid, year, month, 1, days_per_month(2000 + year, month), &status);

    //logger.printfln("history_wallbox_daily_response: u%u %d-%02d",
    //                uid, 2000 + year, month);

    if (rc != TF_E_OK || status != 0) {
        OwnershipGuard ownership_guard(response_ownership, response_owner_id);

        if (ownership_guard.have_ownership()) {
            response->begin(false);

            if (rc != TF_E_OK) {
                response->writef("Failed to get wallbox daily data point: seqnum %u, error %d", seqnum, rc);
                logger.printfln("Failed to get wallbox daily data point: seqnum %u, error %d", seqnum, rc);
            }
            else if (status != 0) {
                response->writef("Failed to get wallbox daily data point: seqnum %u, status (%s, %u)", seqnum, get_data_status_string(status), status);
                logger.printfln("Failed to get wallbox daily data point: seqnum %u, status (%s, %u)", seqnum, get_data_status_string(status), status);
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
        metadata->seqnum = seqnum;

        em_common.wem_register_sd_wallbox_daily_data_points_low_level_callback(wallbox_daily_data_points_handler, metadata);
    }
}

struct [[gnu::packed]] EnergyManager5MinData {
#if MODULE_EM_V1_AVAILABLE()
    uint8_t flags;  // v1: bit 0 = 1p/3p, bit 1-2 = inputs, bit 3 = relay, bit 7 = no data (read only)
#elif MODULE_EM_V2_AVAILABLE()
    uint16_t flags; // v2: bit 0-3 = inputs, bit 4-5 = SG ready, bit 6-7 = relays, bit 15 = no data (read only)
#endif
    int32_t power[7]; // W
    uint32_t price_bits; // mct/kWh
};

static void energy_manager_5min_data_points_handler(void *do_not_use,
                                                    uint16_t data_length,
                                                    uint16_t data_chunk_offset,
                                                    uint8_t data_chunk_data[34], // v1: 33 byte, v2: 34 byte
                                                    void *user_data)
{
    StreamMetadata *metadata = (StreamMetadata *)user_data;
    IChunkedResponse *response = metadata->response;
    bool write_success = true;
    OwnershipGuard ownership_guard(metadata->response_ownership, metadata->response_owner_id);

    if (!ownership_guard.have_ownership()) {
        em_common.wem_register_sd_energy_manager_data_points_low_level_callback(nullptr, nullptr);
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
        logger.printfln("Failed to get energy manager 5min data point: seqnum %u, stream out of sync (%u != %u)",
                        metadata->seqnum, metadata->next_offset, data_chunk_offset);

        if (write_success) {
            write_success = response->write("]");
        }

        write_success &= response->flush();
        response->end(write_success ? "" : "write error");

        em_common.wem_register_sd_energy_manager_data_points_low_level_callback(nullptr, nullptr);
        return;
    }

    uint16_t actual_length = data_length - data_chunk_offset;

    if (actual_length >= sizeof(EnergyManager5MinData)) {
        if (metadata->write_comma && write_success) {
            write_success = response->write(",", 1);
        }

        EnergyManager5MinData *p = (EnergyManager5MinData *)data_chunk_data;

        if (write_success) {
            if ((p->flags & FLAGS_NO_DATA) == 0) {
                write_success = response->writef("%u", p->flags);
            } else {
                write_success = response->writef("null");

                for (int k = 0; k < 7; ++k) {
                    p->power[k] = INT32_MAX;
                }

                p->price_bits = UINT32_MAX;
            }
        }

        if (write_success) {
            for (int k = 0; k < 7 && write_success; ++k) {
                if (p->power[k] != INT32_MAX) {
                    write_success = response->writef(",%d", p->power[k]);
                } else {
                    write_success = response->writef(",null");
                }
            }
        }

        if (write_success) {
            if (p->price_bits != UINT32_MAX) {
                write_success = response->writef(",%.3f", (double)(int32_t)p->price_bits / 1000.0); // mct/kWh -> ct/kWh
            } else {
                write_success = response->writef(",null");
            }
        }
    }

    metadata->write_comma = true;
#if MODULE_EM_V1_AVAILABLE()
    metadata->next_offset += 33;
#elif MODULE_EM_V2_AVAILABLE()
    metadata->next_offset += 34;
#endif

    if (metadata->next_offset >= data_length) {
        if (metadata->utc_end_slots > 0) {
            if (!write_success) {
                response->flush();
                response->end("write error");

                em_common.wem_register_sd_energy_manager_data_points_low_level_callback(nullptr, nullptr);
            } else {
                task_scheduler.scheduleOnce([metadata, response]{
                    uint8_t status;
                    int rc = em_common.wem_get_sd_energy_manager_data_points(metadata->utc_end_year,
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
                            logger.printfln("Failed to continue getting energy manager 5min data point: seqnum %u, error %d", metadata->seqnum, rc);
                        }
                        else if (status != 0) {
                            logger.printfln("Failed to continue getting energy manager 5min data point: seqnum %u, status (%s, %u)", metadata->seqnum, get_data_status_string(status), status);
                        }

                        OwnershipGuard ownership_guard2(metadata->response_ownership, metadata->response_owner_id);

                        if (ownership_guard2.have_ownership()) {
                            response->flush();
                            response->end("continuation error");
                        }

                        em_common.wem_register_sd_energy_manager_data_points_low_level_callback(nullptr, nullptr);
                    }
                });
            }
        }
        else {
            if (write_success) {
                write_success = response->write("]");
            }

            write_success &= response->flush();
            response->end(write_success ? "" : "write error");

            em_common.wem_register_sd_energy_manager_data_points_low_level_callback(nullptr, nullptr);
        }

        return;
    }

    if (!write_success) {
        response->end("write error");

        em_common.wem_register_sd_energy_manager_data_points_low_level_callback(nullptr, nullptr);
        return;
    }
}

void EMEnergyAnalysis::history_energy_manager_5min_response(IChunkedResponse *response,
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

    uint32_t seqnum = history_request_seqnum++;
    uint8_t status;
    int rc;

    if (utc_start_slots > 0) {
        rc = em_common.wem_get_sd_energy_manager_data_points(utc_start_year,
                                                             utc_start_month,
                                                             utc_start_day,
                                                             utc_start_hour,
                                                             utc_start_minute,
                                                             utc_start_slots,
                                                             &status);
    }
    else {
        rc = em_common.wem_get_sd_energy_manager_data_points(utc_end_year,
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
                response->writef("Failed to get energy manager 5min data point: seqnum %u, error %d", seqnum, rc);
                logger.printfln("Failed to get energy manager 5min data point: seqnum %u, error %d", seqnum, rc);
            }
            else if (status != 0) {
                response->writef("Failed to get energy manager 5min data point: seqnum %u, status (%s, %u)", seqnum, get_data_status_string(status), status);
                logger.printfln("Failed to get energy manager 5min data point: seqnum %u, status (%s, %u)", seqnum, get_data_status_string(status), status);
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
        metadata->seqnum = seqnum;
        metadata->utc_end_year = utc_end_year;
        metadata->utc_end_month = utc_end_month;
        metadata->utc_end_day = utc_end_day;
        metadata->utc_end_slots = utc_end_slots;

        em_common.wem_register_sd_energy_manager_data_points_low_level_callback(energy_manager_5min_data_points_handler, metadata);
    }
}

static void energy_manager_daily_data_points_handler(void *do_not_use,
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
        em_common.wem_register_sd_energy_manager_daily_data_points_low_level_callback(nullptr, nullptr);
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
        logger.printfln("Failed to get energy manager daily data point: seqnum %u, stream out of sync (%u != %u)",
                        metadata->seqnum, metadata->next_offset, data_chunk_offset);

        if (write_success) {
            write_success = response->write("]");
        }

        write_success &= response->flush();
        response->end(write_success ? "" : "write error");

        em_common.wem_register_sd_energy_manager_daily_data_points_low_level_callback(nullptr, nullptr);
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

    // the data is stored as:
    // i0, e0, i1, i2, i3, i4, i5, i6, e1, e2, e3, e4, e5, e6, p
    // but we want to report it as:
    // i0, i1, i2, i3, i4, i5, i6, e0, e1, e2, e3, e4, e5, e6, p
    uint32_t energy_export_0 = data_chunk_data[1];
    memmove(&data_chunk_data[1], &data_chunk_data[2], sizeof(uint32_t) * 6);
    data_chunk_data[7] = energy_export_0;

    for (i = 0; i < actual_length && write_success; ++i) {
        if (i == 14) {
            if (data_chunk_data[i] != UINT32_MAX) {
                int32_t price_min = price_from_10bit((data_chunk_data[i] >> 20) & 0x3FF);
                int32_t price_avg = price_from_10bit((data_chunk_data[i] >> 10) & 0x3FF);
                int32_t price_max = price_from_10bit( data_chunk_data[i]        & 0x3FF);

                if (price_min != INT32_MAX) {
                    write_success = response->writef("%d", price_min);
                }
                else {
                    write_success = response->writef("null");
                }

                if (write_success) {
                    if (price_avg != INT32_MAX) {
                        write_success = response->writef(",%d", price_avg);
                    }
                    else {
                        write_success = response->writef(",null");
                    }
                }

                if (write_success) {
                    if (price_max != INT32_MAX) {
                        write_success = response->writef(",%d", price_max);
                    }
                    else {
                        write_success = response->writef(",null");
                    }
                }
            } else {
                write_success = response->write("null,null,null");
            }
        }
        else {
            if (data_chunk_data[i] != UINT32_MAX) {
                write_success = response->writef("%.2f", (double)data_chunk_data[i] / 100.0); // daWh -> kWh
            } else {
                write_success = response->write("null");
            }
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

        em_common.wem_register_sd_energy_manager_daily_data_points_low_level_callback(nullptr, nullptr);
        return;
    }

    if (!write_success) {
        response->end("write error");

        em_common.wem_register_sd_energy_manager_daily_data_points_low_level_callback(nullptr, nullptr);
        return;
    }
}

void EMEnergyAnalysis::history_energy_manager_daily_response(IChunkedResponse *response,
                                                             Ownership *response_ownership,
                                                             uint32_t response_owner_id)
{
    // date in local time to have the days properly aligned
    uint8_t year = history_energy_manager_daily.get("year")->asUint() - 2000;
    uint8_t month = history_energy_manager_daily.get("month")->asUint();

    uint32_t seqnum = history_request_seqnum++;
    uint8_t status;
    int rc = em_common.wem_get_sd_energy_manager_daily_data_points(year, month, 1, days_per_month(2000 + year, month), &status);

    //logger.printfln("history_energy_manager_daily_response: %d-%02d",
    //                2000 + year, month);

    if (rc != TF_E_OK || status != 0) {
        OwnershipGuard ownership_guard(response_ownership, response_owner_id);

        if (ownership_guard.have_ownership()) {
            response->begin(false);

            if (rc != TF_E_OK) {
                response->writef("Failed to get energy manager daily data point: seqnum %u, error %d", seqnum, rc);
                logger.printfln("Failed to get energy manager daily data point: seqnum %u, error %d", seqnum, rc);
            }
            else if (status != 0) {
                response->writef("Failed to get energy manager daily data point: seqnum %u, status (%s, %u)", seqnum, get_data_status_string(status), status);
                logger.printfln("Failed to get energy manager daily data point: seqnum %u, status (%s, %u)", seqnum, get_data_status_string(status), status);
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
        metadata->seqnum = seqnum;

        em_common.wem_register_sd_energy_manager_daily_data_points_low_level_callback(energy_manager_daily_data_points_handler, metadata);
    }
}
