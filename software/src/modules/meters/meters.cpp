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

#include "meters.h"
#include "meter_class_none.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"

#include "gcc_warnings.h"

static MeterGeneratorNone meter_generator_none;

static void init_uint32_array(uint32_t *arr, size_t len, uint32_t val)
{
    if (len <= 0)
        return;

    uint32_t *arr_end = arr + len;
    do {
        *arr++ = val;
    } while (arr < arr_end);
}

void Meters::pre_setup()
{
    for (MeterSlot &meter_slot : meter_slots) {
        meter_slot.value_ids = Config::Array({},
            get_config_uint_max_prototype(),
            0, UINT16_MAX - 1, Config::type_id<Config::ConfUint>()
        );
        meter_slot.values = Config::Array({},
            get_config_float_nan_prototype(),
            0, UINT16_MAX - 1, Config::type_id<Config::ConfFloat>()
        );

        meter_slot.values_last_updated_at = INT64_MIN;

        init_uint32_array(meter_slot.index_cache_single_values, INDEX_CACHE_SINGLE_VALUES_COUNT, UINT32_MAX);
        init_uint32_array(meter_slot.index_cache_currents,      INDEX_CACHE_CURRENT_COUNT,       UINT32_MAX);
    }

    generators.reserve(METER_CLASSES);
    register_meter_generator(METER_CLASS_NONE, &meter_generator_none);
}

void Meters::setup()
{
    generators.shrink_to_fit();

    // Create config prototypes, depending on available generators.
    uint8_t class_count = static_cast<uint8_t>(generators.size());
    ConfUnionPrototype *config_prototypes = new ConfUnionPrototype[class_count];

    for (uint32_t i = 0; i < class_count; i++) {
        const auto &generator_tuple = generators[i];
        uint8_t meter_class = static_cast<uint8_t>(std::get<0>(generator_tuple));
        auto meter_generator = std::get<1>(generator_tuple);
        config_prototypes[i] = {meter_class, *meter_generator->get_config_prototype()};
    }

    for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
        MeterSlot &meter_slot = meter_slots[slot];

        // Initialize config.
        meter_slot.config_union = Config::Union(
            *get_generator_for_class(METER_CLASS_NONE)->get_config_prototype(),
            METER_CLASS_NONE,
            config_prototypes,
            class_count
        );

        // Load config.
        api.restorePersistentConfig(get_path(slot, Meters::PathType::Config), &meter_slot.config_union);

        uint32_t configured_meter_class = meter_slot.config_union.getTag();

        // Generator might be a NONE class generator if the requested class is not available.
        MeterGenerator *generator = get_generator_for_class(configured_meter_class);

        // Initialize state to match (loaded) config.
        meter_slot.state = *generator->get_state_prototype();

        // Create meter from config.
        Config *meter_conf = static_cast<Config *>(meter_slot.config_union.get());
        Config *meter_state = &meter_slot.state;

        IMeter *meter = new_meter_of_class(configured_meter_class, slot, meter_state, meter_conf);
        if (!meter) {
            logger.printfln("meters: Failed to create meter of class %u.", configured_meter_class);
            continue;
        }
        if (configured_meter_class != METER_CLASS_NONE) {
            meter_slot.power_hist.setup();
        }
        meter->setup();
        meter_slot.meter = meter;
    }

    api.addFeature("meters");
    initialized = true;
}

void Meters::register_urls()
{
    for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
        MeterSlot &meter_slot = meter_slots[slot];

        api.addPersistentConfig(get_path(slot, Meters::PathType::Config), &meter_slot.config_union, {}, 1000);
        api.addState(get_path(slot, Meters::PathType::State),    &meter_slot.state,     {}, 1000);
        api.addState(get_path(slot, Meters::PathType::ValueIDs), &meter_slot.value_ids, {}, 1000);
        api.addState(get_path(slot, Meters::PathType::Values),   &meter_slot.values,    {}, 1000);

        const String base_path = get_path(slot, Meters::PathType::Base);

        if (meter_slot.meter->get_class() != METER_CLASS_NONE) {
            meter_slot.power_hist.register_urls(base_path);
        }
        if (meter_slot.meter) {
            meter_slot.meter->register_urls(base_path);
        }
    }
}

