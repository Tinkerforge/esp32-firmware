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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "meter_class_none.h"
#include "tools.h"
#include "string_builder.h"

#include "gcc_warnings.h"
#ifdef __GNUC__
// The code is this file contains several casts to a type defined by a macro, which may result in useless casts.
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

static MeterGeneratorNone meter_generator_none;

static inline float get_value_from_concat_values(size_t index, size_t base_values_length, const float *base_values, float *extra_values)
{
    if (index < base_values_length) {
        return base_values[index];
    } else {
        return extra_values[index - base_values_length];
    }
}

// Takes three import values and three export values and will calculate their ImExDiff values.
static void filter_im_ex2imexdiff(const Meters::value_combiner_filter_data *filter_data, size_t base_values_length, const float *base_values, float *extra_values)
{
    const uint8_t *input_pos = filter_data->input_pos;
    const size_t output_pos = filter_data->output_pos - base_values_length;

    for (size_t i = 0; i < 3; i++) {
        float im = get_value_from_concat_values(input_pos[i    ], base_values_length, base_values, extra_values);
        float ex = get_value_from_concat_values(input_pos[i + 3], base_values_length, base_values, extra_values);

        extra_values[output_pos + i] = im - ex;
    }
}

// Takes 2x3 inputs and will copy the signs from the latter three values to the former three values
static void filter_sign2sign(const Meters::value_combiner_filter_data *filter_data, size_t base_values_length, const float *base_values, float *extra_values)
{
    const uint8_t *input_pos = filter_data->input_pos;
    const size_t output_pos = filter_data->output_pos - base_values_length;

    for (size_t i = 0; i < 3; i++) {
        union {
            float    f;
            uint32_t u32;
        } receiver, donor;

        receiver.f = get_value_from_concat_values(input_pos[i    ], base_values_length, base_values, extra_values);
        donor.f    = get_value_from_concat_values(input_pos[i + 3], base_values_length, base_values, extra_values);

        // Replace receiver value's sign with donor value's sign.
        receiver.u32 ^= (receiver.u32 ^ donor.u32) & 0x80000000u;

        extra_values[output_pos + i] = receiver.f;
    }
}

// Takes three phase values and will calculate their phase sum value.
static void filter_phase2sum(const Meters::value_combiner_filter_data *filter_data, size_t base_values_length, const float *base_values, float *extra_values)
{
    const uint8_t *input_pos = filter_data->input_pos;
    const size_t output_pos = filter_data->output_pos - base_values_length;

    float l1 = get_value_from_concat_values(input_pos[0], base_values_length, base_values, extra_values);
    float l2 = get_value_from_concat_values(input_pos[1], base_values_length, base_values, extra_values);
    float l3 = get_value_from_concat_values(input_pos[2], base_values_length, base_values, extra_values);

    extra_values[output_pos] = l1 + l2 + l3;
}

