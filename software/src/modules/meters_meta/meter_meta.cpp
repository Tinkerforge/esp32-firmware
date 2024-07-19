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

#include "meter_meta.h"

#include <vector>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "meters_meta.h"
#include "modules/meters/meter_value_id.h"
#include "tools.h"

#include "gcc_warnings.h"

MeterClassID MeterMeta::get_class() const
{
    return MeterClassID::Meta;
}

void MeterMeta::setup(const Config &ephemeral_config)
{
    // Cache config
    mode           = static_cast<ConfigMode>(ephemeral_config.get("mode")->asUint());
    source_meter_a = ephemeral_config.get("source_meter_a")->asUint();
    source_meter_b = ephemeral_config.get("source_meter_b")->asUint();
    constant       = ephemeral_config.get("constant")->asInt();

    if (mode == ConfigMode::Sum || mode == ConfigMode::Diff) {
        source_mode = SourceMode::Double;
    } else if (mode == ConfigMode::Add || mode == ConfigMode::Mul || mode == ConfigMode::Pf2Current) {
        source_mode = SourceMode::Single;
    } else {
        source_mode = SourceMode::Unknown;
    }
}

void MeterMeta::register_events()
{
    if (source_meter_a == slot || (source_mode == SourceMode::Double && source_meter_b == slot)) {
        logger.printfln("Invalid source meter. Meta meter cannot listen to itself in slot %u. Slot A is %u, slot b is %u.", slot, source_meter_a, source_meter_b);
        return;
    }

    uint32_t value_ids_missing = 2;

    // Meter A
    String value_ids_path = meters.get_path(source_meter_a, Meters::PathType::ValueIDs);

    const Config *old_value_ids = api.getState(value_ids_path);
    if (old_value_ids->count() > 0) {
        value_ids_missing--;
    } else {
        event.registerEvent(value_ids_path, {}, [this](const Config *event_value_ids) {
            return on_value_ids_change(event_value_ids);
        });
    }

    if (source_mode == SourceMode::Double) {
        // Meter B
        value_ids_path = meters.get_path(source_meter_b, Meters::PathType::ValueIDs);

        old_value_ids = api.getState(value_ids_path);
        if (old_value_ids->count() > 0) {
            value_ids_missing--;
        } else {
            event.registerEvent(value_ids_path, {}, [this](const Config *event_value_ids) {
                return on_value_ids_change(event_value_ids);
            });
        }
    } else {
        value_ids_missing--;
    }

    if (value_ids_missing == 0) {
        on_value_ids_change(nullptr);
    }
}

struct value_id_pair {
    uint32_t value_id;
    uint32_t index_a;
    uint32_t index_b;
};

static void declare_value_ids(uint32_t slot, std::vector<struct value_id_pair> &value_id_pairs)
{
    MeterValueID value_ids[METERS_MAX_VALUES_PER_METER];
    size_t value_id_count = value_id_pairs.size();

    for (size_t i = 0; i < value_id_count; i++) {
        value_ids[i] = static_cast<MeterValueID>(value_id_pairs[i].value_id);
    }

    meters.declare_value_ids(slot, value_ids, value_id_count);
}

