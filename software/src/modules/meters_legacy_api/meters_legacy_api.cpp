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

#define EVENT_LOG_PREFIX "meters_legacy"

#include "meters_legacy_api.h"

#include <math.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modules/meters/sdm_helpers.h"
#include "tools.h"

#include "gcc_warnings.h"

static const MeterValueID legacy_values_ids[3] = {
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImExSum,
};

static const MeterValueID value_ids_import_imex[4] = {
    MeterValueID::EnergyActiveLSumImportResettable,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImExSum,
};

static const char *values_names[] = {
    "power",
    "energy_rel",
    "energy_abs",
};

void MetersLegacyAPI::pre_setup()
{
    state = Config::Object({
        {"writable", Config::Bool(false)},
    });

    config = Config::Object({
        {"linked_meter_slot", Config::Uint(0)}, // link to meter in slot 0 by default
    });
}

void MetersLegacyAPI::setup()
{
    initialized = true;

    api.restorePersistentConfig("meters_legacy_api/config", &config);

    linked_meter_slot = config.get("linked_meter_slot")->asUint();
    if (linked_meter_slot >= METERS_SLOTS) {
        if (linked_meter_slot != UINT32_MAX) {
            logger.printfln("Configured meter slot %u not available.", linked_meter_slot);
        }
        return;
    }

    legacy_api_enabled = true;

    const Config *config_prototype_float_nan  = Config::get_prototype_float_nan();
    const Config *config_prototype_bool_false = Config::get_prototype_bool_false();

    // BEGIN from old meter.cpp pre_setup()
    legacy_state = Config::Object({
        {"state", Config::Uint8(0)}, // 0 - no energy meter, 1 - initialization error, 2 - meter available
        {"type", Config::Uint8(0)} // 0 - not available, 1 - sdm72, 2 - sdm630, 3 - sdm72v2
    });

    legacy_values = Config::Object({
        {"power", Config::Float(NAN)},
        {"energy_rel", Config::Float(NAN)},
        {"energy_abs", Config::Float(NAN)},
    });

    legacy_phases = Config::Object({
        {"phases_connected", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            config_prototype_bool_false,
            3, 3, Config::type_id<Config::ConfBool>())},
        {"phases_active", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            config_prototype_bool_false,
            3, 3, Config::type_id<Config::ConfBool>())}
    });

    legacy_all_values = Config::Array({},
        config_prototype_float_nan,
        0, METER_ALL_VALUES_LEGACY_COUNT, Config::type_id<Config::ConfFloat>()
    );

    legacy_last_reset = Config::Object({
        {"last_reset", Config::Uint32(0)}
    });
    // END from old meter.cpp pre_setup()

    // BEGIN from old api_meter.cpp pre_setup()
    legacy_state_update = Config::Object({
        {"state", Config::Uint8(0)}, // 0 - no energy meter, 1 - initialization error, 2 - meter available
        {"type", Config::Uint8(0)} // 0 - not available, 1 - sdm72, 2 - sdm630, 3 - sdm72v2
    });

    legacy_values_update = Config::Object({
        {"power", Config::Float(0.0)},
        {"energy_rel", Config::Float(0.0)},
        {"energy_abs", Config::Float(0.0)},
    });

    legacy_phases_update = Config::Object({
        {"phases_connected", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            config_prototype_bool_false,
            3, 3, Config::type_id<Config::ConfBool>())},
        {"phases_active", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            config_prototype_bool_false,
            3, 3, Config::type_id<Config::ConfBool>())}
    });

    legacy_all_values_update = Config::Array({},
        config_prototype_float_nan,
        METER_ALL_VALUES_LEGACY_COUNT, METER_ALL_VALUES_LEGACY_COUNT, Config::type_id<Config::ConfFloat>());
    // END from old api_meter.cpp pre_setup()

    task_scheduler.scheduleOnce([this]() {
        this->show_blank_value_id_update_warnings = true;
    }, 250_ms);
}