// Filters will be run in array order
static const Meters::value_combiner_filter value_combiner_filters[] = {
    {
        &filter_im_ex2imexdiff,
        "Power import and export to ImExDiff",
        {
            MeterValueID::PowerActiveL1Import,
            MeterValueID::PowerActiveL2Import,
            MeterValueID::PowerActiveL3Import,
            MeterValueID::PowerActiveL1Export,
            MeterValueID::PowerActiveL2Export,
            MeterValueID::PowerActiveL3Export,
        },
        {
            MeterValueID::PowerActiveL1ImExDiff,
            MeterValueID::PowerActiveL2ImExDiff,
            MeterValueID::PowerActiveL3ImExDiff,
        },
    },
    {   // Prefer directional currents from power factors over power
        &filter_sign2sign,
        "Directional currents from power factors",
        {
            MeterValueID::CurrentL1ImExSum,
            MeterValueID::CurrentL2ImExSum,
            MeterValueID::CurrentL3ImExSum,
            MeterValueID::PowerFactorL1Directional,
            MeterValueID::PowerFactorL2Directional,
            MeterValueID::PowerFactorL3Directional,
        },
        {
            MeterValueID::CurrentL1ImExDiff,
            MeterValueID::CurrentL2ImExDiff,
            MeterValueID::CurrentL3ImExDiff,
        },
    },
    {
        &filter_sign2sign,
        "Directional currents from power",
        {
            MeterValueID::CurrentL1ImExSum,
            MeterValueID::CurrentL2ImExSum,
            MeterValueID::CurrentL3ImExSum,
            MeterValueID::PowerActiveL1ImExDiff,
            MeterValueID::PowerActiveL2ImExDiff,
            MeterValueID::PowerActiveL3ImExDiff,
        },
        {
            MeterValueID::CurrentL1ImExDiff,
            MeterValueID::CurrentL2ImExDiff,
            MeterValueID::CurrentL3ImExDiff,
        },
    },
    {
        &filter_phase2sum,
        "Phase sum from phases",
        {
            MeterValueID::PowerActiveL1ImExDiff,
            MeterValueID::PowerActiveL2ImExDiff,
            MeterValueID::PowerActiveL3ImExDiff,
        },
        {
            MeterValueID::PowerActiveLSumImExDiff,
        },
    },
};

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

    generators.reserve(METER_CLASS_ID_COUNT);
    register_meter_generator(MeterClassID::None, &meter_generator_none);

    last_reset_prototype = Config::Object({
        {"last_reset", Config::Uint32(0)}
    });


#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_action(
        AutomationActionID::MeterReset,
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
            logger.printfln("Failed to create meter of class %u in slot %u.", static_cast<uint32_t>(configured_meter_class), slot);
            meter = new_meter_of_class(MeterClassID::None, slot, meter_state, meter_errors);
        }
        if (configured_meter_class != MeterClassID::None) {
            meter_slot.power_history.setup();
        }

        meter->setup(*static_cast<const Config *>(meter_slot.config_union.get()));
        // Setup before calling supports_reset to allow a meter to decide in
        // setup whether to support reset. This could for example depend on the
        // meter's configuration.
        if (meter->supports_reset()) {
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
        StringBuilder sb;

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
        if (sb.setCapacity(METERS_SLOTS * history_chars_per_value + 100)) {
            sb.printf("{\"topic\":\"meters/live_samples\",\"payload\":{\"samples_per_second\":%f,\"samples\":[", static_cast<double>(live_samples_per_second()));

            for (uint32_t slot = 0; slot < METERS_SLOTS && sb.getRemainingLength() > 0; slot++) {
                if (!valid_samples[slot]) {
                    sb.printf(slot == 0 ? "[%s]" : ",[%s]", "");
                }
                else if (live_samples[slot] == val_min) {
                    sb.printf(slot == 0 ? "[%s]" : ",[%s]", "null");
                }
                else {
                    sb.printf(slot == 0 ? "[%d]" : ",[%d]", static_cast<int>(live_samples[slot]));
                }
            }

            sb.puts("]}}\n");

            size_t len = sb.getLength();
            char *buf = sb.take().release();

            ws.web_sockets.sendToAllOwned(buf, len);
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
            if (sb.setCapacity(METERS_SLOTS * history_chars_per_value + 100)) {
                sb.puts("{\"topic\":\"meters/history_samples\",\"payload\":{\"samples\":[");

                for (uint32_t slot = 0; slot < METERS_SLOTS && sb.getRemainingLength() > 0; slot++) {
                    if (!valid_samples[slot]) {
                        sb.printf(slot == 0 ? "[%s]" : ",[%s]", "");
                    }
                    else if (history_samples[slot] == val_min) {
                        sb.printf(slot == 0 ? "[%s]" : ",[%s]", "null");
                    }
                    else {
                        sb.printf(slot == 0 ? "[%d]" : ",[%d]", static_cast<int>(history_samples[slot]));
                    }
                }

                sb.puts("]}}\n");

                size_t len = sb.getLength();
                char *buf = sb.take().release();

                ws.web_sockets.sendToAllOwned(buf, len);
            }
#endif
        }

        last_history_slot = current_history_slot;
    }, 0, 500);

    initialized = true;
}

