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

#include "imeter.h"
#include "meter_class_defs.h"
#include "meter_generator.h"

#include <stdint.h>

#include "config.h"
#include "module.h"
#include "value_history.h"
#include "meter_value_id.h"
#include "tools.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

#define INDEX_CACHE_POWER         0
#define INDEX_CACHE_ENERGY_IMPORT 1
#define INDEX_CACHE_ENERGY_EXPORT 2
#define INDEX_CACHE_SINGLE_VALUES_COUNT 3

#define INDEX_CACHE_CURRENT_N  0
#define INDEX_CACHE_CURRENT_L1 1
#define INDEX_CACHE_CURRENT_L2 2
#define INDEX_CACHE_CURRENT_L3 3
#define INDEX_CACHE_CURRENT_COUNT 4

class Meters final : public IModule
{
public:
    enum class ValueAvailability {
        Fresh,              // Meter declared requested value ID and value is fresh.
        Stale,              // Meter declared requested value ID and value is stale or not yet set.
        Unavailable,        // (a) Meter declared its value IDs but requisted value ID was not among them. (b) Meter hasn't declared its values and meter config can't provide this value ID.
        CurrentlyUnknown,   // Meter hasn't declared its value IDs yet and the meter config doesn't know.
    };

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

    Meters(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void register_meter_generator(uint32_t meter_class, MeterGenerator *generator);
    IMeter *get_meter(uint32_t slot);
    uint32_t get_meters(uint32_t meter_class, IMeter **found_meters, uint32_t found_meters_capacity);
    uint32_t get_meter_class(uint32_t slot);

    ValueAvailability get_power(uint32_t slot, float *power_w, micros_t max_age = 0_usec);
    ValueAvailability get_energy_import(uint32_t slot, float *total_import_kwh, micros_t max_age = 0_usec);
    ValueAvailability get_energy_export(uint32_t slot, float *total_export_kwh, micros_t max_age = 0_usec);
    //uint32_t get_currents(uint32_t slot, float currents[INDEX_CACHE_CURRENT_COUNT], micros_t max_age = 0_usec);

    void update_value(uint32_t slot, uint32_t index, float new_value);
    void update_all_values(uint32_t slot, const float new_values[]);
    void update_all_values(uint32_t slot, Config *new_values);
    void declare_value_ids(uint32_t slot, const MeterValueID value_ids[], uint32_t value_id_count);

    bool get_cached_power_index(uint32_t slot, uint32_t *index);

    String get_path(uint32_t slot, PathType path_type);

    const Config * get_config_bool_false_prototype() const _ATTRIBUTE((const));
    const Config * get_config_float_nan_prototype() _ATTRIBUTE((const));
    const Config * get_config_uint_max_prototype() _ATTRIBUTE((const));

private:
    class MeterSlot final
    {
    public:
        ConfigRoot value_ids;
        ConfigRoot values;

        micros_t values_last_updated_at;
        bool     values_declared;

        IMeter *meter;

        // Caches must be initialized to UINT32_MAX in setup().
        uint32_t index_cache_single_values[INDEX_CACHE_SINGLE_VALUES_COUNT];
        uint32_t index_cache_currents[INDEX_CACHE_CURRENT_COUNT];

        ConfigRoot config_union;
        ConfigRoot state;
        ConfigRoot errors;

        ValueHistory power_hist;
    };

    MeterGenerator *get_generator_for_class(uint32_t meter_class);
    IMeter *new_meter_of_class(uint32_t meter_class, uint32_t slot, Config *state, Config *config, Config *errors);

    ValueAvailability get_single_value(uint32_t slot, uint32_t kind, float *value, micros_t max_age_us);

    MeterSlot meter_slots[METERS_SLOTS];

    Config config_bool_false_prototype = Config::Bool(false);
    Config config_float_nan_prototype;
    Config config_uint_max_prototype;

    std::vector<std::tuple<uint32_t, MeterGenerator *>> generators;
};

extern uint32_t meters_find_id_index(const MeterValueID value_ids[], uint32_t value_id_count, MeterValueID id);

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