void MetersLegacyAPI::register_urls()
{
    api.addState("meters_legacy_api/state", &state);
    api.addPersistentConfig("meters_legacy_api/config", &config);

    if (!legacy_api_enabled)
        return;

    api.addState("meter/state",      &legacy_state);
    api.addState("meter/values",     &legacy_values);
    api.addState("meter/phases",     &legacy_phases);
    api.addState("meter/all_values", &legacy_all_values);
    api.addState("meter/last_reset", &legacy_last_reset);
    // meter/error_counters registered in meters module

    api.addCommand("meter/reset", Config::Null(), {}, [this](String &errmsg) {
        errmsg = api.callCommand(meters.get_path(this->linked_meter_slot, Meters::PathType::Reset).c_str(), nullptr);
    }, true);

    api.addCommand("meter/state_update", &legacy_state_update, {}, [this](String &/*errmsg*/) {
        if (!this->meter_writable) {
            logger.printfln("Meter %u cannot be updated via the API. Only an API meter can be updated.", this->linked_meter_slot);
            return;
        }
        logger.printfln("Meter %u state cannot be updated. Change the meter's configuration instead.", this->linked_meter_slot);
    }, false);

    api.addCommand("meter/values_update", &legacy_values_update, {}, [this](String &/*errmsg*/) {
        if (!this->meter_writable) {
            logger.printfln("Meter %u cannot be updated via the API. Only an API meter can be updated.", this->linked_meter_slot);
            return;
        }

        for (size_t i = 0; i < 3; i++) {
            size_t target_index = value_indices_legacy_values_to_linked_meter[i];
            if (target_index >= this->linked_meter_value_count) {
                // Value not present in target.
                continue;
            }
            const char *field_name = values_names[i];
            float val = as_const(this->legacy_values_update).get(field_name)->asFloat();
            meters.update_value(this->linked_meter_slot, target_index, val);
        }

        meters.finish_update(this->linked_meter_slot);
    }, false);

    api.addCommand("meter/phases_update", &legacy_phases_update, {}, [this](String &/*errmsg*/) {
        if (!this->meter_writable) {
            logger.printfln("Meter %u cannot be updated via the API. Only an API meter can be updated.", this->linked_meter_slot);
            return;
        }
        if (!this->has_phases) {
            this->has_phases = true;
            api.addFeature("meter_phases");
        }
        this->phases_overridden = true;

        Config *conf_in  = static_cast<Config *>(legacy_phases_update.get("phases_active"));
        Config *conf_out = static_cast<Config *>(legacy_phases.get("phases_active"));
        for (size_t i = 0; i < 3; ++i) {
            conf_out->get(i)->updateBool(conf_in->get(i)->asBool());
        }

        conf_in  = static_cast<Config *>(legacy_phases_update.get("phases_connected"));
        conf_out = static_cast<Config *>(legacy_phases.get("phases_connected"));
        for (size_t i = 0; i < 3; ++i) {
            conf_out->get(i)->updateBool(conf_in->get(i)->asBool());
        }
    }, false);

    api.addCommand("meter/all_values_update", &legacy_all_values_update, {}, [this](String &/*errmsg*/) {
        if (!this->meter_writable) {
            logger.printfln("Meter %u cannot be updated via the API. Only an API meter can be updated.", this->linked_meter_slot);
            return;
        }

        if (this->linked_meter_value_count == 0) {
            logger.printfln("Cannot update meter %u that holds no values.", this->linked_meter_slot);
            return;
        }

        if (this->linked_meter_value_count > METERS_MAX_VALUES_PER_METER) {
            logger.printfln("Cannot update meter %u with too many many values (%u)", this->linked_meter_slot, this->linked_meter_value_count);
            return;
        }

        float values[METERS_MAX_VALUES_PER_METER]; // Always allocate max size to avoid VLA.

        // Pre-fill values with NaN because maybe not all of the target meter's values are available.
        float *values_end = values + this->linked_meter_value_count;
        do {
            *--values_end = NAN;
        } while (values_end > values);

        for (size_t source_index = 0; source_index < METER_ALL_VALUES_LEGACY_COUNT; source_index++) {
            size_t target_index = this->value_indices_legacy_all_values_to_linked_meter[source_index];
            if (target_index >= this->linked_meter_value_count) {
                // Value not present in target.
                continue;
            }

            values[target_index] = legacy_all_values_update.get(source_index)->asFloat();
        }

        meters.update_all_values(linked_meter_slot, values);
    }, false);
}

