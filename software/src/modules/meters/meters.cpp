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
#include "meter_value_imexdiff.h"
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

char *format_meter_slot(uint32_t slot)
{
    char *result;

    if (asprintf(&result, "Meter %lu: ", slot) < 0) {
        result = strdup("Meter ?: ");
    }

    return result;
}

void Meters::pre_setup()
{
    for (MeterSlot &meter_slot : meter_slots) {
        meter_slot.value_ids = Config::Array({},
            Config::get_prototype_uint32_0(),
            0, METERS_MAX_VALUES_PER_METER, Config::type_id<Config::ConfUint>()
        );
        meter_slot.values = Config::Array({},
            Config::get_prototype_float_nan(),
            0, METERS_MAX_VALUES_PER_METER, Config::type_id<Config::ConfFloat>()
        );

        meter_slot.values_last_updated_at = INT64_MIN;
        meter_slot.values_last_changed_at = INT64_MIN;

        init_uint32_array(meter_slot.index_cache_single_values, INDEX_CACHE_SINGLE_VALUES_COUNT, UINT32_MAX);
        init_uint32_array(meter_slot.index_cache_currents,      INDEX_CACHE_CURRENT_COUNT,       UINT32_MAX);
    }

    generators.reserve(METER_CLASS_ID_COUNT);
    register_meter_generator(MeterClassID::None, &meter_generator_none);

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_action(
        AutomationActionID::MeterReset,
        Config::Object({
            {"meter_slot", Config::Uint(0, 0, METERS_SLOTS - 1)}
        }),
        [this](const Config *config) {
            uint32_t slot = config->get("meter_slot")->asUint();
            const String err = api.callCommand(get_path(slot, Meters::PathType::Reset).c_str());
            if (!err.isEmpty()) {
                logger.printfln_meter("Automation couldn't reset meter: %s", err.c_str());
            }
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

    const Config last_reset_prototype = Config::Object({
        {"last_reset", Config::Uint53(0)}
    });

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
        IMeterGenerator *generator = get_generator_for_class(configured_meter_class);

        // Initialize state and errors to match (loaded) config.
        meter_slot.state  = *generator->get_state_prototype();
        meter_slot.errors = *generator->get_errors_prototype();

        // Create meter from config.
        Config *meter_state = &meter_slot.state;
        Config *meter_errors = &meter_slot.errors;

        IMeter *meter = new_meter_of_class(configured_meter_class, slot, meter_state, meter_errors);
        if (meter == nullptr) {
            logger.printfln_meter("Failed to create meter of class %lu", static_cast<uint32_t>(configured_meter_class));
            meter = new_meter_of_class(MeterClassID::None, slot, meter_state, meter_errors);
        }
        if (configured_meter_class != MeterClassID::None) {
            meter_slot.power_history.setup();
        }

        meter->setup(static_cast<Config *>(meter_slot.config_union.get()));
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
        micros_t now = now_us();
        uint32_t current_history_slot = (now / minutes_t{HISTORY_MINUTE_INTERVAL}).as<uint32_t>();
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
            char *buf = sb.take();

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
                char *buf = sb.take();

                ws.web_sockets.sendToAllOwned(buf, len);
            }
#endif
        }

        last_history_slot = current_history_slot;
    }, 500_ms);

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
        api.addState(get_path(slot, Meters::PathType::Values),   &meter_slot.values, {}, {}, METERS_VALUES_LOW_LATENCY);

        const String base_path = get_path(slot, Meters::PathType::Base);

        if (meter_slot.meter->get_class() != MeterClassID::None) {
            meter_slot.power_history.register_urls(base_path);
        } else {
            meter_slot.power_history.register_urls_empty(base_path);
        }

        if (meter_slot.meter->supports_reset()) {
            api.addCommand(get_path(slot, Meters::PathType::Reset), Config::Null(), {}, [this, &meter_slot, slot](String &/*errmsg*/) mutable {
                if (!meter_slot.meter->reset())
                    return;

                struct timeval tv_now;

                if (rtc.clock_synced(&tv_now)) {
                    meter_slot.last_reset.get("last_reset")->updateUint53(static_cast<uint64_t>(tv_now.tv_sec));
                } else {
                    uint64_t last = meter_slot.last_reset.get("last_reset")->asUint53();
                    if (last < 1000000000) {
                        meter_slot.last_reset.get("last_reset")->updateUint53(last + 1);
                    } else {
                        meter_slot.last_reset.get("last_reset")->updateUint53(1);
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

        sb.printf("{\"offset\":%lu,\"samples\":[", (now_us() - last_history_update).to<millis_t>().as<uint32_t>());
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

        sb.printf("{\"offset\":%lu,\"samples_per_second\":%f,\"samples\":[", (now_us() - last_live_update).to<millis_t>().as<uint32_t>(), static_cast<double>(live_samples_per_second()));
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

void Meters::register_events()
{
    for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
        meter_slots[slot].meter->register_events();
    }
}

void Meters::pre_reboot()
{
    for (uint32_t slot = 0; slot < METERS_SLOTS; slot++) {
        meter_slots[slot].meter->pre_reboot();
    }
}

void Meters::register_meter_generator(MeterClassID meter_class, IMeterGenerator *generator)
{
    for (const auto &generator_tuple : generators) {
        MeterClassID known_class = std::get<0>(generator_tuple);
        if (meter_class == known_class) {
            logger.printfln("Tried to register meter generator for already registered meter class %lu.", static_cast<uint32_t>(meter_class));
            return;
        }
    }

    generators.push_back({meter_class, generator});
}

IMeterGenerator *Meters::get_generator_for_class(MeterClassID meter_class)
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

    logger.printfln("No generator for meter class %lu.", static_cast<uint32_t>(meter_class));
    return get_generator_for_class(MeterClassID::None);
}

IMeter *Meters::new_meter_of_class(MeterClassID meter_class, uint32_t slot, Config *state, Config *errors)
{
    IMeterGenerator *generator = get_generator_for_class(meter_class);

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

MeterValueAvailability Meters::get_value_ids(uint32_t slot, const Config **value_ids)
{
    if (slot >= METERS_SLOTS) {
        *value_ids = nullptr;
        return MeterValueAvailability::Unavailable;
    }

    const MeterSlot &meter_slot = meter_slots[slot];

    *value_ids = &meter_slot.value_ids;

    if ((*value_ids)->count() == 0) {
        return MeterValueAvailability::CurrentlyUnknown;
    } else {
        return MeterValueAvailability::Fresh;
    }
}

static bool contains_value_id(const MeterValueID *value_ids, size_t value_id_count, MeterValueID needle)
{
    if (needle == MeterValueID::NotSupported) {
        return false;
    }

    for (size_t i = 0; i < value_id_count; i++) {
        if (value_ids[i] == needle) {
            return true;
        }
    }

    return false;
}

static const imexdiff_mapping *mvid_find_imexdiff_mapping(MeterValueID needle)
{
    for (size_t i = 0; i < meter_value_imexdiff_mappings_count; i++) {
        if (meter_value_imexdiff_mappings[i].imexdiff == needle) {
            return &meter_value_imexdiff_mappings[i];
        }
    }

    return nullptr;
}

static void generate_extra_value_ids(Meters::extra_value_id **extra_value_ids_, size_t *extra_value_id_count_, const Config *value_ids_config)
{
    MeterValueID value_ids[METERS_MAX_VALUES_PER_METER];
    const size_t value_id_count = value_ids_config->count();

    for (size_t i = 0; i < value_id_count; i++) {
        value_ids[i] = value_ids_config->get(i)->asEnum<MeterValueID>();
    }

    Meters::extra_value_id *extra_value_ids = static_cast<Meters::extra_value_id *>(malloc(2 * value_id_count * sizeof(Meters::extra_value_id)));
    size_t extra_value_id_count = 0;

    for (uint8_t i = 0; i < value_id_count; i++) {
        const MeterValueID value_id = value_ids[i];
        const MeterValueDirection direction = getMeterValueDirection(value_id);
        if (direction != MeterValueDirection::ImExDiff && direction != MeterValueDirection::IndCapDiff && direction != MeterValueDirection::ChaDisDiff) {
            continue;
        }

        const imexdiff_mapping *const mapping = mvid_find_imexdiff_mapping(value_id);
        if (!mapping) {
            continue;
        }

        const MeterValueID value_id_import = mapping->im;
        const MeterValueID value_id_export = mapping->ex;

        if (value_id_import != MeterValueID::NotSupported && !contains_value_id(value_ids, value_id_count, value_id_import)) {
            Meters::extra_value_id *extra_value_id = &extra_value_ids[extra_value_id_count];
            extra_value_id->value_id     = static_cast<uint16_t>(value_id_import);
            extra_value_id->source_index = i;
            extra_value_id->direction    = Meters::ExtraValueDirection::Positive;

            extra_value_id_count++;
        }

        if (value_id_export != MeterValueID::NotSupported && !contains_value_id(value_ids, value_id_count, value_id_export)) {
            Meters::extra_value_id *extra_value_id = &extra_value_ids[extra_value_id_count];
            extra_value_id->value_id     = static_cast<uint16_t>(value_id_export);
            extra_value_id->source_index = i;
            extra_value_id->direction    = Meters::ExtraValueDirection::Negative;

            extra_value_id_count++;
        }

        //logger.printfln("%s is ImExDiff, %zu extra value IDs", getMeterValueName(value_id), extra_value_id_count);
    }

    // Shrink array
    *extra_value_ids_ = static_cast<Meters::extra_value_id *>(realloc(extra_value_ids, extra_value_id_count * sizeof(Meters::extra_value_id)));
    extra_value_ids = nullptr;

    *extra_value_id_count_ = extra_value_id_count;
}

MeterValueAvailability Meters::get_value_ids_extended(uint32_t slot, MeterValueID *value_ids_out, size_t *value_ids_length)
{
    if (slot >= METERS_SLOTS) {
        *value_ids_length = 0;
        return MeterValueAvailability::Unavailable;
    }

    MeterSlot &meter_slot = meter_slots[slot];
    const Config &value_ids = meter_slot.value_ids;
    const size_t value_ids_count = value_ids.count();

    if (value_ids_count == 0) {
        return MeterValueAvailability::CurrentlyUnknown;
    }

    if (!meter_slot.extra_value_ids) {
        generate_extra_value_ids(&meter_slot.extra_value_ids, &meter_slot.extra_value_id_count, &value_ids);
    }

    if (!value_ids_out) {
        *value_ids_length = value_ids_count + meter_slot.extra_value_id_count;
        return MeterValueAvailability::Fresh;
    }

    const size_t value_ids_count_limited = std::min(value_ids_count, *value_ids_length);

    for (size_t i = 0; i < value_ids_count_limited; i++) {
        value_ids_out[i] = value_ids.get(i)->asEnum<MeterValueID>();
    }

    const extra_value_id *extra_value_ids = meter_slot.extra_value_ids;
    const size_t extra_value_ids_count = std::min(meter_slot.extra_value_id_count, *value_ids_length - value_ids_count_limited);
    value_ids_out += value_ids_count_limited;

    for (size_t i = 0; i < extra_value_ids_count; i++) {
        value_ids_out[i] = static_cast<MeterValueID>(extra_value_ids[i].value_id);
    }

    *value_ids_length = value_ids_count_limited + extra_value_ids_count;

    return MeterValueAvailability::Fresh;
}

static float get_extended_value(const Config *values, const Meters::extra_value_id *extra_value)
{
    const float base_value = values->get(extra_value->source_index)->asFloat();

    if (extra_value->direction == Meters::ExtraValueDirection::Positive) {
        return base_value > 0 ? base_value : 0;
    } else {
        return base_value < 0 ? base_value : 0;
    }
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

MeterValueAvailability Meters::get_values_with_cache(uint32_t slot, float *values, const uint32_t *index_cache, size_t value_count, micros_t max_age)
{
    if (slot >= METERS_SLOTS) {
        return MeterValueAvailability::Unavailable;
    }

    const MeterSlot &meter_slot = meter_slots[slot];
    const uint32_t meter_value_count = static_cast<uint32_t>(meter_slot.values.count());

    for (size_t i = 0; i < value_count; i++) {
        uint32_t index = index_cache[i];

        if (index == UINT32_MAX) {
            values[i] = NAN;
        } else if (index < meter_value_count) {
            values[i] = meter_slot.values.get(index)->asFloat();
        } else {
            uint32_t extra_value_index = index - meter_value_count;

            if (extra_value_index < meter_slot.extra_value_id_count) {
                values[i] = get_extended_value(&meter_slot.values, &(meter_slot.extra_value_ids[extra_value_index]));
            } else {
                logger.printfln_meter("Attempted to get index %lu via cache but have only %lu values and %u extra values", index, meter_value_count, meter_slot.extra_value_id_count);
                values[i] = NAN;
                return MeterValueAvailability::Unavailable;
            }
        }
    }

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
    const uint32_t value_count = static_cast<uint32_t>(meter_slot.values.count());

    if (index < value_count) {
        *value_out = meter_slot.values.get(index)->asFloat();
    } else {
        uint32_t extra_value_index = index - value_count;

        if (extra_value_index < meter_slot.extra_value_id_count) {
            *value_out = get_extended_value(&meter_slot.values, &(meter_slot.extra_value_ids[extra_value_index]));
        } else {
            logger.printfln_meter("Attempted to get index %lu but have only %lu values and %u extra values", index, value_count, meter_slot.extra_value_id_count);
            *value_out = NAN;
            return MeterValueAvailability::Unavailable;
        }
    }

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
            case INDEX_CACHE_POWER:          supported = meter_slot.meter->supports_power();  break;
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

    *value_out = meter_slot.values.get(cached_index)->asFloat();

    if (!this->meter_is_fresh(slot, max_age)) {
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

    uint32_t currents_available = 0;
    for (uint32_t i = 0; i < INDEX_CACHE_CURRENT_COUNT; i++) {
        uint32_t cached_index = meter_slot.index_cache_currents[i];

        if (cached_index == UINT32_MAX) {
            currents[i] = NAN;
        } else {
            currents_available++;
            currents[i] = values.get(cached_index)->asFloat();
        }
    }

    if (currents_available == 0) {
        // If the meter already declared its values but we didn't find any currents, we know they're not supported.
        if (meter_slot.values_declared || !meter_slot.meter->supports_currents()) {
            return MeterValueAvailability::Unavailable;
        } else {
            return MeterValueAvailability::CurrentlyUnknown;
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

        //logger.printfln_meter("Applying filter %s", filter->name);

        value_combiner_filter_bitmask &= ~(1u << 31 >> filter_num);
        filter_count++;
    } while (value_combiner_filter_bitmask);

    size_t extra_value_count = values.count() - base_value_count;
    for (size_t i = 0; i < extra_value_count; i++) {
        float value = extra_values[i];
        if (!isnan(value)) {
            values.get(base_value_count + i)->updateFloat(value);
        }
    }
}

void Meters::update_value(uint32_t slot, uint32_t index, float new_value)
{
    if (isnan(new_value))
        return;

    if (slot >= METERS_SLOTS) {
        logger.printfln_meter("Tried to update value %lu for non-existent slot", index);
        return;
    }

    if (index == UINT32_MAX) {
        logger.printfln_meter("Tried to update a value that is known to not exist (index = UINT32_MAX)");
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    Config &values = meter_slot.values;
    Config *conf_val = static_cast<Config *>(values.get(index));
    micros_t t_now = now_us();

    // Think about ordering and short-circuting issues before changing this!
    float old_value = conf_val->asFloat();
    if (conf_val->updateFloat(new_value) && !isnan(old_value))
        meter_slot.values_last_changed_at = t_now;

    meter_slot.values_last_updated_at = t_now;

    if (meter_slot.value_combiner_filters_bitmask) {
        float base_values[METERS_MAX_VALUES_PER_METER];
        size_t base_value_count = meter_slot.base_value_count;

        for (size_t i = 0; i < base_value_count; i++) {
            base_values[i] = values.get(i)->asFloat();
        }

        apply_filters(meter_slot, base_value_count, base_values);
    }
}

void Meters::update_all_values(uint32_t slot, const float new_values[])
{
    if (slot >= METERS_SLOTS) {
        logger.printfln_meter("Tried to update all values from array for non-existent slot");
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    Config &values = meter_slot.values;
    size_t base_value_count = meter_slot.base_value_count;
    bool updated_any_value = false;
    bool changed_any_value = false;

    for (size_t i = 0; i < base_value_count; i++) {
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
        logger.printfln_meter("Tried to update all values from Config for non-existent slot");
        return;
    }

    size_t value_count = meter_slots[slot].base_value_count;

    if (new_values->count() != value_count) {
        logger.printfln_meter("Update all values element count mismatch: %u != %u", new_values->count(), value_count);
        return;
    }

    float float_values[METERS_MAX_VALUES_PER_METER];
    for (size_t i = 0; i < value_count; i++) {
        float_values[i] = new_values->get(i)->asFloat();
    }

    update_all_values(slot, float_values);
}

void Meters::finish_update(uint32_t slot)
{
    if (slot >= METERS_SLOTS) {
        logger.printfln_meter("Tried to finish an update for non-existent slot");
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];
    float power;

    if (get_power(slot, &power) == MeterValueAvailability::Fresh) {
        meter_slot.power_history.add_sample(power);
    }
}

void Meters::declare_value_ids(uint32_t slot, const MeterValueID new_value_ids[], uint32_t value_id_count)
{
    if (slot >= METERS_SLOTS) {
        logger.printfln_meter("Tried to declare value IDs for meter in non-existent slot");
        return;
    }

    MeterSlot &meter_slot = meter_slots[slot];

    Config &value_ids = meter_slot.value_ids;
    Config &values    = meter_slot.values;

    size_t value_id_count_old = value_ids.count();
    if (value_id_count_old != 0) {
        const char *plural_s_old = value_id_count_old == 1 ? "" : "s";
        const char *plural_s_new = value_id_count     == 1 ? "" : "s";
        logger.printfln_meter("Meter already declared %u value%s. Refusing to re-declare %lu value%s", value_id_count_old, plural_s_old, value_id_count, plural_s_new);
        return;
    }

    if (value_id_count <= 0) {
        logger.printfln_meter("Cannot declare zero value IDs for meter");
        return;
    }

    MeterValueID total_value_ids[METERS_MAX_VALUES_PER_METER];
    memcpy(total_value_ids, new_value_ids, sizeof(MeterValueID) * value_id_count);

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
                logger.printfln_meter("Too many values (> %u) after applying filter '%s'", ARRAY_SIZE(total_value_ids), filter->name);
                filter_not_applicable = true;
                break;
            }
            total_value_ids[pos] = vid;
            filter_output_id_count++;
        }

        if (filter_not_applicable) {
            //logger.printfln_meter("Found output ID");
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
            //logger.printfln_meter("Input ID not found");
            all_filter_data.pop_back();
            continue;
        }

        total_value_id_count += filter_output_id_count;

        uint32_t filter_bitmask = 1u << 31 >> i_f;
        meter_slot.value_combiner_filters_bitmask |= filter_bitmask;
        //logger.printfln_meter("Applying filter %s", filter->name);
    }

    size_t filter_count = all_filter_data.size();
    value_combiner_filter_data *filter_data_compact = static_cast<value_combiner_filter_data *>(malloc(sizeof(value_combiner_filter_data) * filter_count));
    for (size_t i = 0; i < filter_count; i++) {
        const value_combiner_filter_data &filter_data = all_filter_data[i];
        memcpy(filter_data_compact + i, &filter_data, sizeof(value_combiner_filter_data));
    }
    meter_slot.value_combiner_filters_data = filter_data_compact;

    for (uint32_t i = 0; i < total_value_id_count; i++) {
        auto val = value_ids.add();
        val->updateUint(static_cast<uint32_t>(total_value_ids[i]));

        values.add();
    }

    meter_slot.index_cache_single_values[INDEX_CACHE_POWER]          = meters_find_id_index(total_value_ids, total_value_id_count, {
                                                                                            MeterValueID::PowerActiveLSumImExDiff,
                                                                                            MeterValueID::PowerDCImExDiff,
                                                                                            MeterValueID::PowerDCChaDisDiff,
                                                                                            MeterValueID::PowerPVSumImExDiff});
    meter_slot.index_cache_single_values[INDEX_CACHE_ENERGY_IMPORT]  = meters_find_id_index(total_value_ids, total_value_id_count, {
                                                                                            MeterValueID::EnergyActiveLSumImport,
                                                                                            MeterValueID::EnergyDCImport,
                                                                                            MeterValueID::EnergyDCCharge});
    meter_slot.index_cache_single_values[INDEX_CACHE_ENERGY_IMEXSUM] = meters_find_id_index(total_value_ids, total_value_id_count, {
                                                                                            MeterValueID::EnergyActiveLSumImExSum,
                                                                                            MeterValueID::EnergyDCImExSum,
                                                                                            MeterValueID::EnergyDCChaDisSum});
    meter_slot.index_cache_single_values[INDEX_CACHE_ENERGY_EXPORT]  = meters_find_id_index(total_value_ids, total_value_id_count, {
                                                                                            MeterValueID::EnergyActiveLSumExport,
                                                                                            MeterValueID::EnergyDCExport,
                                                                                            MeterValueID::EnergyDCDischarge,
                                                                                            MeterValueID::EnergyPVSumExport});
    meter_slot.index_cache_currents[INDEX_CACHE_CURRENT_L1]          = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::CurrentL1ImExDiff);
    meter_slot.index_cache_currents[INDEX_CACHE_CURRENT_L2]          = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::CurrentL2ImExDiff);
    meter_slot.index_cache_currents[INDEX_CACHE_CURRENT_L3]          = meters_find_id_index(total_value_ids, total_value_id_count, MeterValueID::CurrentL3ImExDiff);

    meter_slot.values_declared = true;

    const char *plural_s = total_value_id_count == 1 ? "" : "s";
    if (total_value_id_count == value_id_count) {
        logger.printfln_meter("Meter declared %lu value%s", total_value_id_count, plural_s);
    } else {
        logger.printfln_meter("Meter declared %lu (%lu) value%s", total_value_id_count, value_id_count, plural_s);
    }

    if (!meters_feature_declared) {
        api.addFeature("meters");
        meters_feature_declared = true;
    }
}

bool Meters::get_cached_power_index(uint32_t slot, uint32_t *index)
{
    *index = meter_slots[slot].index_cache_single_values[INDEX_CACHE_POWER];
    return *index != UINT32_MAX;
}

void Meters::fill_index_cache(uint32_t slot, size_t find_value_count, const MeterValueID find_value_ids[], uint32_t index_cache[])
{
    if (slot >= METERS_SLOTS) {
        logger.printfln_meter("Tried to fill an index cache for non-existent slot");
        return;
    }

    const MeterSlot &meter_slot = meter_slots[slot];
    const Config &value_ids = meter_slot.value_ids;
    const size_t value_id_count = value_ids.count();

    // Cache value IDs in a simple array to avoid excessive get() calls.
    MeterValueID value_ids_arr[METERS_MAX_VALUES_PER_METER];
    for (size_t i = 0; i < value_id_count; i++) {
        value_ids_arr[i] = value_ids.get(i)->asEnum<MeterValueID>();
    }

    for (size_t fv_i = 0; fv_i < find_value_count; fv_i++) {
        const MeterValueID mvid_to_find = find_value_ids[fv_i];

        for (uint32_t vid_i = 0; vid_i < value_id_count; vid_i++) {
            if (value_ids_arr[vid_i] == mvid_to_find) {
                index_cache[fv_i] = vid_i;
                goto mvid_found;
            }
        }

        {
            const extra_value_id *extra_value_ids = meter_slot.extra_value_ids;
            const uint32_t extra_value_id_count = meter_slot.extra_value_id_count;

            for (uint32_t evid_i = 0; evid_i < extra_value_id_count; evid_i++) {
                if (static_cast<MeterValueID>(extra_value_ids[evid_i].value_id) == mvid_to_find) {
                    index_cache[fv_i] = evid_i + value_id_count;
                    goto mvid_found;
                }
            }
        }

        index_cache[fv_i] = UINT32_MAX;

        mvid_found: {}
    }
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

uint32_t meters_find_id_index(const MeterValueID value_ids[], uint32_t value_id_count, std::initializer_list<MeterValueID> ids)
{
    for (MeterValueID id : ids) {
        for (uint32_t i = 0; i < value_id_count; i++) {
            if (value_ids[i] == id) {
                return i;
            }
        }
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
        auto duration = end_last_interval - begin_last_interval;

        if (duration > 0_us) {
            // (samples_last_interval - 1) because there are N samples but only (N - 1) gaps
            // between them covering (end_last_interval - begin_last_interval) milliseconds
            samples_per_second = static_cast<float>((samples_last_interval - 1) * 1000) / duration.to<millis_t>().as<float>();
        }
    }
    // Checking only for > 0 in this branch is fine: If we have seen
    // 0 or 1 samples in the last interval and exactly 1 in this interval,
    // we can only report that samples_per_second is 0.
    // This fixes itself when the next sample arrives.
    else if (samples_this_interval > 0) {
        auto duration = end_this_interval - begin_this_interval;

        if (duration > 0_us) {
            // (samples_this_interval - 1) because there are N samples but only (N - 1) gaps
            // between them covering (end_this_interval - begin_this_interval) milliseconds
            samples_per_second = static_cast<float>((samples_this_interval - 1) * 1000) / duration.to<millis_t>().as<float>();
        }
    }

    return samples_per_second;
}