#if defined(METERS_LOW_LATENCY) && METERS_LOW_LATENCY != 0
#define METERS_VALUES_LOW_LATENCY true
#else
#define METERS_VALUES_LOW_LATENCY false
#endif

void Meters::register_urls()
{
    for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
        MeterSlot &meter_slot = meter_slots[slot];

        api.addPersistentConfig(get_path(slot, Meters::PathType::Config), &meter_slot.config_union);
        api.addState(get_path(slot, Meters::PathType::State),    &meter_slot.state);
        api.addState(get_path(slot, Meters::PathType::Errors),   &meter_slot.errors);
        api.addState(get_path(slot, Meters::PathType::ValueIDs), &meter_slot.value_ids);
        api.addState(get_path(slot, Meters::PathType::Values),   &meter_slot.values, {}, METERS_VALUES_LOW_LATENCY);

        const String base_path = get_path(slot, Meters::PathType::Base);

        if (meter_slot.meter->get_class() != MeterClassID::None) {
            meter_slot.power_history.register_urls(base_path);
        } else {
            meter_slot.power_history.register_urls_empty(base_path);
        }

        if (meter_slot.meter->supports_reset()) {
            api.addCommand(get_path(slot, Meters::PathType::Reset), Config::Null(), {}, [this, &meter_slot, slot]() mutable {
                if (!meter_slot.meter->reset())
                    return;

                struct timeval tv_now;

                if (clock_synced(&tv_now)) {
                    //FIXME not Y2038-safe
                    meter_slot.last_reset.get("last_reset")->updateUint(static_cast<uint32_t>(tv_now.tv_sec));
                } else {
                    uint32_t last = meter_slot.last_reset.get("last_reset")->asUint();
                    if (last < 1000000000) {
                        meter_slot.last_reset.get("last_reset")->updateUint(last + 1);
                    } else {
                        meter_slot.last_reset.get("last_reset")->updateUint(1);
                    }
                }
                api.writeConfig(get_path(slot, Meters::PathType::LastReset), &meter_slot.last_reset);
            }, true);

            api.addState(get_path(slot, Meters::PathType::LastReset), &meter_slot.last_reset);
        }

        meter_slot.meter->register_urls(base_path);
    }

    server.on("/meters/history", HTTP_GET, [this](WebServerRequest request) {
        StringBuilder sb;

        if (!sb.setCapacity(HISTORY_RING_BUF_SIZE * history_chars_per_value + 100)) {
            return request.send(500, "text/plain", "Failed to allocate buffer");
        }

        sb.printf("{\"offset\":%lu,\"samples\":[", millis() - last_history_update);
        request.beginChunkedResponse(200, "application/json; charset=utf-8");

        for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
            MeterSlot &meter_slot = meter_slots[slot];

            if (meter_slot.meter->get_class() != MeterClassID::None) {
                sb.puts(slot == 0 ? "[" : ",[");
                meter_slot.power_history.format_history_samples(&sb);
                sb.puts("]");
            } else {
                sb.puts(slot == 0 ? "null" : ",null");
            }

            request.sendChunk(sb.getPtr(), static_cast<ssize_t>(sb.getLength()));
            sb.clear();
        }

        request.sendChunk("]}", 2);

        return request.endChunkedResponse();
    });

    server.on("/meters/live", HTTP_GET, [this](WebServerRequest request) {
        StringBuilder sb;

        if (!sb.setCapacity(HISTORY_RING_BUF_SIZE * history_chars_per_value + 100)) {
            return request.send(500, "text/plain", "Failed to allocate buffer");
        }

        sb.printf("{\"offset\":%lu,\"samples_per_second\":%f,\"samples\":[", millis() - last_live_update, static_cast<double>(live_samples_per_second()));
        request.beginChunkedResponse(200, "application/json; charset=utf-8");

        for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
            MeterSlot &meter_slot = meter_slots[slot];

            if (meter_slot.meter->get_class() != MeterClassID::None) {
                sb.puts(slot == 0 ? "[" : ",[");
                meter_slot.power_history.format_live_samples(&sb);
                sb.puts("]");
            } else {
                sb.puts(slot == 0 ? "null" : ",null");
            }

            request.sendChunk(sb.getPtr(), static_cast<ssize_t>(sb.getLength()));
            sb.clear();
        }

        request.sendChunk("]}", 2);

        return request.endChunkedResponse();
    });