void MetersLegacyAPI::register_events()
{
    if (!legacy_api_enabled)
        return;

    if (linked_meter_slot >= METERS_SLOTS)
        return;

    String value_ids_path = meters.get_path(linked_meter_slot, Meters::PathType::ValueIDs);

    const Config *old_value_ids = api.getState(value_ids_path);
    if (old_value_ids->count() > 0) {
        on_value_ids_change(old_value_ids);
    } else {
        event.registerEvent(value_ids_path, {}, [this](const Config *event_value_ids) {
            return on_value_ids_change(event_value_ids);
        });
    }
}

uint32_t MetersLegacyAPI::get_linked_meter_slot()
{
    return linked_meter_slot;
}

static void fill_index_array(uint16_t *indices, const MeterValueID *needles, uint32_t needle_count, const MeterValueID *haystack, uint32_t haystack_size)
{
    for (uint32_t needle_i = 0; needle_i < needle_count; needle_i++) {
        MeterValueID needle = needles[needle_i];
        indices[needle_i] = UINT16_MAX;
        for (uint16_t haystack_i = 0; haystack_i < haystack_size; haystack_i++) {
            if (needle == haystack[haystack_i]) {
                indices[needle_i] = haystack_i;
                break;
            }
        }
    }
}

static bool is_values_value(MeterValueID value_id)
{
    for (size_t i = 0; i < ARRAY_SIZE(legacy_values_ids); i++) {
        if (value_id == legacy_values_ids[i])
            return true;
    }
    return false;
}

static bool indices_match_meter_indices(const bool all_values_present[], const uint32_t all_value_indices[], uint32_t all_value_indices_length)
{
    bool values_present[METER_ALL_VALUES_LEGACY_COUNT] = {false};
    for (uint32_t i = 0; i < all_value_indices_length; i++) {
        uint32_t index = all_value_indices[i];
        if (index < ARRAY_SIZE(values_present))
            values_present[index] = true;
    }

    for (uint32_t i = 0; i < METER_ALL_VALUES_LEGACY_COUNT; i++) {
        if (values_present[i] != all_values_present[i]) {
            return false;
        }
    }

    return true;
}

