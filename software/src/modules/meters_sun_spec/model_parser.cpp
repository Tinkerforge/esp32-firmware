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

#include "model_parser.h"

#include <stdlib.h>
#include <string.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "options.h"
#include "model_parser_160.h"
#include "model_parser_714.h"

#include "gcc_warnings.h"

IMetersSunSpecParser *MetersSunSpecParser::new_parser(uint32_t slot, const char *manufacturer_name, uint16_t model_id, DCPortType dc_port_type)
{
    if (model_id == 160) {
        return new MetersSunSpecParser160(slot);
    }

    if (model_id == 714) {
        if (dc_port_type == DCPortType::NotImplemented) {
            logger.printfln_meter("No DC Port Type selected");
            return nullptr;
        }

        return new MetersSunSpecParser714(slot, dc_port_type);
    }

    for (size_t i = 0; i < meters_sun_spec_all_model_data.model_count; i++) {
        const ModelData *model_data = meters_sun_spec_all_model_data.model_data[i];

        if (model_data->model_id == model_id) {
            const ModelData *actual_model_data = model_data;

            // WattNode meter reports phase currents as ImExDiff
            if (strcmp(manufacturer_name, "WattNode") == 0 && model_id >= 200 && model_id < 300) {
                logger.printfln_meter("Mapping phase currents as ImExDiff for WattNode meter");

                size_t model_data_length = sizeof(ModelData) + sizeof(ValueData) * model_data->value_count;

                // FIXME: leaking this, as meter instances are not destroyed
                ModelData *patched_model_data = static_cast<ModelData *>(malloc(model_data_length));
                memcpy(patched_model_data, model_data, model_data_length);

                for (size_t k = 0; k < patched_model_data->value_count; ++k) {
                    MeterValueID value_id = patched_model_data->value_data[k].value_id;

                    if      (value_id == MeterValueID::CurrentL1ImExSum)   value_id = MeterValueID::CurrentL1ImExDiff;
                    else if (value_id == MeterValueID::CurrentL2ImExSum)   value_id = MeterValueID::CurrentL2ImExDiff;
                    else if (value_id == MeterValueID::CurrentL3ImExSum)   value_id = MeterValueID::CurrentL3ImExDiff;
                    else if (value_id == MeterValueID::CurrentLSumImExSum) value_id = MeterValueID::CurrentLSumImExDiff;

                    patched_model_data->value_data[k].value_id = value_id;
                }

                actual_model_data = patched_model_data;
            }

            return new MetersSunSpecParser(slot, actual_model_data);
        }
    }

    return nullptr;
}

bool MetersSunSpecParser::detect_values(const uint16_t *const register_data[2], uint32_t quirks, size_t *registers_to_read)
{
    if (!model->validator(register_data))
        return false;

    const uint16_t *data = model->read_twice ? register_data[1] : register_data[0];
    uint8_t max_register = 0;

    detected_values.reserve(model->value_count);

    for (size_t i = 0; i < model->value_count; i++) {
        const ValueData *value_data = &model->value_data[i];
        if (!model->is_meter || !isnan(value_data->get_value(data, quirks, true))) {
            detected_values.push_back(value_data);

            if (value_data->max_register > max_register) {
                max_register = value_data->max_register;
            }
        }
    }

    *registers_to_read = static_cast<uint32_t>(max_register) + 1;

    detected_values.shrink_to_fit();
    size_t detected_value_count = detected_values.size();

    MeterValueID *ids = static_cast<MeterValueID *>(malloc(detected_value_count * sizeof(MeterValueID)));
    for (uint16_t i = 0; i < detected_value_count; i++) {
        ids[i] = detected_values[i]->value_id;
    }
    meters.declare_value_ids(meter_slot, ids, detected_value_count);
    free(ids);

    return true;
}

bool MetersSunSpecParser::parse_values(const uint16_t *const register_data[2], uint32_t quirks)
{
    if (!model->validator(register_data))
        return false;

    const uint16_t *data = model->read_twice ? register_data[1] : register_data[0];

    float meter_values[OPTIONS_METERS_MAX_VALUES_PER_METER()];
    size_t value_count = detected_values.size();
    for (size_t i = 0; i < value_count; i++) {
        meter_values[i] = detected_values[i]->get_value(data, quirks, false);
    }
    meters.update_all_values(meter_slot, meter_values);
    return true;
}

bool MetersSunSpecParser::must_read_twice()
{
    return model->read_twice;
}

bool MetersSunSpecParser::is_model_length_supported(uint32_t model_length)
{
    return model_length == model->model_length;
}

uint32_t MetersSunSpecParser::get_interesting_registers_count()
{
    return model->interesting_registers_count;
}