#if MODULE_METERS_LEGACY_API_AVAILABLE()
    if (meters_legacy_api.get_linked_meter_slot() < METERS_SLOTS) {
        api.addState("meter/error_counters", &meter_slots[meters_legacy_api.get_linked_meter_slot()].errors);
        meter_slots[meters_legacy_api.get_linked_meter_slot()].power_history.register_urls("meter/");
    }
#endif
}

void Meters::pre_reboot()
{
    for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
        meter_slots[slot].meter->pre_reboot();
    }
}

void Meters::register_meter_generator(MeterClassID meter_class, MeterGenerator *generator)
{
    for (const auto &generator_tuple : generators) {
        MeterClassID known_class = std::get<0>(generator_tuple);
        if (meter_class == known_class) {
            logger.printfln("Tried to register meter generator for already registered meter class %u.", static_cast<uint32_t>(meter_class));
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
        logger.printfln("No generator for dummy meter available. This is probably fatal.");
        return nullptr;
    }

    logger.printfln("No generator for meter class %u.", static_cast<uint32_t>(meter_class));
    return get_generator_for_class(MeterClassID::None);
}

IMeter *Meters::new_meter_of_class(MeterClassID meter_class, uint32_t slot, Config *state, Config *errors)
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
    return max_age_us == 0_us || !deadline_elapsed(meter_slots[slot].values_last_updated_at + max_age_us);
}

bool Meters::meter_has_value_changed(uint32_t slot, micros_t max_age_us)
{
    return max_age_us == 0_us || !deadline_elapsed(meter_slots[slot].values_last_changed_at + max_age_us);
}

MeterValueAvailability Meters::get_values(uint32_t slot, const Config **values, micros_t max_age)
{
    if (slot >= METERS_SLOTS) {
        *values = nullptr;
        return MeterValueAvailability::Unavailable;
    }

    const MeterSlot &meter_slot = meter_slots[slot];

    *values = &meter_slot.values;

    if (!this->meter_is_fresh(slot, max_age)) {
        return MeterValueAvailability::Stale;
    } else {
        return MeterValueAvailability::Fresh;
    }
}

MeterValueAvailability Meters::get_value_by_index(uint32_t slot, uint32_t index, float *value_out, micros_t max_age)
{
    if (slot >= METERS_SLOTS || index == UINT32_MAX) {
        *value_out = NAN;
        return MeterValueAvailability::Unavailable;
    }

    const MeterSlot &meter_slot = meter_slots[slot];

    *value_out = meter_slot.values.get(static_cast<uint16_t>(index))->asFloat();

    if (!this->meter_is_fresh(slot, max_age)) {
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
            case INDEX_CACHE_POWER_REAL:     // fallthrough
            case INDEX_CACHE_POWER_VIRTUAL:  supported = meter_slot.meter->supports_power();  break;
            case INDEX_CACHE_ENERGY_IMPORT:  supported = meter_slot.meter->supports_energy_import(); break;
            case INDEX_CACHE_ENERGY_IMEXSUM: supported = meter_slot.meter->supports_energy_imexsum(); break;
            case INDEX_CACHE_ENERGY_EXPORT:  supported = meter_slot.meter->supports_energy_export(); break;
            default: supported = false;
        }
        if (supported) {
            return MeterValueAvailability::CurrentlyUnknown;
        } else {
            return MeterValueAvailability::Unavailable;
        }
    }

    *value_out = meter_slot.values.get(static_cast<uint16_t>(cached_index))->asFloat();

    if (!this->meter_is_fresh(slot, max_age)) {
        return MeterValueAvailability::Stale;
    } else {
        return MeterValueAvailability::Fresh;
    }
}

