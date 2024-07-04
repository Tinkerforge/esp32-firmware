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

#include "module_dependencies.h"

#include "gcc_warnings.h"

MetersSunSpecParser *MetersSunSpecParser::new_parser(uint32_t meter_slot, uint16_t model_id)
{
    for (size_t i = 0; i < meters_sun_spec_all_model_data.model_count; i++) {
        auto *model_data = meters_sun_spec_all_model_data.model_data[i];
        if (model_data->model_id == model_id) {
            return new MetersSunSpecParser(meter_slot, model_data);
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

    meter_values = static_cast<float *>(malloc(detected_value_count * sizeof(float)));
    return true;
}

bool MetersSunSpecParser::parse_values(const uint16_t *const register_data[2], uint32_t quirks)
{
    if (!model->validator(register_data))
        return false;

    const uint16_t *data = model->read_twice ? register_data[1] : register_data[0];

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

uint32_t MetersSunSpecParser::get_model_length()
{
    return model->model_length;
}

uint32_t MetersSunSpecParser::get_interesting_registers_count()
{
    return model->interesting_registers_count;
}
