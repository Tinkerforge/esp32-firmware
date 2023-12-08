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
#include "module_dependencies.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"

#include "gcc_warnings.h"
#ifdef __GNUC__
// The code is this file contains several casts to a type defined by a macro, which may result in useless casts.
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

// string_length_visitor assumes that a float is max. 20 byte long
// We need n+1 bytes extra for n meter values for '[', ',' and ']'
// The MQTT send buffer is 2K on a WARP1 -> 2048/21 ~ 97,5.
#define METERS_MAX_VALUES_PER_METER 96

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
            0, METERS_MAX_VALUES_PER_METER, Config::type_id<Config::ConfUint>()
        );
        meter_slot.values = Config::Array({},
            get_config_float_nan_prototype(),
            0, METERS_MAX_VALUES_PER_METER, Config::type_id<Config::ConfFloat>()
        );

        meter_slot.values_last_updated_at = INT64_MIN;
        meter_slot.values_last_changed_at = INT64_MIN;

        init_uint32_array(meter_slot.index_cache_single_values, INDEX_CACHE_SINGLE_VALUES_COUNT, UINT32_MAX);
        init_uint32_array(meter_slot.index_cache_currents,      INDEX_CACHE_CURRENT_COUNT,       UINT32_MAX);
    }

    generators.reserve(METER_CLASSES);
    register_meter_generator(MeterClassID::None, &meter_generator_none);

    last_reset_prototype = Config::Object({
        {"last_reset", Config::Uint32(0)}
    });


#if MODULE_CRON_AVAILABLE()
    cron.register_action(
        CronActionID::MeterReset,
        Config::Object({
            {"meter_slot", Config::Uint(0, 0, METERS_SLOTS - 1)}
        }),
        [this](const Config *config) {
            api.callCommand(get_path(config->get("meter_slot")->asUint(), Meters::PathType::Reset).c_str(), {});
        }
    );
#endif
}