MeterValueAvailability Meters::get_power_real(uint32_t slot, float *power, micros_t max_age)
{
    return get_single_value(slot, INDEX_CACHE_POWER_REAL, power, max_age);
}

MeterValueAvailability Meters::get_power_virtual(uint32_t slot, float *power, micros_t max_age)
{
    return get_single_value(slot, INDEX_CACHE_POWER_VIRTUAL, power, max_age);
}

MeterValueAvailability Meters::get_energy_import(uint32_t slot, float *total_import_kwh, micros_t max_age)
{
    return get_single_value(slot, INDEX_CACHE_ENERGY_IMPORT, total_import_kwh, max_age);
}

MeterValueAvailability Meters::get_energy_imexsum(uint32_t slot, float *total_imexsum_kwh, micros_t max_age)
{
    return get_single_value(slot, INDEX_CACHE_ENERGY_IMEXSUM, total_imexsum_kwh, max_age);
}

MeterValueAvailability Meters::get_energy_export(uint32_t slot, float *total_export_kwh, micros_t max_age)
{
    return get_single_value(slot, INDEX_CACHE_ENERGY_EXPORT, total_export_kwh, max_age);
}

MeterValueAvailability Meters::get_currents(uint32_t slot, float currents[INDEX_CACHE_CURRENT_COUNT], micros_t max_age)
{
    if (slot >= METERS_SLOTS) {
        return MeterValueAvailability::Unavailable;
    }

    const MeterSlot &meter_slot = meter_slots[slot];
    const ConfigRoot &values = meter_slot.values;

    uint32_t currents_unavailable = 0;
    for (uint32_t i = 0; i < INDEX_CACHE_CURRENT_COUNT; i++) {
        uint32_t cached_index = meter_slot.index_cache_currents[i];

        if (cached_index == UINT32_MAX) {
            currents_unavailable++;
            currents[i] = NAN;
        } else {
            currents[i] = values.get(static_cast<uint16_t>(cached_index))->asFloat();
        }
    }

    if (currents_unavailable > 0) {
        if (meter_slot.meter->supports_currents()) {
            return MeterValueAvailability::CurrentlyUnknown;
        } else {
            return MeterValueAvailability::Unavailable;
        }
    }

    if (!this->meter_is_fresh(slot, max_age)) {
        return MeterValueAvailability::Stale;
    } else {
        return MeterValueAvailability::Fresh;
    }
}

void Meters::apply_filters(MeterSlot &meter_slot, size_t base_value_count, const float *base_values)
{
    Config &values = meter_slot.values;
    float extra_values[METERS_MAX_VALUES_PER_METER];
    size_t filter_count = 0;
    uint32_t value_combiner_filter_bitmask = meter_slot.value_combiner_filters_bitmask;

    // Can use do-while because function is only called if the filter bitmask is non-zero.
    do {
        int32_t filter_num = __builtin_clz(value_combiner_filter_bitmask);
        const value_combiner_filter *filter = value_combiner_filters + filter_num;

        const value_combiner_filter_data *filter_data = meter_slot.value_combiner_filters_data + filter_count;
        filter->fn(filter_data, base_value_count, base_values, extra_values);

        //logger.printfln("Applying filter %s for slot %u", filter->name, slot);

        value_combiner_filter_bitmask &= ~(1u << 31 >> filter_num);
        filter_count++;
    } while (value_combiner_filter_bitmask);

    size_t extra_value_count = values.count() - base_value_count;
    for (size_t i = 0; i < extra_value_count; i++) {
        float value = extra_values[i];
        if (!isnan(value)) {
            values.get(static_cast<uint16_t>(base_value_count + i))->updateFloat(value);
        }
    }
}

