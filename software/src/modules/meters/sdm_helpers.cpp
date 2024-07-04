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

#define EVENT_LOG_PREFIX "sdm_helpers"

#include "sdm_helpers.h"

#include <string.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "gcc_warnings.h"

const MeterValueID sdm_helper_all_ids[METER_ALL_VALUES_RESETTABLE_COUNT] = {
    MeterValueID::VoltageL1N,
    MeterValueID::VoltageL2N,
    MeterValueID::VoltageL3N,
    MeterValueID::CurrentL1ImExSum,
    MeterValueID::CurrentL2ImExSum,
    MeterValueID::CurrentL3ImExSum,
    MeterValueID::PowerActiveL1ImExDiff,
    MeterValueID::PowerActiveL2ImExDiff,
    MeterValueID::PowerActiveL3ImExDiff,
    MeterValueID::PowerApparentL1ImExSum,
    MeterValueID::PowerApparentL2ImExSum,
    MeterValueID::PowerApparentL3ImExSum,
    MeterValueID::PowerReactiveL1IndCapDiff,
    MeterValueID::PowerReactiveL2IndCapDiff,
    MeterValueID::PowerReactiveL3IndCapDiff,
    MeterValueID::PowerFactorL1Directional,
    MeterValueID::PowerFactorL2Directional,
    MeterValueID::PowerFactorL3Directional,
    MeterValueID::PhaseAngleL1,
    MeterValueID::PhaseAngleL2,
    MeterValueID::PhaseAngleL3,
    MeterValueID::VoltageLNAvg,
    MeterValueID::CurrentLAvgImExSum,
    MeterValueID::CurrentLSumImExSum,
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::PowerApparentLSumImExSum,
    MeterValueID::PowerReactiveLSumIndCapDiff,
    MeterValueID::PowerFactorLSumDirectional,
    MeterValueID::PhaseAngleLSum,
    MeterValueID::FrequencyLAvg,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::EnergyReactiveLSumInductive,
    MeterValueID::EnergyReactiveLSumCapacitive,
    MeterValueID::EnergyApparentLSumImExSum,
    MeterValueID::ElectricCharge,
    MeterValueID::NotSupported,
    MeterValueID::NotSupported,
    MeterValueID::NotSupported,
    MeterValueID::NotSupported,
    MeterValueID::NotSupported,
    MeterValueID::NotSupported,
    MeterValueID::VoltageL1L2,
    MeterValueID::VoltageL2L3,
    MeterValueID::VoltageL3L1,
    MeterValueID::VoltageLLAvg,
    MeterValueID::CurrentNImExSum,
    MeterValueID::VoltageTHDL1N,
    MeterValueID::VoltageTHDL2N,
    MeterValueID::VoltageTHDL3N,
    MeterValueID::CurrentTHDL1,
    MeterValueID::CurrentTHDL2,
    MeterValueID::CurrentTHDL3,
    MeterValueID::VoltageTHDLNAvg,
    MeterValueID::CurrentTHDLAvg,
    MeterValueID::NotSupported,
    MeterValueID::NotSupported,
    MeterValueID::NotSupported,
    MeterValueID::NotSupported,
    MeterValueID::NotSupported,
    MeterValueID::NotSupported,
    MeterValueID::VoltageTHDL1L2,
    MeterValueID::VoltageTHDL2L3,
    MeterValueID::VoltageTHDL3L1,
    MeterValueID::VoltageTHDLLAvg,
    MeterValueID::EnergyActiveLSumImExSum,
    MeterValueID::EnergyReactiveLSumIndCapSum,
    MeterValueID::EnergyActiveL1Import,
    MeterValueID::EnergyActiveL2Import,
    MeterValueID::EnergyActiveL3Import,
    MeterValueID::EnergyActiveL1Export,
    MeterValueID::EnergyActiveL2Export,
    MeterValueID::EnergyActiveL3Export,
    MeterValueID::EnergyActiveL1ImExSum,
    MeterValueID::EnergyActiveL2ImExSum,
    MeterValueID::EnergyActiveL3ImExSum,
    MeterValueID::EnergyReactiveL1Inductive,
    MeterValueID::EnergyReactiveL2Inductive,
    MeterValueID::EnergyReactiveL3Inductive,
    MeterValueID::EnergyReactiveL1Capacitive,
    MeterValueID::EnergyReactiveL2Capacitive,
    MeterValueID::EnergyReactiveL3Capacitive,
    MeterValueID::EnergyReactiveL1IndCapSum,
    MeterValueID::EnergyReactiveL2IndCapSum,
    MeterValueID::EnergyReactiveL3IndCapSum,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImportResettable,
    MeterValueID::EnergyActiveLSumExportResettable,
};

const uint32_t sdm_helper_630_all_value_indices[76]  = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,42,43,44,45,46,47,48,49,50,51,52,53,54,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87};
const uint32_t sdm_helper_72v2_all_value_indices[38] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,21,22,23,24,25,26,27,29,30,31,42,43,44,45,46,65,66,85,86,87};
const uint32_t sdm_helper_72_all_value_indices[7]    = {24,30,31,65,85,86,87};

