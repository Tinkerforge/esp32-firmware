/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <stdint.h>

#include "module.h"
#include "config.h"
#include "imeter.h"
#include "meter_generator.h"
#include "meter_value_availability.h"
#include "value_history.h"
#include "meter_value_id.h"
#include "tools.h"

// string_length_visitor assumes that a float is max. 20 byte long
// We need n+1 bytes extra for n meter values for '[', ',' and ']'
// The MQTT send buffer is 2K on a WARP1 -> 2048/21 ~ 97,5.
#define METERS_MAX_VALUES_PER_METER 96

#define METERS_MAX_FILTER_VALUES 7

#define INDEX_CACHE_POWER_REAL     0
#define INDEX_CACHE_POWER_VIRTUAL  1
#define INDEX_CACHE_ENERGY_IMPORT  2
#define INDEX_CACHE_ENERGY_IMEXSUM 3
#define INDEX_CACHE_ENERGY_EXPORT  4
#define INDEX_CACHE_SINGLE_VALUES_COUNT 5

#define INDEX_CACHE_CURRENT_L1 0
#define INDEX_CACHE_CURRENT_L2 1
#define INDEX_CACHE_CURRENT_L3 2
#define INDEX_CACHE_CURRENT_COUNT 3

class Meters final : public IModule
{
public:
    enum class PathType {
        Base        = 0,
        Config      = 1,
        State       = 2,
        ValueIDs    = 3,
        Values      = 4,
        Errors      = 5,
        Reset       = 6,
        LastReset   = 7,
        _max        = 7,
    };

    struct value_combiner_filter_data {
        uint8_t input_pos[METERS_MAX_FILTER_VALUES];
        uint8_t output_pos;
    };

    struct value_combiner_filter {
        void (*fn)(const value_combiner_filter_data *filter_data, size_t base_values_length, const float *base_values, float *extra_values);
        const char *name;
        MeterValueID input_ids[METERS_MAX_FILTER_VALUES];
        MeterValueID output_ids[METERS_MAX_FILTER_VALUES];
    };
    static_assert(METERS_MAX_VALUES_PER_METER <= 256, "Increase size of input_id_count and output_id_count");

    Meters(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void pre_reboot() override;

    void register_meter_generator(MeterClassID meter_class, MeterGenerator *generator);
    IMeter *get_meter(uint32_t slot);
    uint32_t get_meters(MeterClassID meter_class, IMeter **found_meters, uint32_t found_meters_capacity);
    MeterClassID get_meter_class(uint32_t slot);
    bool meter_is_fresh(uint32_t slot, micros_t max_age_us);
    bool meter_has_value_changed(uint32_t slot, micros_t max_age_us);

    MeterValueAvailability get_values(uint32_t slot, const Config **values, micros_t max_age = 0_us);
    MeterValueAvailability get_value_by_index(uint32_t slot, uint32_t index, float *value, micros_t max_age = 0_us);
    MeterValueAvailability get_power_real(uint32_t slot, float *power_w, micros_t max_age = 0_us);
    MeterValueAvailability get_power_virtual(uint32_t slot, float *power_w, micros_t max_age = 0_us);
    MeterValueAvailability get_energy_import(uint32_t slot, float *total_import_kwh, micros_t max_age = 0_us);
    MeterValueAvailability get_energy_imexsum(uint32_t slot, float *total_imexsum_kwh, micros_t max_age = 0_us);
    MeterValueAvailability get_energy_export(uint32_t slot, float *total_export_kwh, micros_t max_age = 0_us);
    MeterValueAvailability get_currents(uint32_t slot, float currents[INDEX_CACHE_CURRENT_COUNT], micros_t max_age = 0_us);

    void update_value(uint32_t slot, uint32_t index, float new_value);
    void update_all_values(uint32_t slot, const float new_values[]);
    void update_all_values(uint32_t slot, const Config *new_values);
    void finish_update(uint32_t slot);
    void declare_value_ids(uint32_t slot, const MeterValueID value_ids[], uint32_t value_id_count);

    bool get_cached_real_power_index(uint32_t slot, uint32_t *index);

    void fill_index_cache(uint32_t slot, size_t value_count, const MeterValueID value_ids[], uint32_t index_cache[]);

    String get_path(uint32_t slot, PathType path_type);

    [[gnu::const]] const Config *get_config_bool_false_prototype() const;
    [[gnu::const]] const Config *get_config_float_nan_prototype();
    [[gnu::const]] const Config *get_config_uint_max_prototype();

private:
    class MeterSlot final
    {
    public:
        ConfigRoot value_ids;
        ConfigRoot values;

        micros_t values_last_updated_at;
        micros_t values_last_changed_at;
        bool     values_declared;

        IMeter *meter;

        size_t base_value_count;
        uint32_t value_combiner_filters_bitmask;
        const value_combiner_filter_data *value_combiner_filters_data;

        // Caches must be initialized to UINT32_MAX in setup().
        uint32_t index_cache_single_values[INDEX_CACHE_SINGLE_VALUES_COUNT];
        uint32_t index_cache_currents[INDEX_CACHE_CURRENT_COUNT];

        ConfigRoot config_union;
        ConfigRoot state;
        ConfigRoot errors;
        ConfigRoot last_reset;

        ValueHistory power_history;
    };

    MeterGenerator *get_generator_for_class(MeterClassID meter_class);
    IMeter *new_meter_of_class(MeterClassID meter_class, uint32_t slot, Config *state, Config *errors);

    MeterValueAvailability get_single_value(uint32_t slot, uint32_t kind, float *value, micros_t max_age_us);
    void apply_filters(MeterSlot &meter_slot, size_t base_value_count, const float *base_values);

    float live_samples_per_second();

    MeterSlot meter_slots[METERS_SLOTS];

    bool meters_feature_declared = false;

    Config config_bool_false_prototype = Config::Bool(false);
    Config config_float_nan_prototype;
    Config config_uint_max_prototype;
    ConfigRoot last_reset_prototype;

    std::vector<std::tuple<MeterClassID, MeterGenerator *>> generators;

    size_t history_chars_per_value;
    uint32_t last_live_update = 0;
    uint32_t last_history_update = 0;
    uint32_t last_history_slot = UINT32_MAX;

    int samples_this_interval = 0;
    uint32_t begin_this_interval = 0;
    uint32_t end_this_interval = 0;

    int samples_last_interval = 0;
    uint32_t begin_last_interval = 0;
    uint32_t end_last_interval = 0;

    // For reproducable screenshots, the ScreenshotDataFaker injects values into the power_history of the first meter.
#ifdef SCREENSHOT_DATA_FAKER_PRO
    friend class ScreenshotDataFaker;
#endif
};

extern uint32_t meters_find_id_index(const MeterValueID value_ids[], uint32_t value_id_count, MeterValueID id);