void Meters::update_value(uint32_t slot, uint32_t index, float new_value)
{
    if (isnan(new_value))
        return;

    if (slot >= METERS_SLOTS) {
        logger.printfln("Tried to update value %u for meter in non-existent slot %u.", index, slot);
        return;
    }

    if (index == UINT32_MAX) {
        logger.printfln("Tried to update a value for meter in slot %u that is known to not exist (index = UINT32_MAX).", slot);
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    Config &values = meter_slot.values;
    Config *conf_val = static_cast<Config *>(values.get(static_cast<uint16_t>(index)));
    micros_t t_now = now_us();

    // Think about ordering and short-circuting issues before changing this!
    float old_value = conf_val->asFloat();
    if (conf_val->updateFloat(new_value) && !isnan(old_value))
        meter_slot.values_last_changed_at = t_now;

    meter_slot.values_last_updated_at = t_now;

    if (meter_slot.value_combiner_filters_bitmask) {
        float base_values[METERS_MAX_VALUES_PER_METER];
        size_t base_value_count = meter_slot.base_value_count;

        for (uint16_t i = 0; i < base_value_count; i++) {
            base_values[i] = values.get(i)->asFloat();
        }

        apply_filters(meter_slot, base_value_count, base_values);
    }
}

void Meters::update_all_values(uint32_t slot, const float new_values[])
{
    if (slot >= METERS_SLOTS) {
        logger.printfln("Tried to update all values from array for meter in non-existent slot %u.", slot);
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    Config &values = meter_slot.values;
    auto base_value_count = meter_slot.base_value_count;
    bool updated_any_value = false;
    bool changed_any_value = false;

    for (uint16_t i = 0; i < base_value_count; i++) {
        float new_value = new_values[i];
        if (!isnan(new_value)) {
            Config *conf_val = static_cast<Config *>(values.get(i));

            // Think about ordering and short-circuting issues before changing this!
            float old_value = conf_val->asFloat();
            if (conf_val->updateFloat(new_value) && !isnan(old_value))
                changed_any_value = true;

            updated_any_value = true;
        }
    }

    if (meter_slot.value_combiner_filters_bitmask) {
        apply_filters(meter_slot, base_value_count, new_values);
    }

    micros_t t_now = now_us();

    if (changed_any_value)
        meter_slot.values_last_changed_at = t_now;

    if (updated_any_value)
        meter_slot.values_last_updated_at = t_now;

    finish_update(slot);
}

void Meters::update_all_values(uint32_t slot, const Config *new_values)
{
    if (slot >= METERS_SLOTS) {
        logger.printfln("Tried to update all values from Config for meter in non-existent slot %u.", slot);
        return;
    }

    auto value_count = meter_slots[slot].base_value_count;

    if (new_values->count() != value_count) {
        logger.printfln("Update all values element count mismatch: %u != %u", new_values->count(), value_count);
        return;
    }

    float float_values[METERS_MAX_VALUES_PER_METER];
    for (uint16_t i = 0; i < value_count; i++) {
        float_values[i] = new_values->get(i)->asFloat();
    }

    update_all_values(slot, float_values);
}

void Meters::finish_update(uint32_t slot)
{
    if (slot >= METERS_SLOTS) {
        logger.printfln("Tried to finish an update for meter in non-existent slot %u.", slot);
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];
    float power;

    if (get_power_real(slot, &power) == MeterValueAvailability::Fresh) {
        meter_slot.power_history.add_sample(power);
    }
}

void Meters::declare_value_ids(uint32_t slot, const MeterValueID new_value_ids[], uint32_t value_id_count)
{
    if (slot >= METERS_SLOTS) {
        logger.printfln("Tried to declare value IDs for meter in non-existent slot %u.", slot);
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    Config &value_ids = meter_slot.value_ids;
    Config &values    = meter_slot.values;

    if (value_ids.count() != 0) {
        logger.printfln("Meter in slot %u already declared %u values. Refusing to re-declare %u values.", slot, value_ids.count(), value_id_count);
        return;
    }

    if (value_id_count <= 0) {
        logger.printfln("Cannot declare zero value IDs for meter in slot %u.", value_id_count);
        return;
    }

    MeterValueID total_value_ids[METERS_MAX_VALUES_PER_METER];
    mempcpy(total_value_ids, new_value_ids, sizeof(MeterValueID) * value_id_count);

    uint32_t total_value_id_count = value_id_count;
    meter_slot.base_value_count = value_id_count;
    std::vector<value_combiner_filter_data> all_filter_data;

    for (size_t i_f = 0; i_f < ARRAY_SIZE(value_combiner_filters); i_f++) {
        const value_combiner_filter *filter = value_combiner_filters + i_f;
        const MeterValueID *output_ids = filter->output_ids;

        bool filter_not_applicable = false;
        uint32_t filter_output_id_count = 0;

        for (size_t i_oi = 0; i_oi < METERS_MAX_FILTER_VALUES; i_oi++) {
            MeterValueID vid = output_ids[i_oi];
            if (vid == MeterValueID::NotSupported) {
                break;
            }

            if (meters_find_id_index(total_value_ids, total_value_id_count, vid) != UINT32_MAX) {
                filter_not_applicable = true;
                break;
            }

            uint32_t pos = total_value_id_count + filter_output_id_count;
            if (pos >= ARRAY_SIZE(total_value_ids)) {
                logger.printfln("Too many values (>%u) after applying filter '%s'", ARRAY_SIZE(total_value_ids), filter->name);
                filter_not_applicable = true;
                break;
            }
            total_value_ids[pos] = vid;
            filter_output_id_count++;
        }

        if (filter_not_applicable) {
            //logger.printfln("Found output ID for slot %u", slot);
            continue;
        }

        const MeterValueID *input_ids = filter->input_ids;
        all_filter_data.resize(all_filter_data.size() + 1);
        Meters::value_combiner_filter_data &filter_data = all_filter_data.back();
        filter_data.output_pos = static_cast<uint8_t>(total_value_id_count);

        for (size_t i_ii = 0; i_ii < METERS_MAX_FILTER_VALUES; i_ii++) {
            MeterValueID vid = input_ids[i_ii];
            if (vid == MeterValueID::NotSupported) {
                break;
            }

            uint32_t pos = meters_find_id_index(total_value_ids, total_value_id_count, vid);
            if (pos == UINT32_MAX) {
                filter_not_applicable = true;
                break;
            }

            filter_data.input_pos[i_ii] = static_cast<uint8_t>(pos);
        }

        if (filter_not_applicable) {
            //logger.printfln("Input ID for slot %u not found", slot);
            all_filter_data.pop_back();
            continue;
        }

        total_value_id_count += filter_output_id_count;

        uint32_t filter_bitmask = 1u << 31 >> i_f;
        meter_slot.value_combiner_filters_bitmask |= filter_bitmask;
        //logger.printfln("Applying filter %s for slot %u", filter->name, slot);
    }

    size_t filter_count = all_filter_data.size();
    value_combiner_filter_data *filter_data_compact = static_cast<value_combiner_filter_data *>(malloc(sizeof(value_combiner_filter_data) * filter_count));
    for (size_t i = 0; i < filter_count; i++) {
        const value_combiner_filter_data &filter_data = all_filter_data[i];
        memcpy(filter_data_compact + i, &filter_data, sizeof(value_combiner_filter_data));
    }
    meter_slot.value_combiner_filters_data = filter_data_compact;

    for (uint16_t i = 0; i < static_cast<uint16_t>(total_value_id_count); i++) {
        auto val = value_ids.add();
        val->updateUint(static_cast<uint32_t>(total_value_ids[i]));

        values.add();
    }

    uint32_t index_power_ac            = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::PowerActiveLSumImExDiff);
    uint32_t index_power_dc            = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::PowerDCImExDiff);
    uint32_t index_power_dc_battery    = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::PowerDCChaDisDiff);
    uint32_t index_power_real          = std::min({index_power_ac, index_power_dc, index_power_dc_battery});
    uint32_t index_power_virtual       = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::PowerActiveLSumImExDiffVirtual);
    uint32_t index_energy_ac_import    = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::EnergyActiveLSumImport);
    uint32_t index_energy_ac_imexsum   = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::EnergyActiveLSumImExSum);
    uint32_t index_energy_ac_export    = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::EnergyActiveLSumExport);
    uint32_t index_energy_dc_import    = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::EnergyDCImport);
    uint32_t index_energy_dc_imexsum   = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::EnergyDCImExSum);
    uint32_t index_energy_dc_export    = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::EnergyDCExport);
    uint32_t index_energy_dc_charge    = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::EnergyDCCharge);
    uint32_t index_energy_dc_chadissum = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::EnergyDCChaDisSum);
    uint32_t index_energy_dc_discharge = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::EnergyDCDischarge);

    meter_slot.index_cache_single_values[INDEX_CACHE_POWER_REAL]     = index_power_real;
    meter_slot.index_cache_single_values[INDEX_CACHE_POWER_VIRTUAL]  = index_power_virtual != UINT32_MAX ? index_power_virtual : index_power_real;
    meter_slot.index_cache_single_values[INDEX_CACHE_ENERGY_IMPORT]  = std::min({index_energy_ac_import,  index_energy_dc_import,  index_energy_dc_charge});
    meter_slot.index_cache_single_values[INDEX_CACHE_ENERGY_IMEXSUM] = std::min({index_energy_ac_imexsum, index_energy_dc_imexsum, index_energy_dc_chadissum});
    meter_slot.index_cache_single_values[INDEX_CACHE_ENERGY_EXPORT]  = std::min({index_energy_ac_export,  index_energy_dc_export,  index_energy_dc_discharge});
    meter_slot.index_cache_currents[INDEX_CACHE_CURRENT_L1]          = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::CurrentL1ImExDiff);
    meter_slot.index_cache_currents[INDEX_CACHE_CURRENT_L2]          = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::CurrentL2ImExDiff);
    meter_slot.index_cache_currents[INDEX_CACHE_CURRENT_L3]          = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::CurrentL3ImExDiff);

    meter_slot.values_declared = true;

    if (total_value_id_count == value_id_count) {
        logger.printfln("Meter in slot %u declared %u values.", slot, total_value_id_count);
    } else {
        logger.printfln("Meter in slot %u declared %u (%u) values.", slot, total_value_id_count, value_id_count);
    }

    if (!meters_feature_declared) {
        api.addFeature("meters");
        meters_feature_declared = true;
    }
}

bool Meters::get_cached_real_power_index(uint32_t slot, uint32_t *index)
{
    *index = meter_slots[slot].index_cache_single_values[INDEX_CACHE_POWER_REAL];
    return *index != UINT32_MAX;
}

void Meters::fill_index_cache(uint32_t slot, size_t find_value_count, const MeterValueID find_value_ids[], uint32_t index_cache[])
{
    if (slot >= METERS_SLOTS) {
        logger.printfln("Tried to fill an index cache for meter in non-existent slot %u.", slot);
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

[[gnu::const]]
const Config *Meters::get_config_bool_false_prototype() const
{
    return &config_bool_false_prototype;
}

[[gnu::const]]
const Config *Meters::get_config_float_nan_prototype()
{
    if (config_float_nan_prototype.is_null()) {
        config_float_nan_prototype = Config::Float(NAN);
    }
    return &config_float_nan_prototype;
}

[[gnu::const]]
const Config *Meters::get_config_uint_max_prototype()
{
    if (config_uint_max_prototype.is_null()) {
        config_uint_max_prototype = Config::Uint32(UINT32_MAX);
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