void Meters::setup()
{
    generators.shrink_to_fit();

    // Create config prototypes, depending on available generators.
    uint8_t class_count = static_cast<uint8_t>(generators.size());
    ConfUnionPrototype<MeterClassID> *config_prototypes = new ConfUnionPrototype<MeterClassID>[class_count];

    for (uint32_t i = 0; i < class_count; i++) {
        const auto &generator_tuple = generators[i];
        MeterClassID meter_class = std::get<0>(generator_tuple);
        auto meter_generator = std::get<1>(generator_tuple);
        config_prototypes[i] = {meter_class, *meter_generator->get_config_prototype()};
    }

    for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
        MeterSlot &meter_slot = meter_slots[slot];
#ifdef METERS_SLOT_0_DEFAULT_CLASS
        MeterClassID meter_class = slot == 0 ? METERS_SLOT_0_DEFAULT_CLASS : MeterClassID::None;
#else
        MeterClassID meter_class = MeterClassID::None;
#endif

        // Initialize config.
        meter_slot.config_union = Config::Union(
            *get_generator_for_class(meter_class)->get_config_prototype(),
            meter_class,
            config_prototypes,
            class_count
        );

        // Load config.
        api.restorePersistentConfig(get_path(slot, Meters::PathType::Config), &meter_slot.config_union);

        MeterClassID configured_meter_class = meter_slot.config_union.getTag<MeterClassID>();

        // Generator might be a NONE class generator if the requested class is not available.
        MeterGenerator *generator = get_generator_for_class(configured_meter_class);

        // Initialize state and errors to match (loaded) config.
        meter_slot.state  = *generator->get_state_prototype();
        meter_slot.errors = *generator->get_errors_prototype();

        // Create meter from config.
        Config *meter_state = &meter_slot.state;
        Config *meter_errors = &meter_slot.errors;

        IMeter *meter = new_meter_of_class(configured_meter_class, slot, meter_state, meter_errors);
        if (!meter) {
            logger.printfln("meters: Failed to create meter of class %u in slot %u.", static_cast<uint32_t>(configured_meter_class), slot);
            meter = new_meter_of_class(MeterClassID::None, slot, meter_state, meter_errors);
        }
        if (configured_meter_class != MeterClassID::None) {
            meter_slot.power_history.setup();
        }

        meter->setup(*static_cast<Config *>(meter_slot.config_union.get()));
        // Setup before calling supports_reset to allow a meter to decide in
        // setup whether to support reset. This could for example depend on the
        // meter's configuration.
        logger.printfln("meter supports reset: %d", meter->supports_reset());
        if (meter->supports_reset()) {
            meter_slot.reset = *Config::Null();
            meter_slot.last_reset = last_reset_prototype;
            api.restorePersistentConfig(get_path(slot, Meters::PathType::LastReset), &meter_slot.last_reset);
        }
        meter_slot.meter = meter;
    }

    history_chars_per_value = max(String(METER_VALUE_HISTORY_VALUE_MIN).length(), String(METER_VALUE_HISTORY_VALUE_MAX).length());
    // val_min values are replaced with null -> require at least 4 chars per value.
    history_chars_per_value = max(4U, history_chars_per_value);
    // For ',' between the values.
    ++history_chars_per_value;

    task_scheduler.scheduleWithFixedDelay([this](){
        uint32_t now = millis();
        uint32_t current_history_slot = now / (HISTORY_MINUTE_INTERVAL * 60 * 1000);
        bool update_history = current_history_slot != last_history_slot;
        METER_VALUE_HISTORY_VALUE_TYPE live_samples[METERS_SLOTS];
        METER_VALUE_HISTORY_VALUE_TYPE history_samples[METERS_SLOTS];
        bool valid_samples[METERS_SLOTS];
        METER_VALUE_HISTORY_VALUE_TYPE val_min = std::numeric_limits<METER_VALUE_HISTORY_VALUE_TYPE>::lowest();

        for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
            MeterSlot &meter_slot = this->meter_slots[slot];

            if (meter_slot.meter->get_class() != MeterClassID::None) {
                meter_slot.power_history.tick(now, update_history, &live_samples[slot], &history_samples[slot]);
                valid_samples[slot] = true;
            }
            else {
                valid_samples[slot] = false;
            }
        }

        last_live_update = now;
        end_this_interval = last_live_update;

        if (samples_this_interval == 0) {
            begin_this_interval = last_live_update;
        }

        ++samples_this_interval;

#if MODULE_WS_AVAILABLE()
        {
            const size_t buf_size = METERS_SLOTS * history_chars_per_value + 100;
            char *buf_ptr = static_cast<char *>(malloc(sizeof(char) * buf_size));
            size_t buf_written = 0;

            buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "{\"topic\":\"meters/live_samples\",\"payload\":{\"samples_per_second\":%f,\"samples\":[", static_cast<double>(live_samples_per_second()));

            if (buf_written < buf_size) {
                for (uint32_t slot = 0; slot < METERS_SLOTS && buf_written < buf_size; slot++) {
                    if (!valid_samples[slot]) {
                        buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, slot == 0 ? "[%s]" : ",[%s]", "");
                    }
                    else if (live_samples[slot] == val_min) {
                        buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, slot == 0 ? "[%s]" : ",[%s]", "null");
                    }
                    else {
                        buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, slot == 0 ? "[%d]" : ",[%d]", static_cast<int>(live_samples[slot]));
                    }
                }

                if (buf_written < buf_size) {
                    buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "%s", "]}}\n");
                }
            }

            if (buf_written > 0) {
                ws.web_sockets.sendToAllOwned(buf_ptr, static_cast<size_t>(buf_written));
            }
        }
