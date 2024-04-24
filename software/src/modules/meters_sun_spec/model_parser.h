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

#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "modules/meters/meter_value_id.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    //#include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

#define SUN_SPEC_QUIRKS_ACC32_IS_INT32                         (1u << 0)
#define SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16              (1u << 1)
#define SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY    (1u << 2)
#define SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED               (1u << 3)
#define SUN_SPEC_QUIRKS_INTEGER_INVERTER_POWER_FACTOR_IS_UNITY (1u << 4)

class MetersSunSpecParser
{
public:
    typedef float (*get_value_fn)(const void *register_data, uint32_t quirks, bool detection);
    typedef bool (*model_validator_fn)(const uint16_t *const register_data[2]);

    struct ValueData {
        get_value_fn get_value;
        MeterValueID value_id;
        uint8_t max_register;
    };

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif
    struct ModelData {
        uint16_t model_id;
        uint16_t model_length; // as specified in the model length register, excludes model ID and model length
        uint16_t interesting_registers_count; // amount of interesting registers, including model ID and model length
        bool is_meter;
        bool read_twice;
        model_validator_fn validator;
        size_t value_count;
        ValueData value_data[]; // ISO C++ forbids flexible array members
    };

    struct AllModelData {
        size_t model_count;
        const ModelData *model_data[]; // ISO C++ forbids flexible array members
    };
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

    static MetersSunSpecParser *new_parser(uint32_t meter_slot, uint16_t model_id);

    bool detect_values(const uint16_t *const register_data[2], uint32_t quirks, size_t *registers_to_read);
    bool parse_values(const uint16_t *const register_data[2], uint32_t quirks);

    bool must_read_twice();
    uint32_t get_model_length();
    uint32_t get_interesting_registers_count();

private:
    MetersSunSpecParser() : meter_slot(0), model(nullptr) {}
    MetersSunSpecParser(uint32_t meter_slot_, const ModelData *model_) : meter_slot(meter_slot_), model(model_) {}

    const uint32_t meter_slot;
    const ModelData *const model;
    std::vector<const ValueData *> detected_values;
    float *meter_values;
};

extern const MetersSunSpecParser::AllModelData meters_sun_spec_all_model_data;

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