EventResult MetersLegacyAPI::on_value_ids_change(const Config *value_ids)
{
    // ==== Fill index arrays ====

    auto cnt = value_ids->count();
    if (cnt == 0) {
        if (show_blank_value_id_update_warnings) {
            logger.printfln("Ignoring blank value IDs update from linked meter in slot %u.", linked_meter_slot);
        }
        return EventResult::OK;
    }

    linked_meter_value_count = cnt;
    meter_setup_done = true;

    if (linked_meter_value_count > METERS_MAX_VALUES_PER_METER) {
        logger.printfln("Linked meter has too many values (%u)", linked_meter_value_count);
        return EventResult::Deregister;
    }

    MeterValueID meter_value_ids[METERS_MAX_VALUES_PER_METER]; // Always allocate max size to avoid VLA.
    for (size_t i = 0; i < linked_meter_value_count; i++) {
        meter_value_ids[i] = static_cast<MeterValueID>(value_ids->get(i)->asUint());
    }

    fill_index_array(value_indices_legacy_all_values_to_linked_meter, sdm_helper_all_ids, METER_ALL_VALUES_LEGACY_COUNT, meter_value_ids, linked_meter_value_count);

    // Get power index
    fill_index_array(value_indices_legacy_values_to_linked_meter, legacy_values_ids, 1, meter_value_ids, linked_meter_value_count);

    uint16_t value_indices_import_imex[ARRAY_SIZE(value_ids_import_imex)];
    fill_index_array(value_indices_import_imex, value_ids_import_imex, ARRAY_SIZE(value_ids_import_imex), meter_value_ids, linked_meter_value_count);

    bool use_imexsum = false;
#if MODULE_EVSE_COMMON_AVAILABLE()
    use_imexsum = evse_common.get_use_imexsum();
#endif

    if (use_imexsum) {
        if (value_indices_import_imex[2] != UINT16_MAX) {
            value_indices_legacy_values_to_linked_meter[1] = value_indices_import_imex[2];
        } else {
            value_indices_legacy_values_to_linked_meter[1] = value_indices_import_imex[0];
        }

        if (value_indices_import_imex[3] != UINT16_MAX) {
            value_indices_legacy_values_to_linked_meter[2] = value_indices_import_imex[3];
        } else {
            value_indices_legacy_values_to_linked_meter[2] = value_indices_import_imex[1];
        }
    } else {
            value_indices_legacy_values_to_linked_meter[1] = value_indices_import_imex[0];
            value_indices_legacy_values_to_linked_meter[2] = value_indices_import_imex[1];
    }

    if (value_indices_legacy_values_to_linked_meter[2] == UINT16_MAX && value_indices_legacy_values_to_linked_meter[1] != UINT16_MAX) {
        value_indices_legacy_values_to_linked_meter[2] = value_indices_legacy_values_to_linked_meter[1];
        logger.printfln("Meter in slot %u doesn't provide energy_abs; copying energy_rel instead.", linked_meter_slot);
    }


    // ==== Meter type detection ====

    bool all_values_present[METER_ALL_VALUES_LEGACY_COUNT];
    bool has_any_known_value = false;
    uint32_t can_be_sdm72;
    uint32_t can_be_sdm72v2;
    uint32_t can_be_sdm630;

    for (uint32_t i = 0; i < METER_ALL_VALUES_LEGACY_COUNT; i++) {
        size_t value_index = value_indices_legacy_all_values_to_linked_meter[i];
        if (value_index >= linked_meter_value_count) {
            // Linked meter doesn't have this value.
            all_values_present[i] = false;
        } else {
            // Linked meter has this value.
            all_values_present[i] = true;
            has_any_known_value = true;

            MeterValueID value_id = meter_value_ids[value_index];
            if (!is_values_value(value_id)) {
                has_all_values = true;
            }
        }
    }

    can_be_sdm72   = indices_match_meter_indices(all_values_present, sdm_helper_72_all_value_indices,   ARRAY_SIZE(sdm_helper_72_all_value_indices));
    can_be_sdm72v2 = indices_match_meter_indices(all_values_present, sdm_helper_72v2_all_value_indices, ARRAY_SIZE(sdm_helper_72v2_all_value_indices));
    can_be_sdm630  = indices_match_meter_indices(all_values_present, sdm_helper_630_all_value_indices,  ARRAY_SIZE(sdm_helper_630_all_value_indices));

    uint32_t can_be_count = can_be_sdm72 + can_be_sdm72v2 + can_be_sdm630;
    uint32_t meter_type = METER_TYPE_NONE;
    if (can_be_count == 0) { // Not a known SDM meter.
        if (has_any_known_value) {
            if (has_all_values) {
                meter_type = METER_TYPE_CUSTOM_ALL_VALUES;
            } else {
                meter_type = METER_TYPE_CUSTOM_BASIC;
            }
        } else {
            // Doesn't have any known value, values and all_values are empty.
        }
    } else if (can_be_count == 1) { // One known SDM matched.
        if (can_be_sdm72) {
            meter_type = METER_TYPE_SDM72DM;
        } else if (can_be_sdm72v2) {
            meter_type = METER_TYPE_SDM72DMV2;
        } else if (can_be_sdm630) {
            meter_type = METER_TYPE_SDM630;
        }
    } else {
        logger.printfln("Meter detection failed: %u matching meters. 72:%u 72v2:%u 630:%u", can_be_count, can_be_sdm72, can_be_sdm72v2, can_be_sdm630);
    }

    MeterClassID linked_meter_class = meters.get_meter_class(linked_meter_slot);
    if (linked_meter_class == MeterClassID::RS485Bricklet
        || linked_meter_class == MeterClassID::EVSEV2
        || linked_meter_class == MeterClassID::EnergyManager) {
        const String state_path = meters.get_path(linked_meter_slot, Meters::PathType::State);
        const Config *linked_state = api.getState(state_path);
        if (linked_state) {
            uint32_t local_meter_type = linked_state->get("type")->asUint();
            if (meter_type != local_meter_type) {
                if (local_meter_type != METER_TYPE_DSZ15DZMOD) { // Known undetectable
                    logger.printfln("Meter type %u from Bricklet overrides auto-detected meter type %u.", local_meter_type, meter_type);
                }
                meter_type = local_meter_type;
            }
        } else {
            logger.printfln("Expected state %s not found.", state_path.c_str());
        }
    }

    if (meter_type == METER_TYPE_NONE) {
        logger.printfln("Meter type detection failed. 72=%u 72v2=%u 630=%u", can_be_sdm72, can_be_sdm72v2, can_be_sdm630);
        legacy_state.get("state")->updateUint(1); // 1 - initialization error
        return EventResult::Deregister;
    }


    // ==== Detect phases support ====

    has_phases = all_values_present[METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L1] &
                 all_values_present[METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L2] &
                 all_values_present[METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L3] &
                 all_values_present[METER_ALL_VALUES_CURRENT_L1_A] &
                 all_values_present[METER_ALL_VALUES_CURRENT_L2_A] &
                 all_values_present[METER_ALL_VALUES_CURRENT_L3_A];


    // ==== Get values and set up event handler ====

    if (has_all_values) {
        for (size_t i = legacy_all_values.count(); i < METER_ALL_VALUES_LEGACY_COUNT; ++i) {
            legacy_all_values.add();
        }
    }

    String values_path = meters.get_path(linked_meter_slot, Meters::PathType::Values);

    const Config *old_values = api.getState(values_path);
    if (old_values->count() > 0) {
        on_values_change(old_values);
    }

    event.registerEvent(values_path, {}, [this](const Config *event_values) {
        on_values_change(event_values);
        return EventResult::OK;
    });


    // ==== Check reset support ====

    String last_reset_path = meters.get_path(linked_meter_slot, Meters::PathType::LastReset);
    const Config *last_reset_config = api.getState(last_reset_path, false);
    if (last_reset_config) {
        on_last_reset_change(last_reset_config);

        event.registerEvent(last_reset_path, {}, [this](const Config *event_last_reset) {
            on_last_reset_change(event_last_reset);
            return EventResult::OK;
        });
    }


    // ==== Set states and features ====

    legacy_state.get("state")->updateUint(2); // 2 - meter available
    legacy_state.get("type" )->updateUint(meter_type);

    api.addFeature("meter");

    if (has_all_values) {
        api.addFeature("meter_all_values");
    }

    if (has_phases) {
        api.addFeature("meter_phases");
    }

    if (linked_meter_class == MeterClassID::API) {
        state.get("writable")->updateBool(true);
        meter_writable = true;
    }

    return EventResult::Deregister;
}