#endif

        if (update_history) {
            last_history_update = now;
            samples_last_interval = samples_this_interval;
            begin_last_interval = begin_this_interval;
            end_last_interval = end_this_interval;

            samples_this_interval = 0;
            begin_this_interval = 0;
            end_this_interval = 0;

#if MODULE_WS_AVAILABLE()
            {
                const size_t buf_size = METERS_SLOTS * history_chars_per_value + 100;
                char *buf_ptr = static_cast<char *>(malloc(sizeof(char) * buf_size));
                size_t buf_written = 0;

                buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "%s", "{\"topic\":\"meters/history_samples\",\"payload\":{\"samples\":[");

                if (buf_written < buf_size) {
                    for (uint32_t slot = 0; slot < METERS_SLOTS && buf_written < buf_size; slot++) {
                        if (!valid_samples[slot]) {
                            buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, slot == 0 ? "[%s]" : ",[%s]", "");
                        }
                        else if (history_samples[slot] == val_min) {
                            buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, slot == 0 ? "[%s]" : ",[%s]", "null");
                        }
                        else {
                            buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, slot == 0 ? "[%d]" : ",[%d]", static_cast<int>(history_samples[slot]));
                        }
                    }

                    if (buf_written < buf_size) {
                        buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "%s", "]}}\n");
                    }
                }

                if (buf_written > 0) {
                    ws.web_sockets.sendToAllOwned(buf_ptr, static_cast<size_t>(buf_written));
                }
            }
#endif
        }

        last_history_slot = current_history_slot;
    }, 0, 500);

    initialized = true;
}

void Meters::register_urls()
{
    for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
        MeterSlot &meter_slot = meter_slots[slot];

        api.addPersistentConfig(get_path(slot, Meters::PathType::Config), &meter_slot.config_union, {}, 1000);
        api.addState(get_path(slot, Meters::PathType::State),    &meter_slot.state,     {}, 1000);
        api.addState(get_path(slot, Meters::PathType::Errors),   &meter_slot.errors,    {}, 1000);
        api.addState(get_path(slot, Meters::PathType::ValueIDs), &meter_slot.value_ids, {}, 1000);
        api.addState(get_path(slot, Meters::PathType::Values),   &meter_slot.values,    {}, 1000);

        const String base_path = get_path(slot, Meters::PathType::Base);

        if (meter_slot.meter->get_class() != MeterClassID::None) {
            meter_slot.power_history.register_urls(base_path);
        } else {
            meter_slot.power_history.register_urls_empty(base_path);
        }

        if (meter_slot.meter->supports_reset()) {
            api.addCommand(get_path(slot, Meters::PathType::Reset), &meter_slot.reset, {}, [this, &meter_slot, slot]() mutable {
                if (!meter_slot.meter->reset())
                    return;

                struct timeval tv_now;

                if (clock_synced(&tv_now)) {
                    //FIXME not Y2038-safe
                    meter_slot.last_reset.get("last_reset")->updateUint(static_cast<uint32_t>(tv_now.tv_sec));
                } else {
                    meter_slot.last_reset.get("last_reset")->updateUint(0);
                }
                api.writeConfig(get_path(slot, Meters::PathType::LastReset), &meter_slot.last_reset);
            }, true);

            api.addState(get_path(slot, Meters::PathType::LastReset), &meter_slot.last_reset);
        }

        meter_slot.meter->register_urls(base_path);
    }

    server.on("/meters/history", HTTP_GET, [this](WebServerRequest request) {
        uint32_t now = millis();
        const size_t buf_size = HISTORY_RING_BUF_SIZE * history_chars_per_value + 100;
        std::unique_ptr<char[]> buf{new char[buf_size]};
        char *buf_ptr = buf.get();
        size_t buf_written = 0;
        uint32_t offset = now - last_history_update;

        buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "{\"offset\":%u,\"samples\":[", offset);

        request.beginChunkedResponse(200, "application/json; charset=utf-8");

        for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
            MeterSlot &meter_slot = meter_slots[slot];

            if (meter_slot.meter->get_class() != MeterClassID::None) {
                if (buf_written < buf_size) {
                    buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "%s", slot == 0 ? "[" : ",[");

                    if (buf_written < buf_size) {
                        buf_written += meter_slot.power_history.format_history_samples(buf_ptr + buf_written, buf_size - buf_written);

                        if (buf_written < buf_size) {
                            buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "%s", "]");
                        }
                    }
                }
            } else if (buf_written < buf_size) {
                buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "%s", slot == 0 ? "null" : ",null");
            }

            request.sendChunk(buf_ptr, static_cast<ssize_t>(buf_written));

            buf_written = 0;
        }

        request.sendChunk("]}", 2);

        return request.endChunkedResponse();
    });

    server.on("/meters/live", HTTP_GET, [this](WebServerRequest request) {
        uint32_t now = millis();
        const size_t buf_size = HISTORY_RING_BUF_SIZE * history_chars_per_value + 100;
        std::unique_ptr<char[]> buf{new char[buf_size]};
        char *buf_ptr = buf.get();
        size_t buf_written = 0;
        uint32_t offset = now - last_live_update;

        buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "{\"offset\":%u,\"samples_per_second\":%f,\"samples\":[", offset, static_cast<double>(live_samples_per_second()));

        request.beginChunkedResponse(200, "application/json; charset=utf-8");

        for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
            MeterSlot &meter_slot = meter_slots[slot];

            if (meter_slot.meter->get_class() != MeterClassID::None) {
                if (buf_written < buf_size) {
                    buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "%s", slot == 0 ? "[" : ",[");

                    if (buf_written < buf_size) {
                        buf_written += meter_slot.power_history.format_live_samples(buf_ptr + buf_written, buf_size - buf_written);

                        if (buf_written < buf_size) {
                            buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "%s", "]");
                        }
                    }
                }
            } else if (buf_written < buf_size) {
                buf_written += snprintf_u(buf_ptr + buf_written, buf_size - buf_written, "%s", slot == 0 ? "null" : ",null");
            }

            request.sendChunk(buf_ptr, static_cast<ssize_t>(buf_written));

            buf_written = 0;
        }

        request.sendChunk("]}", 2);

        return request.endChunkedResponse();
    });