void sdm_helper_get_value_ids(uint32_t meter_type, MeterValueID *value_ids, size_t *value_ids_len)
{
    const uint32_t *all_value_indices;
    size_t id_count = 0;

    switch (meter_type) {
        case METER_TYPE_SDM630:
        case METER_TYPE_SDM630MCTV2:
            all_value_indices = sdm_helper_630_all_value_indices;
            id_count = ARRAY_SIZE(sdm_helper_630_all_value_indices);
            break;

        case METER_TYPE_SDM72DMV2:
            all_value_indices = sdm_helper_72v2_all_value_indices;
            id_count = ARRAY_SIZE(sdm_helper_72v2_all_value_indices);
            break;

        case METER_TYPE_SDM72DM:
            all_value_indices = sdm_helper_72_all_value_indices;
            id_count = ARRAY_SIZE(sdm_helper_72_all_value_indices);
            break;

        default:
            logger.printfln("Value IDs unsupported for meter type %u.", meter_type);
            /* FALLTHROUGH */
        case METER_TYPE_DSZ15DZMOD: // Known unsupported
            *value_ids_len = 0;
            return;
    }

    if (*value_ids_len < id_count) {
        logger.printfln("Passed array of length %u too short for %u value IDs for meter type %u.", *value_ids_len, id_count, meter_type);
        *value_ids_len = 0;
        return;
    }

    for (size_t i = 0; i < id_count; i++) {
        size_t src_i = all_value_indices[i];
        value_ids[i] = sdm_helper_all_ids[src_i];
    }
    *value_ids_len = id_count;
}

void sdm_helper_parse_values(uint32_t meter_type, float all_values[METER_ALL_VALUES_RESETTABLE_COUNT], size_t *value_count, MeterValueID *value_ids, uint8_t *packing_cache)
{
    size_t i_out = 0;
    for (size_t i_in = 0; i_in < METER_ALL_VALUES_RESETTABLE_COUNT; i_in++) {
        if (isnan(all_values[i_in]))
            continue;

        if (i_out >= *value_count) {
            logger.printfln("Not enough space for all value IDs: %u", *value_count);
            break;
        }

        MeterValueID value_id = sdm_helper_all_ids[i_in];
        if (value_id == MeterValueID::NotSupported)
            continue;

        value_ids[i_out] = value_id;
        packing_cache[i_out] = static_cast<uint8_t>(i_in);
        i_out++;
    }

    *value_count = i_out;

    if (meter_type != METER_TYPE_NONE && meter_type < 100) {
        MeterValueID static_value_ids[METER_ALL_VALUES_RESETTABLE_COUNT];
        size_t value_ids_len = METER_ALL_VALUES_RESETTABLE_COUNT;
        sdm_helper_get_value_ids(meter_type, static_value_ids, &value_ids_len);

        if (i_out != value_ids_len) {
            if (!value_ids_len) {
                if (meter_type != METER_TYPE_DSZ15DZMOD) { // Validating this meter type is known unsupported
                    logger.printfln("Can't validate value IDs for meter type %u.", meter_type);
                }
            } else {
                logger.printfln("Value ID count mismatch for meter type %u: detected %u but expected %u.", meter_type, i_out, value_ids_len);
            }
        } else {
            for (size_t i = 0; i < value_ids_len; i++) {
                if (value_ids[i] != static_value_ids[i]) {
                    logger.printfln("Value ID mismatch at position %u for meter type %u: detected %u but expected %u.", i, meter_type, static_cast<uint32_t>(value_ids[i]), static_cast<uint32_t>(static_value_ids[i]));
                }
            }
        }
    }
}

void sdm_helper_pack_all_values(uint32_t meter_type, float *values, size_t *values_len)
{
    const uint32_t *all_value_indices;
    size_t values_count = 0;

    switch (meter_type) {
        case METER_TYPE_SDM630:
        case METER_TYPE_SDM630MCTV2:
            all_value_indices = sdm_helper_630_all_value_indices;
            values_count = ARRAY_SIZE(sdm_helper_630_all_value_indices);
            break;

        case METER_TYPE_SDM72DMV2:
            all_value_indices = sdm_helper_72v2_all_value_indices;
            values_count = ARRAY_SIZE(sdm_helper_72v2_all_value_indices);
            break;

        case METER_TYPE_SDM72DM:
            all_value_indices = sdm_helper_72_all_value_indices;
            values_count = ARRAY_SIZE(sdm_helper_72_all_value_indices);
            break;

        default:
            logger.printfln("Cannot pack values for meter type %u.", meter_type);
            *values_len = 0;
            return;
    }

    if (*values_len < values_count) {
        logger.printfln("Not enough space to pack %u values for meter type %u into an array of size %u.", values_count, meter_type, *values_len);
        *values_len = 0;
        return;
    }

    for (size_t i = 0; i < values_count; i++) {
        size_t src_i = all_value_indices[i];
        values[i] = values[src_i];
    }
    *values_len = values_count;
}

void sdm_helper_pack_all_values(float *values, size_t values_len, uint8_t *packing_cache)
{
    for (size_t i = 0; i < values_len; i++) {
        uint8_t source_index = packing_cache[i];
        values[i] = values[source_index];
    }
}
