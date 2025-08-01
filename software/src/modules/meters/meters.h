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
#include "options.h"
#include "imeter.h"
#include "imeter_generator.h"
#include "meter_location.enum.h"
#include "meter_value_availability.h"
#include "value_history.h"
#include "meter_value_id.h"
#include "tools.h"

#define METERS_MAX_FILTER_VALUES 7

#define INDEX_CACHE_POWER          0
#define INDEX_CACHE_ENERGY_IMPORT  1
#define INDEX_CACHE_ENERGY_IMEXSUM 2
#define INDEX_CACHE_ENERGY_EXPORT  3
#define INDEX_CACHE_SOC            4
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
    static_assert(OPTIONS_METERS_MAX_VALUES_PER_METER() <= 256, "Increase size of input_id_count and output_id_count");

    enum class ExtraValueDirection : uint8_t {
        Positive,
        Negative,
    };

    struct extra_value_id {
        uint16_t value_id;
        uint8_t source_index;
        ExtraValueDirection direction;
    };

    Meters(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;
    void pre_reboot() override;

    void register_meter_generator(MeterClassID meter_class, IMeterGenerator *generator);
    IMeter *get_meter(uint32_t slot);
    uint32_t get_meters(MeterClassID meter_class, IMeter **found_meters, uint32_t found_meters_capacity);
    MeterClassID get_meter_class(uint32_t slot);
    MeterLocation get_meter_location(uint32_t slot);
    bool meter_is_fresh(uint32_t slot, micros_t max_age_us);
    bool meter_has_value_changed(uint32_t slot, micros_t max_age_us);

    MeterValueAvailability get_value_ids(uint32_t slot, const Config **value_ids);
    MeterValueAvailability get_value_ids_extended(uint32_t slot, MeterValueID *value_ids_out, size_t *value_ids_length);
    MeterValueAvailability get_values(uint32_t slot, const Config **values, micros_t max_age = 0_us);
    MeterValueAvailability get_values_with_cache(uint32_t slot, float *values, const uint32_t *index_cache, size_t value_count, micros_t max_age = 0_us);
    MeterValueAvailability get_value_by_index(uint32_t slot, uint32_t index, float *value, micros_t max_age = 0_us);
    MeterValueAvailability get_power(uint32_t slot, float *power_w, micros_t max_age = 0_us);
    MeterValueAvailability get_energy_import(uint32_t slot, float *total_import_kwh, micros_t max_age = 0_us);
    MeterValueAvailability get_energy_imexsum(uint32_t slot, float *total_imexsum_kwh, micros_t max_age = 0_us);
    MeterValueAvailability get_energy_export(uint32_t slot, float *total_export_kwh, micros_t max_age = 0_us);
    MeterValueAvailability get_soc(uint32_t slot, float *soc, micros_t max_age = 0_us);
    MeterValueAvailability get_currents(uint32_t slot, float currents[INDEX_CACHE_CURRENT_COUNT], micros_t max_age = 0_us);

    void update_value(uint32_t slot, uint32_t index, float new_value);
    void update_all_values(uint32_t slot, const float new_values[]);
    void update_all_values(uint32_t slot, const Config *new_values);
    void finish_update(uint32_t slot);
    void declare_value_ids(uint32_t slot, const MeterValueID value_ids[], uint32_t value_id_count);

    bool get_cached_power_index(uint32_t slot, uint32_t *index);

    void fill_index_cache(uint32_t slot, size_t value_count, const MeterValueID value_ids[], uint32_t index_cache[]);

    String get_path(uint32_t slot, PathType path_type);

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

        extra_value_id *extra_value_ids;
        size_t extra_value_id_count;

        // Caches must be initialized to UINT32_MAX in setup().
        uint32_t index_cache_single_values[INDEX_CACHE_SINGLE_VALUES_COUNT];
        uint32_t index_cache_currents[INDEX_CACHE_CURRENT_COUNT];

        ConfigRoot config_union;
        ConfigRoot state;
        ConfigRoot errors;
        ConfigRoot last_reset;

        ValueHistory power_history;
    };

    IMeterGenerator *get_generator_for_class(MeterClassID meter_class);
    IMeter *new_meter_of_class(MeterClassID meter_class, uint32_t slot, Config *state, Config *errors);

    MeterValueAvailability get_single_value(uint32_t slot, uint32_t kind, float *value, micros_t max_age_us);
    void apply_filters(MeterSlot &meter_slot, size_t base_value_count, const float *base_values);

    float live_samples_per_second();

    MeterSlot meter_slots[OPTIONS_METERS_MAX_SLOTS()];

    bool meters_feature_declared = false;

    std::vector<std::tuple<MeterClassID, IMeterGenerator *>> generators;

    size_t history_chars_per_value;
    micros_t last_live_update = 0_us;
    micros_t last_history_update = 0_us;
    uint32_t last_history_slot = UINT32_MAX;

    int samples_this_interval = 0;
    micros_t begin_this_interval = 0_us;
    micros_t end_this_interval = 0_us;

    int samples_last_interval = 0;
    micros_t begin_last_interval = 0_us;
    micros_t end_last_interval = 0_us;

    // For reproducable screenshots, the ScreenshotDataFaker injects values into the power_history of the first meter.
#ifdef OPTIONS_SCREENSHOT_DATA_FAKER_PRO
#if OPTIONS_SCREENSHOT_DATA_FAKER_PRO()
    friend class ScreenshotDataFaker;
#endif
#endif
};

extern uint32_t meters_find_id_index(const MeterValueID value_ids[], uint32_t value_id_count, std::initializer_list<MeterValueID> ids);

inline uint32_t meters_find_id_index(const MeterValueID value_ids[], uint32_t value_id_count, MeterValueID id)
{
    return meters_find_id_index(value_ids, value_id_count, {id});
}