EventResult MeterMeta::on_value_ids_change(const Config *value_ids)
{
    if (value_ids && value_ids->count() <= 0) {
        return EventResult::OK;
    }

    const Config *value_ids_a = api.getState(meters.get_path(source_meter_a, Meters::PathType::ValueIDs));
    size_t value_count_a = value_ids_a->count();
    if (value_count_a == 0) {
        logger.printfln("Meter A value IDs not available yet.");
        return EventResult::OK;
    }

    const Config *value_ids_b = nullptr;
    size_t value_count_b;
    if (source_mode == SourceMode::Double) {
        value_ids_b = api.getState(meters.get_path(source_meter_b, Meters::PathType::ValueIDs));
        value_count_b = value_ids_b->count();

        if (value_count_b == 0) {
            logger.printfln("Meter B value IDs not available yet.");
            return EventResult::OK;
        }
    }

    if (mode == ConfigMode::Add || mode == ConfigMode::Mul) {
        logger.printfln("Mode %u not supported yet.", static_cast<uint32_t>(mode));
        return EventResult::Deregister;
    } else if (mode == ConfigMode::Sum || mode == ConfigMode::Diff) {
        std::vector<struct value_id_pair> value_id_pairs;
        value_id_pairs.reserve(min(value_count_a, value_count_b));

        for (uint16_t i_a = 0; i_a < value_count_a; i_a++) {
            uint32_t value_id_a = value_ids_a->get(i_a)->asUint();

            for (uint16_t i_b = 0; i_b < value_count_b; i_b++) {
                uint32_t value_id_b = value_ids_b->get(i_b)->asUint();

                if (value_id_a == value_id_b) {
                    value_id_pairs.push_back({value_id_a, i_a, i_b});
                }
            }
        }

        value_count = value_id_pairs.size();

        value_indices = static_cast<uint8_t(*)[][2]>(malloc(sizeof((*value_indices)[0][0]) * 2 * value_count));

        for (size_t i = 0; i < value_count; i++) {
            struct value_id_pair &value_id_pair = value_id_pairs[i];
            uint8_t *pair_indices = (*value_indices)[i];
            pair_indices[0] = static_cast<uint8_t>(value_id_pair.index_a);
            pair_indices[1] = static_cast<uint8_t>(value_id_pair.index_b);
        }
        declare_value_ids(slot, value_id_pairs);
    } else if (mode == ConfigMode::Pf2Current) {
        MeterValueID value_ids_pf2current[7] = {
            MeterValueID::PowerActiveLSumImExDiff,
            MeterValueID::CurrentL1ImExSum,
            MeterValueID::CurrentL2ImExSum,
            MeterValueID::CurrentL3ImExSum,
            MeterValueID::PowerFactorL1Directional,
            MeterValueID::PowerFactorL2Directional,
            MeterValueID::PowerFactorL3Directional,
        };

        uint32_t index_cache[ARRAY_SIZE(value_ids_pf2current)];
        meters.fill_index_cache(source_meter_a, ARRAY_SIZE(value_ids_pf2current), value_ids_pf2current, index_cache);

        for (size_t i = 0; i < ARRAY_SIZE(index_cache); i++) {
            if (index_cache[i] == UINT32_MAX) {
                logger.printfln("Value ID %u in position %u not provided by meter", static_cast<uint32_t>(value_ids_pf2current[i]), i);
                return EventResult::OK; // Try again, if possible.
            }
            if (index_cache[i] > UINT8_MAX) {
                logger.printfln("Index %u of value ID %u in position %u is out of range", index_cache[i], static_cast<uint32_t>(value_ids_pf2current[i]), i);
                return EventResult::OK; // Try again, if possible.
            }
        }

        value_indices = static_cast<uint8_t(*)[][2]>(malloc(sizeof((*value_indices)[0][0]) * 2 * ARRAY_SIZE(value_ids_pf2current)));
        if (!value_indices) {
            logger.printfln("Not enough memory for value_indices");
            return EventResult::OK; // Try again, if possible.
        }

        for (size_t i = 0; i < ARRAY_SIZE(index_cache); i++) {
            (*value_indices)[i][0] = static_cast<uint8_t>(index_cache[i]);
        }

        value_ids_pf2current[1] = MeterValueID::CurrentL1ImExDiff;
        value_ids_pf2current[2] = MeterValueID::CurrentL2ImExDiff;
        value_ids_pf2current[3] = MeterValueID::CurrentL3ImExDiff;

        meters.declare_value_ids(slot, value_ids_pf2current, ARRAY_SIZE(value_ids_pf2current));
    }

    String values_path_a = meters.get_path(source_meter_a, Meters::PathType::Values);

    if (source_mode == SourceMode::Single) {
        event.registerEvent(values_path_a, {}, [this](const Config *event_values) {
            this->on_values_change_single(event_values);
            return EventResult::OK;
        });

        const Config *values;
        if (meters.get_values(source_meter_a, &values, 0_us) == MeterValueAvailability::Fresh) {
            on_values_change_single(values);
        }
    } else if (source_mode == SourceMode::Double) {
        event.registerEvent(values_path_a, {}, [this](const Config */*event_values*/) {
            this->on_values_change_double();
            return EventResult::OK;
        });

        String values_path_b = meters.get_path(source_meter_b, Meters::PathType::Values);
        event.registerEvent(values_path_b, {}, [this](const Config */*event_values*/) {
            this->on_values_change_double();
            return EventResult::OK;
        });

        on_values_change_task_double();
    }

    return EventResult::Deregister;
}

void MeterMeta::on_values_change_single(const Config *source_values)
{
    if (mode == ConfigMode::Pf2Current) {
        float values[METER_META_PF_INDEX_COUNT];
        values[METER_META_PF_INDEX_POWER] = source_values->get((*value_indices)[METER_META_PF_INDEX_POWER][0])->asFloat();

        for (size_t i = 0; i < 3; i++) {
            union {
                float    f;
                uint32_t u32;
            } current, pf;

            current.f = source_values->get((*value_indices)[METER_META_PF_INDEX_CURRENT_L1 + i][0])->asFloat();
            pf.f      = source_values->get((*value_indices)[METER_META_PF_INDEX_PF_L1      + i][0])->asFloat();

            // Replace current value's sign with power factor value's sign.
            current.u32 ^= (current.u32 ^ pf.u32) & 0x80000000u;

            values[METER_META_PF_INDEX_CURRENT_L1 + i] = current.f;
            values[METER_META_PF_INDEX_PF_L1      + i] = pf.f;
        }

        meters.update_all_values(slot, values);
    } else {
        logger.printfln("Unsupported single values mode %u", static_cast<uint32_t>(mode));
    }
}

void MeterMeta::on_values_change_double()
{
    if (!update_pending) {
        update_pending = true;
        task_scheduler.scheduleOnce([this]() {
            this->on_values_change_task_double();
            this->update_pending = false;
        }, 0);
    }
}

void MeterMeta::on_values_change_task_double()
{
    const Config *values_a;
    const Config *values_b;

    MeterValueAvailability availability_a = meters.get_values(source_meter_a, &values_a, micros_t{2100 * 1000}); // 2.1s
    MeterValueAvailability availability_b = meters.get_values(source_meter_b, &values_b, micros_t{2100 * 1000}); // 2.1s

    if (values_a->count() <= 0 || values_b->count() <= 0) {
        return;
    }

    if (availability_a != MeterValueAvailability::Fresh) {
        //logger.printfln("Meter A values not fresh.");
        return;
    }

    if (availability_b != MeterValueAvailability::Fresh) {
        //logger.printfln("Meter B values not fresh.");
        return;
    }

    float values[METERS_MAX_VALUES_PER_METER];

    for (size_t i = 0; i < value_count; i++) {
        float value_a = values_a->get((*value_indices)[i][0])->asFloat();
        float value_b = values_b->get((*value_indices)[i][1])->asFloat();
        float value;

        if (mode == ConfigMode::Sum) {
            value = value_a + value_b;
        } else if (mode == ConfigMode::Diff) {
            value = value_a - value_b;
        } else {
            logger.printfln("Unsupported mode %u", static_cast<uint32_t>(mode));
            value = NAN;
        }

        values[i] = value;
    }

    meters.update_all_values(slot, values);
}