static void update_config_values(uint16_t *indices, uint16_t index_count, const Config *source_values, Config *target_values)
{
    bool needs_values_helper = target_values->is<Config::ConfObject>() && index_count == 3;

    size_t source_count = source_values->count();
    size_t target_count = needs_values_helper ? 3 : target_values->count();

    if (target_count != index_count) {
        logger.printfln("Cannot update config values, count mismatch: %zu vs %zu", target_count, index_count);
        return;
    }

    for (size_t target_index = 0; target_index < target_count; target_index++) {
        size_t source_index = indices[target_index];
        if (source_index >= source_count) {
            // Value not available in source.
            continue;
        }

        Config *target_config;

        if (needs_values_helper) {
            const char *field_name = values_names[target_index];
            target_config = static_cast<Config *>(target_values->get(field_name));
        } else {
            target_config = static_cast<Config *>(target_values->get(target_index));
        }

        float source_value = source_values->get(source_index)->asFloat();
        target_config->updateFloat(source_value);
    }
}

void MetersLegacyAPI::on_values_change(const Config *values)
{
    update_config_values(value_indices_legacy_values_to_linked_meter, ARRAY_SIZE(value_indices_legacy_values_to_linked_meter), values, &legacy_values);

    if (has_all_values) {
        update_config_values(value_indices_legacy_all_values_to_linked_meter, ARRAY_SIZE(value_indices_legacy_all_values_to_linked_meter), values, &legacy_all_values);
    }

    if (has_phases && !phases_overridden) {
        auto *phases_connected = static_cast<Config *>(legacy_phases.get("phases_connected"));
        for (size_t i = 0; i < 3; i++) {
            float value = values->get(value_indices_legacy_all_values_to_linked_meter[METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L1 + i])->asFloat();
            phases_connected->get(i)->updateBool(value > PHASE_CONNECTED_VOLTAGE_THRES);
        }
        auto *phases_active = static_cast<Config *>(legacy_phases.get("phases_active"));
        for (size_t i = 0; i < 3; i++) {
            float value = values->get(value_indices_legacy_all_values_to_linked_meter[METER_ALL_VALUES_CURRENT_L1_A + i])->asFloat();
            phases_active->get(i)->updateBool(value > PHASE_ACTIVE_CURRENT_THRES);
        }
    }
}

void MetersLegacyAPI::on_last_reset_change(const Config *last_reset)
{
    legacy_last_reset.get("last_reset")->updateUint(last_reset->get("last_reset")->asUint());
}