void Meters::register_meter_generator(uint32_t meter_class, MeterGenerator *generator)
{
    for (const auto &generator_tuple : generators) {
        uint32_t known_class = std::get<0>(generator_tuple);
        if (meter_class == known_class) {
            logger.printfln("meters: Tried to register meter generator for already registered meter class %u.", meter_class);
            return;
        }
    }

    generators.push_back({meter_class, generator});
}

MeterGenerator *Meters::get_generator_for_class(uint32_t meter_class)
{
    for (auto generator_tuple : generators) {
        uint32_t known_class = std::get<0>(generator_tuple);
        if (meter_class == known_class) {
            return std::get<1>(generator_tuple);
        }
    }

    if (meter_class == METER_CLASS_NONE) {
        logger.printfln("meters: No generator for dummy meter available. This is probably fatal.");
        return nullptr;
    }

    logger.printfln("meters: No generator for meter class %u.", meter_class);
    return get_generator_for_class(METER_CLASS_NONE);
}

IMeter *Meters::new_meter_of_class(uint32_t meter_class, uint32_t slot, Config *state, Config *config)
{
    MeterGenerator *generator = get_generator_for_class(meter_class);

    if (!generator)
        return nullptr;

    return generator->new_meter(slot, state, config);
}

IMeter *Meters::get_meter(uint32_t slot)
{
    if (slot >= METERS_SLOTS)
        return nullptr;

    return meter_slots[slot].meter;
}

uint32_t Meters::get_meters(uint32_t meter_class, IMeter **found_meters, uint32_t found_meters_capacity)
{
    uint32_t found_count = 0;
    for (uint32_t i = 0; i < METERS_SLOTS; i++) {
        IMeter *meter = meter_slots[i].meter;
        if (meter->get_class() == meter_class) {
            if (found_count < found_meters_capacity) {
                found_meters[found_count] = meter;
            }
            found_count++;
        }
    }
    return found_count;
}

uint32_t Meters::get_meter_class(uint32_t slot)
{
    if (slot >= METERS_SLOTS)
        return METER_CLASS_NONE;

    return meter_slots[slot].meter->get_class();
}