#if MODULE_METERS_LEGACY_API_AVAILABLE()
    if (meters_legacy_api.get_linked_meter_slot() < METERS_SLOTS) {
        api.addState("meter/error_counters", &meter_slots[meters_legacy_api.get_linked_meter_slot()].errors, {}, 1000);
    }
#endif
}

void Meters::register_meter_generator(MeterClassID meter_class, MeterGenerator *generator)
{
    for (const auto &generator_tuple : generators) {
        MeterClassID known_class = std::get<0>(generator_tuple);
        if (meter_class == known_class) {
            logger.printfln("meters: Tried to register meter generator for already registered meter class %u.", static_cast<uint32_t>(meter_class));
            return;
        }
    }

    generators.push_back({meter_class, generator});
}

MeterGenerator *Meters::get_generator_for_class(MeterClassID meter_class)
{
    for (auto generator_tuple : generators) {
        MeterClassID known_class = std::get<0>(generator_tuple);
        if (meter_class == known_class) {
            return std::get<1>(generator_tuple);
        }
    }

    if (meter_class == MeterClassID::None) {
        logger.printfln("meters: No generator for dummy meter available. This is probably fatal.");
        return nullptr;
    }

    logger.printfln("meters: No generator for meter class %u.", static_cast<uint32_t>(meter_class));
    return get_generator_for_class(MeterClassID::None);
}

IMeter *Meters::new_meter_of_class(MeterClassID meter_class, uint32_t slot, Config *state,  Config *errors)
{
    MeterGenerator *generator = get_generator_for_class(meter_class);

    if (!generator)
        return nullptr;

    return generator->new_meter(slot, state, errors);
}

IMeter *Meters::get_meter(uint32_t slot)
{
    if (slot >= METERS_SLOTS)
        return nullptr;

    return meter_slots[slot].meter;
}

uint32_t Meters::get_meters(MeterClassID meter_class, IMeter **found_meters, uint32_t found_meters_capacity)
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

MeterClassID Meters::get_meter_class(uint32_t slot)
{
    if (slot >= METERS_SLOTS)
        return MeterClassID::None;

    return meter_slots[slot].meter->get_class();
}

bool Meters::meter_is_fresh(uint32_t slot, micros_t max_age_us)
{
    return !deadline_elapsed(meter_slots[slot].values_last_updated_at + max_age_us);
}

bool Meters::meter_has_value_changed(uint32_t slot, micros_t max_age_us)
{
    return !deadline_elapsed(meter_slots[slot].values_last_changed_at + max_age_us);
}

