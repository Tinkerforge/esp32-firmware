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

#include "sdm_helpers.h"

#include <string.h>

#include "event_log.h"
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
    MeterValueID::PowerApparentL1,
    MeterValueID::PowerApparentL2,
    MeterValueID::PowerApparentL3,
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
    MeterValueID::PowerApparentLSum,
    MeterValueID::PowerReactiveLSumIndCapDiff,
    MeterValueID::PowerFactorLSumDirectional,
    MeterValueID::PhaseAngleLAvg,
    MeterValueID::FrequencyLAvg,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::EnergyReactiveLSumInductive,
    MeterValueID::EnergyReactiveLSumCapacitive,
    MeterValueID::EnergyApparentLSumImExSum,
    MeterValueID::ElectricCharge,
    MeterValueID::PowerActiveLSumImExDiffIntervalAvg,
    MeterValueID::PowerActiveLSumImExDiffIntervalMax,
    MeterValueID::PowerApparentLSumIntervalAvg,
    MeterValueID::PowerApparentLSumIntervalMax,
    MeterValueID::CurrentNImExSumIntervalAvg,
    MeterValueID::CurrentNImExSumIntervalMax,
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
    MeterValueID::CurrentL1ImExSumIntervalAvg,
    MeterValueID::CurrentL2ImExSumIntervalAvg,
    MeterValueID::CurrentL3ImExSumIntervalAvg,
    MeterValueID::CurrentL1ImExSumIntervalMax,
    MeterValueID::CurrentL2ImExSumIntervalMax,
    MeterValueID::CurrentL3ImExSumIntervalMax,
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

const MeterValueID sdm_helper_72v1_ids[7] = {
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::EnergyActiveLSumImExSum,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImportResettable,
    MeterValueID::EnergyActiveLSumExportResettable
};

const uint32_t sdm_helper_72v2_all_value_indices[38] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,21,22,23,24,25,26,27,29,30,31,42,43,44,45,46,65,66,85,86,87};
const uint32_t sdm_helper_72_all_value_indices[7] = {24,30,31,65,85,86,87};

static void copy_value_ids(MeterValueID *dst, const MeterValueID *src, size_t *dst_len, size_t src_len)
{
    if (*dst_len < src_len) {
        logger.printfln("sdm_helpers: Passed array of length %u too short for %u value IDs.", *dst_len, src_len);
        *dst_len = 0;
        return;
    }
    memcpy(dst, src, src_len * sizeof(MeterValueID));
    *dst_len = src_len;
}

void sdm_helper_get_value_ids(uint32_t meter_type, MeterValueID *value_ids, size_t *value_ids_len)
{
    switch (meter_type) {
        case METER_TYPE_SDM630:
        case METER_TYPE_SDM630MCTV2:
            copy_value_ids(value_ids, sdm_helper_all_ids, value_ids_len, ARRAY_SIZE(sdm_helper_all_ids));
            break;

        case METER_TYPE_SDM72DM:
            copy_value_ids(value_ids, sdm_helper_72v1_ids, value_ids_len, ARRAY_SIZE(sdm_helper_72v1_ids));
            break;

        case METER_TYPE_SDM72DMV2: {
            size_t id_count = ARRAY_SIZE(sdm_helper_72v2_all_value_indices);
            if (*value_ids_len < id_count) {
                logger.printfln("sdm_helpers: Passed array of length %u too short for %u value IDs for an SDM72v2.", *value_ids_len, id_count);
                *value_ids_len = 0;
                break;
            }
            for (size_t i = 0; i < id_count; i++) {
                size_t src_i = sdm_helper_72v2_all_value_indices[i];
                value_ids[i] = sdm_helper_all_ids[src_i];
            }
            *value_ids_len = id_count;
            break;
        }
        default:
            logger.printfln("sdm_helpers: Value IDs unsupported for meter type %u.", meter_type);
            *value_ids_len = 0;
            break;
    }
}

void sdm_helper_pack_all_values(uint32_t meter_type, float *values, size_t *values_len)
{
    switch (meter_type) {
        case METER_TYPE_SDM630:
        case METER_TYPE_SDM630MCTV2:
            // Nothing to pack.
            break;

        case METER_TYPE_SDM72DMV2: {
            size_t values_count = ARRAY_SIZE(sdm_helper_72v2_all_value_indices);
            if (*values_len < values_count) {
                logger.printfln("sdm_helpers: Not enough space to pack %u 72v2 values into an array of size %u.", values_count, *values_len);
                *values_len = 0;
                break;
            }
            for (size_t i = 0; i < values_count; i++) {
                size_t src_i = sdm_helper_72v2_all_value_indices[i];
                values[i] = values[src_i];
            }
            *values_len = values_count;
            break;
        }
        default:
            logger.printfln("sdm_helpers: Cannot pack values for meter type %u.", meter_type);
            *values_len = 0;
            break;
    }
}