Meters::ValueAvailability Meters::get_single_value(uint32_t slot, uint32_t kind, float *value_out, micros_t max_age)
{
    if (slot >= METERS_SLOTS) {
        return Meters::ValueAvailability::Unavailable;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    uint32_t cached_index = meter_slot.index_cache_single_values[kind];

    if (cached_index == UINT32_MAX) {
        *value_out = NAN;

        // Meter declared its values but index isn't cached -> value is unavailable.
        if (meter_slot.values_declared)
            return Meters::ValueAvailability::Unavailable;

        // Meter hasn't declared its values yet, ask the configured meter.
        bool supported;
        switch (kind) {
            case INDEX_CACHE_POWER:         supported = meter_slot.meter->supports_power();  break;
            case INDEX_CACHE_ENERGY_IMPORT: supported = meter_slot.meter->supports_energy_import(); break;
            case INDEX_CACHE_ENERGY_EXPORT: supported = meter_slot.meter->supports_energy_export(); break;
            default: supported = false;
        }
        if (supported) {
            return Meters::ValueAvailability::CurrentlyUnknown;
        } else {
            return Meters::ValueAvailability::Unavailable;
        }
    }

    Config *val = static_cast<Config *>(meter_slot.values.get(static_cast<uint16_t>(cached_index)));
    assert(val); // If an index is cached, it must be in values.

    *value_out = val->asFloat();

    if (max_age != 0_usec && deadline_elapsed(meter_slot.values_last_updated_at + max_age)) {
        return Meters::ValueAvailability::Stale;
    } else {
        return Meters::ValueAvailability::Fresh;
    }
}

Meters::ValueAvailability Meters::get_power(uint32_t slot, float *power, micros_t max_age)
{
    return get_single_value(slot, INDEX_CACHE_POWER, power, max_age);
}

Meters::ValueAvailability Meters::get_energy_import(uint32_t slot, float *total_import_kwh, micros_t max_age)
{
    return get_single_value(slot, INDEX_CACHE_ENERGY_IMPORT, total_import_kwh, max_age);
}

Meters::ValueAvailability Meters::get_energy_export(uint32_t slot, float *total_export_kwh, micros_t max_age)
{
    return get_single_value(slot, INDEX_CACHE_ENERGY_EXPORT, total_export_kwh, max_age);
}
/*
uint32_t Meters::get_currents(uint32_t slot, float currents[INDEX_CACHE_CURRENT_COUNT], micros_t max_age)
{
    if (slot >= METERS_SLOTS)
        return 0;

    MeterSlot &meter_slot = meter_slots[slot];

    uint32_t found_N_values = 0;
    uint32_t found_L_values = 0;
    for (uint32_t i = 0; i < INDEX_CACHE_CURRENT_COUNT; i++) {
        uint32_t current_index = meter_slot.index_cache_currents[i];
        Config *val;

        if (current_index == UINT32_MAX) {
            val = nullptr;
        } else {
            val = static_cast<Config *>(meter_slot.values.get(static_cast<uint16_t>(current_index)));
        }

        if (val) {
            currents[i] = val->asFloat();
            if (i == INDEX_CACHE_CURRENT_N) {
                found_N_values++;
            } else {
                found_L_values++;
            }
        } else {
            currents[i] = NAN;
        }
    }

    if (max_age != 0_usec && deadline_elapsed(meter_slot.values_last_updated_at + max_age)) {
        return 0;
    }

    if (found_L_values == 3) {
        if (found_N_values == 1) {
            return 4;
        } else {
            return 3;
        }
    } else {
        if (found_N_values == 1) {
            return 1;
        } else {
            return 0;
        }
    }
}
*/
void Meters::update_value(uint32_t slot, uint32_t index, float new_value)
{
    if (slot >= METERS_SLOTS) {
        logger.printfln("meters: Tried to update value %u for meter in non-existent slot %u.", index, slot);
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    meter_slot.values.get(static_cast<uint16_t>(index))->updateFloat(new_value);
    meter_slot.values_last_updated_at = now_us();

    if (!isnan(new_value) && index == meter_slot.index_cache_single_values[INDEX_CACHE_POWER]) {
        meter_slot.power_hist.add_sample(new_value);
    }
}

void Meters::update_all_values(uint32_t slot, const float new_values[])
{
    if (slot >= METERS_SLOTS) {
        logger.printfln("meters: Tried to update all values from array for meter in non-existent slot %u.", slot);
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    Config &values = meter_slot.values;
    auto value_count = values.count();
    bool updated_any_value = false;

    for (uint16_t i = 0; i < value_count; i++) {
        if (!isnan(new_values[i])) {
            //auto wrap = values.get(i);
            //auto old_value = wrap->asFloat();
            //bool changed = wrap->updateFloat(new_values[i]) && !isnan(old_value);
            //(void)changed;
            values.get(i)->updateFloat(new_values[i]);
            updated_any_value = true;
        }
    }

    if (updated_any_value) {
        meter_slot.values_last_updated_at = now_us();

        float power;
        if (get_power(slot, &power) == ValueAvailability::Fresh) {
            meter_slot.power_hist.add_sample(power);
        }
    }
}

void Meters::update_all_values(uint32_t slot, Config *new_values)
{
    if (slot >= METERS_SLOTS) {
        logger.printfln("meters: Tried to update all values from Config for meter in non-existent slot %u.", slot);
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    ConfigRoot &values = meter_slot.values;
    auto value_count = values.count();
    bool updated_any_value = false;

    if (new_values->count() != value_count) {
        logger.printfln("meters: Update all values element count mismatch: %i != %i", new_values->count(), value_count);
        return;
    }

    for (uint16_t i = 0; i < value_count; i++) {
        float val = new_values->get(i)->asFloat();
        if (!isnan(val)) {
            values.get(i)->updateFloat(val);
            updated_any_value = true;
        }
    }

    if (updated_any_value) {
        meter_slot.values_last_updated_at = now_us();

        float power;
        if (get_power(slot, &power) == ValueAvailability::Fresh) {
            meter_slot.power_hist.add_sample(power);
        }
    }
}

void Meters::declare_value_ids(uint32_t slot, const MeterValueID new_value_ids[], uint32_t value_id_count)
{
    if (slot >= METERS_SLOTS) {
        logger.printfln("meters: Tried to declare value IDs for meter in non-existent slot %u.", slot);
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    Config &value_ids = meter_slot.value_ids;
    Config &values    = meter_slot.values;

    if (value_ids.count() != 0) {
        logger.printfln("meters: Meter in slot %u already declared %i values. Refusing to re-declare %u values.", slot, value_ids.count(), value_id_count);
        return;
    }

    for (uint16_t i = 0; i < static_cast<uint16_t>(value_id_count); i++) {
        auto val = value_ids.add();
        val->updateUint(static_cast<uint32_t>(new_value_ids[i]));

        values.add();
    }

    meter_slot.index_cache_single_values[INDEX_CACHE_POWER]         = meters_find_id_index(new_value_ids, value_id_count, MeterValueID::PowerActiveLSumImExDiff);
    meter_slot.index_cache_single_values[INDEX_CACHE_ENERGY_IMPORT] = meters_find_id_index(new_value_ids, value_id_count, MeterValueID::EnergyActiveLSumImport);
    meter_slot.index_cache_single_values[INDEX_CACHE_ENERGY_EXPORT] = meters_find_id_index(new_value_ids, value_id_count, MeterValueID::EnergyActiveLSumExport);
    meter_slot.index_cache_currents[INDEX_CACHE_CURRENT_N  ]        = meters_find_id_index(new_value_ids, value_id_count, MeterValueID::CurrentNImport);
    meter_slot.index_cache_currents[INDEX_CACHE_CURRENT_L1 ]        = meters_find_id_index(new_value_ids, value_id_count, MeterValueID::CurrentL1Import);
    meter_slot.index_cache_currents[INDEX_CACHE_CURRENT_L2 ]        = meters_find_id_index(new_value_ids, value_id_count, MeterValueID::CurrentL2Import);
    meter_slot.index_cache_currents[INDEX_CACHE_CURRENT_L3 ]        = meters_find_id_index(new_value_ids, value_id_count, MeterValueID::CurrentL3Import);

    meter_slot.values_declared = true;
    logger.printfln("meters: Meter in slot %u declared %u values.", slot, value_id_count);
}

bool Meters::get_cached_power_index(uint32_t slot, uint32_t *index)
{
    *index = meter_slots[slot].index_cache_single_values[INDEX_CACHE_POWER];
    return *index != UINT32_MAX;
}

static const char *meters_path_postfixes[] = {"", "config", "state", "value_ids", "values"};
static_assert(sizeof(ARRAY_SIZE(meters_path_postfixes)) == static_cast<uint32_t>(Meters::PathType::_max), "Path postfix length mismatch");

String Meters::get_path(uint32_t slot, Meters::PathType path_type)
{
    String path = "meters/_";
    path.concat(slot);
    path.concat('_');
    path.concat(meters_path_postfixes[static_cast<uint32_t>(path_type)]);

    return path;
}

_ATTRIBUTE((const))
const Config * Meters::get_config_bool_false_prototype() const
{
    return &config_bool_false_prototype;
}

_ATTRIBUTE((const))
const Config * Meters::get_config_float_nan_prototype()
{
    if (config_float_nan_prototype.is_null()) {
        config_float_nan_prototype = Config::Float(NAN);
    }
    return &config_float_nan_prototype;
}

_ATTRIBUTE((const))
const Config * Meters::get_config_uint_max_prototype()
{
    if (config_uint_max_prototype.is_null()) {
        config_uint_max_prototype  = Config::Uint32(UINT32_MAX);
    }
    return &config_uint_max_prototype;
}

uint32_t meters_find_id_index(const MeterValueID value_ids[], uint32_t value_id_count, MeterValueID id)
{
    for (uint32_t i = 0; i < value_id_count; i++) {
        if (value_ids[i] == id)
            return i;
    }
    return UINT32_MAX;
}