MeterValueAvailability Meters::get_value_by_index(uint32_t slot, uint32_t index, float *value_out, micros_t max_age)
{
    if (slot >= METERS_SLOTS || index == UINT32_MAX) {
        *value_out = NAN;
        return MeterValueAvailability::Unavailable;
    }

    const MeterSlot &meter_slot = meter_slots[slot];

    *value_out = meter_slot.values.get(static_cast<uint16_t>(index))->asFloat();

    if (max_age != 0_usec && deadline_elapsed(meter_slot.values_last_updated_at + max_age)) {
        return MeterValueAvailability::Stale;
    } else {
        return MeterValueAvailability::Fresh;
    }
}

MeterValueAvailability Meters::get_single_value(uint32_t slot, uint32_t kind, float *value_out, micros_t max_age)
{
    if (slot >= METERS_SLOTS) {
        return MeterValueAvailability::Unavailable;
    }

    const MeterSlot &meter_slot = meter_slots[slot];

    uint32_t cached_index = meter_slot.index_cache_single_values[kind];

    if (cached_index == UINT32_MAX) {
        *value_out = NAN;

        // Meter declared its values but index isn't cached -> value is unavailable.
        if (meter_slot.values_declared)
            return MeterValueAvailability::Unavailable;

        // Meter hasn't declared its values yet, ask the configured meter.
        bool supported;
        switch (kind) {
            case INDEX_CACHE_POWER:         supported = meter_slot.meter->supports_power();  break;
            case INDEX_CACHE_ENERGY_IMPORT: supported = meter_slot.meter->supports_energy_import(); break;
            case INDEX_CACHE_ENERGY_EXPORT: supported = meter_slot.meter->supports_energy_export(); break;
            default: supported = false;
        }
        if (supported) {
            return MeterValueAvailability::CurrentlyUnknown;
        } else {
            return MeterValueAvailability::Unavailable;
        }
    }

    *value_out = meter_slot.values.get(static_cast<uint16_t>(cached_index))->asFloat();

    if (max_age != 0_usec && deadline_elapsed(meter_slot.values_last_updated_at + max_age)) {
        return MeterValueAvailability::Stale;
    } else {
        return MeterValueAvailability::Fresh;
    }
}

MeterValueAvailability Meters::get_power(uint32_t slot, float *power, micros_t max_age)
{
    return get_single_value(slot, INDEX_CACHE_POWER, power, max_age);
}

MeterValueAvailability Meters::get_energy_import(uint32_t slot, float *total_import_kwh, micros_t max_age)
{
    return get_single_value(slot, INDEX_CACHE_ENERGY_IMPORT, total_import_kwh, max_age);
}

MeterValueAvailability Meters::get_energy_export(uint32_t slot, float *total_export_kwh, micros_t max_age)
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
    if (isnan(new_value))
        return;

    if (slot >= METERS_SLOTS) {
        logger.printfln("meters: Tried to update value %u for meter in non-existent slot %u.", index, slot);
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    // Think about ordering and short-circuting issues before changing this!
    bool was_nan = isnan(meter_slot.values.get(static_cast<uint16_t>(index))->asFloat());
    if (meter_slot.values.get(static_cast<uint16_t>(index))->updateFloat(new_value) && !was_nan)
        meter_slot.values_last_changed_at = now_us();

    meter_slot.values_last_updated_at = now_us();

    if (index == meter_slot.index_cache_single_values[INDEX_CACHE_POWER]) {
        meter_slot.power_history.add_sample(new_value);
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
    bool changed_any_value = false;

    for (uint16_t i = 0; i < value_count; i++) {
        if (!isnan(new_values[i])) {
            //auto wrap = values.get(i);
            //auto old_value = wrap->asFloat();
            //bool changed = wrap->updateFloat(new_values[i]) && !isnan(old_value);
            //(void)changed;

            // Think about ordering and short-circuting issues before changing this!
            bool was_nan = isnan(values.get(i)->asFloat());
            changed_any_value |= values.get(i)->updateFloat(new_values[i]) && !was_nan;
            updated_any_value = true;
        }
    }

    if (changed_any_value)
        meter_slot.values_last_changed_at = now_us();

    if (updated_any_value) {
        meter_slot.values_last_updated_at = now_us();

        float power;
        if (get_power(slot, &power) == MeterValueAvailability::Fresh) {
            meter_slot.power_history.add_sample(power);
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
    bool changed_any_value = false;

    if (new_values->count() != value_count) {
        logger.printfln("meters: Update all values element count mismatch: %u != %u", new_values->count(), value_count);
        return;
    }

    for (uint16_t i = 0; i < value_count; i++) {
        float val = new_values->get(i)->asFloat();
        if (!isnan(val)) {
            // Think about ordering and short-circuting issues before changing this!
            bool was_nan = isnan(values.get(i)->asFloat());
            changed_any_value |= values.get(i)->updateFloat(val) && !was_nan;
            updated_any_value = true;
        }
    }

    if (changed_any_value)
        meter_slot.values_last_changed_at = now_us();

    if (updated_any_value) {
        meter_slot.values_last_updated_at = now_us();

        float power;
        if (get_power(slot, &power) == MeterValueAvailability::Fresh) {
            meter_slot.power_history.add_sample(power);
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
        logger.printfln("meters: Meter in slot %u already declared %u values. Refusing to re-declare %u values.", slot, value_ids.count(), value_id_count);
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

void Meters::fill_index_cache(uint32_t slot, size_t find_value_count, const MeterValueID find_value_ids[], uint32_t index_cache[])
{
    if (slot >= METERS_SLOTS) {
        logger.printfln("meters: Tried to fill an index cache for meter in non-existent slot %u.", slot);
        return;
    }

    Config &value_ids = meter_slots[slot].value_ids;

    uint16_t value_id_count = static_cast<uint16_t>(value_ids.count());
    MeterValueID *value_ids_arr = static_cast<MeterValueID *>(malloc(value_id_count * sizeof(MeterValueID)));
    for (uint16_t i = 0; i < value_id_count; i++) {
        value_ids_arr[i] = static_cast<MeterValueID>(value_ids.get(i)->asUint());
    }

    for (size_t i = 0; i < find_value_count; i++) {
        index_cache[i] = meters_find_id_index(value_ids_arr, value_id_count, find_value_ids[i]);
    }

    free(value_ids_arr);
}

static const char *meters_path_postfixes[] = {"", "config", "state", "value_ids", "values", "errors", "reset", "last_reset"};
static_assert(ARRAY_SIZE(meters_path_postfixes) == static_cast<uint32_t>(Meters::PathType::_max) + 1, "Path postfix length mismatch");

String Meters::get_path(uint32_t slot, Meters::PathType path_type)
{
    String path = "meters/";
    path.concat(slot);
    path.concat('/');
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

float Meters::live_samples_per_second()
{
    float samples_per_second = 0;

    // Only calculate samples_per_second based on the last interval
    // if we have seen at least 2 values. With the API meter module,
    // it can happen that we see exactly one value in the first interval.
    // In this case 0 samples_per_second is reported for the next
    // interval (i.e. four minutes).
    if (samples_last_interval > 1) {
        uint32_t duration = end_last_interval - begin_last_interval;

        if (duration > 0) {
            // (samples_last_interval - 1) because there are N samples but only (N - 1) gaps
            // between them covering (end_last_interval - begin_last_interval) milliseconds
            samples_per_second = static_cast<float>((samples_last_interval - 1) * 1000) / static_cast<float>(duration);
        }
    }
    // Checking only for > 0 in this branch is fine: If we have seen
    // 0 or 1 samples in the last interval and exactly 1 in this interval,
    // we can only report that samples_per_second is 0.
    // This fixes itself when the next sample arrives.
    else if (samples_this_interval > 0) {
        uint32_t duration = end_this_interval - begin_this_interval;

        if (duration > 0) {
            // (samples_this_interval - 1) because there are N samples but only (N - 1) gaps
            // between them covering (end_this_interval - begin_this_interval) milliseconds
            samples_per_second = static_cast<float>((samples_this_interval - 1) * 1000) / static_cast<float>(duration);
        }
    }

    return samples_per_second;
}
