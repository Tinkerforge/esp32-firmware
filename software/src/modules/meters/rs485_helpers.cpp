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

#define EVENT_LOG_PREFIX "rs485_helpers"

#include "rs485_helpers.h"

#include <string.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "gcc_warnings.h"

const MeterValueID rs485_helper_sdm630_ids[] = {
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

const MeterValueID rs485_helper_sdm72v2_ids[] = {
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
    MeterValueID::VoltageLNAvg,
    MeterValueID::CurrentLAvgImExSum,
    MeterValueID::CurrentLSumImExSum,
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::PowerApparentLSumImExSum,
    MeterValueID::PowerReactiveLSumIndCapDiff,
    MeterValueID::PowerFactorLSumDirectional,
    MeterValueID::FrequencyLAvg,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::VoltageL1L2,
    MeterValueID::VoltageL2L3,
    MeterValueID::VoltageL3L1,
    MeterValueID::VoltageLLAvg,
    MeterValueID::CurrentNImExSum,
    MeterValueID::EnergyActiveLSumImExSum,
    MeterValueID::EnergyReactiveLSumIndCapSum,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImportResettable,
    MeterValueID::EnergyActiveLSumExportResettable,
};

const MeterValueID rs485_helper_sdm72_ids[] = {
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::EnergyActiveLSumImExSum,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImportResettable,
    MeterValueID::EnergyActiveLSumExportResettable,
};

const MeterValueID rs485_helper_dsz15dzmod_ids[] = {
    MeterValueID::VoltageL1N,
    MeterValueID::VoltageL2N,
    MeterValueID::VoltageL3N,
    MeterValueID::CurrentL1ImExSum,
    MeterValueID::CurrentL2ImExSum,
    MeterValueID::CurrentL3ImExSum,
    MeterValueID::PowerActiveL1ImExDiff,
    MeterValueID::PowerActiveL2ImExDiff,
    MeterValueID::PowerActiveL3ImExDiff,
    MeterValueID::PowerFactorL1Directional,
    MeterValueID::PowerFactorL2Directional,
    MeterValueID::PowerFactorL3Directional,
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::PowerFactorLSumDirectional,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::PowerApparentL1ImExSum,
    MeterValueID::PowerApparentL2ImExSum,
    MeterValueID::PowerApparentL3ImExSum,
    MeterValueID::PowerReactiveL1IndCapDiff,
    MeterValueID::PowerReactiveL2IndCapDiff,
    MeterValueID::PowerReactiveL3IndCapDiff,
    MeterValueID::PhaseAngleL1,
    MeterValueID::PhaseAngleL2,
    MeterValueID::PhaseAngleL3,
    MeterValueID::CurrentNImExSum,
    MeterValueID::VoltageLNAvg,
    MeterValueID::CurrentLAvgImExSum,
    MeterValueID::CurrentLSumImExSum,
    MeterValueID::PowerApparentLSumImExSum,
    MeterValueID::PowerReactiveLSumIndCapDiff,
    MeterValueID::PhaseAngleLSum,
    MeterValueID::EnergyActiveLSumImExSum,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImportResettable,
    MeterValueID::EnergyActiveLSumExportResettable,
};

const MeterValueID rs485_helper_dsz16dze_ids[] = {
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
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::PowerApparentLSumImExSum,
    MeterValueID::PowerReactiveLSumIndCapDiff,
    MeterValueID::PowerFactorLSumDirectional,
    MeterValueID::PhaseAngleLSum,
    MeterValueID::FrequencyLAvg,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::PhaseAngleL1,
    MeterValueID::PhaseAngleL2,
    MeterValueID::PhaseAngleL3,
    MeterValueID::PhaseAngleLSum,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImportResettable,
    MeterValueID::EnergyActiveLSumExportResettable,
};

const MeterValueID rs485_helper_dem4a_ids[] = {
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
    MeterValueID::VoltageLNAvg,
    MeterValueID::CurrentLAvgImExSum,
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::PowerApparentLSumImExSum,
    MeterValueID::PowerReactiveLSumIndCapDiff,
    MeterValueID::PowerFactorLSumDirectional,
    MeterValueID::FrequencyLAvg,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::EnergyReactiveLSumInductive,
    MeterValueID::EnergyReactiveLSumCapacitive,
    MeterValueID::VoltageL1L2,
    MeterValueID::VoltageL2L3,
    MeterValueID::VoltageL3L1,
    MeterValueID::VoltageLLAvg,
    MeterValueID::CurrentNImExSum,
    MeterValueID::EnergyActiveLSumImExSum,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImportResettable,
    MeterValueID::EnergyActiveLSumExportResettable,
};

const MeterValueID rs485_helper_dmed341mid7er_ids[] = {
    MeterValueID::VoltageL1N,
    MeterValueID::VoltageL2N,
    MeterValueID::VoltageL3N,
    MeterValueID::CurrentL1ImExSum,
    MeterValueID::CurrentL2ImExSum,
    MeterValueID::CurrentL3ImExSum,
    MeterValueID::VoltageL1L2,
    MeterValueID::VoltageL2L3,
    MeterValueID::VoltageL3L1,
    MeterValueID::PowerActiveL1ImExDiff,
    MeterValueID::PowerActiveL2ImExDiff,
    MeterValueID::PowerActiveL3ImExDiff,
    MeterValueID::PowerReactiveL1IndCapDiff,
    MeterValueID::PowerReactiveL2IndCapDiff,
    MeterValueID::PowerReactiveL3IndCapDiff,
    MeterValueID::PowerApparentL1ImExSum,
    MeterValueID::PowerApparentL2ImExSum,
    MeterValueID::PowerApparentL3ImExSum,
    MeterValueID::PowerFactorL1Directional,
    MeterValueID::PowerFactorL2Directional,
    MeterValueID::PowerFactorL3Directional,
    MeterValueID::FrequencyLAvg,
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::PowerFactorLSumDirectional,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::EnergyReactiveLSumInductive,
    MeterValueID::EnergyReactiveLSumCapacitive,
    MeterValueID::EnergyActiveL1Import,
    MeterValueID::EnergyActiveL1Export,
    MeterValueID::EnergyReactiveL1Inductive,
    MeterValueID::EnergyReactiveL1Capacitive,
    MeterValueID::EnergyActiveL2Import,
    MeterValueID::EnergyActiveL2Export,
    MeterValueID::EnergyReactiveL2Inductive,
    MeterValueID::EnergyReactiveL2Capacitive,
    MeterValueID::EnergyActiveL3Import,
    MeterValueID::EnergyActiveL3Export,
    MeterValueID::EnergyReactiveL3Inductive,
    MeterValueID::EnergyReactiveL3Capacitive,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImportResettable,
    MeterValueID::EnergyActiveLSumExportResettable,
};

const MeterValueID rs485_helper_wm3m4c_ids[] = {
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
    MeterValueID::VoltageL1L2,
    MeterValueID::VoltageL2L3,
    MeterValueID::VoltageL3L1,
    MeterValueID::VoltageLLAvg,
    MeterValueID::VoltageTHDL1N,
    MeterValueID::VoltageTHDL2N,
    MeterValueID::VoltageTHDL3N,
    MeterValueID::CurrentTHDL1,
    MeterValueID::CurrentTHDL2,
    MeterValueID::CurrentTHDL3,
    // TBD: Some values will be added here once the complete register map of WM3M4C is better understood
    MeterValueID::Temperature,
    MeterValueID::RunTime,
    MeterValueID::EnergyActiveLSumImExSumResettable,
    MeterValueID::EnergyActiveLSumImportResettable,
    MeterValueID::EnergyActiveLSumExportResettable,
};

void rs485_helper_get_value_ids(uint32_t meter_type, const MeterValueID **value_ids, size_t *value_ids_len)
{
    switch (meter_type) {
        case METER_TYPE_SDM630:
        case METER_TYPE_SDM630MCTV2:
            *value_ids = rs485_helper_sdm630_ids;
            *value_ids_len = ARRAY_SIZE(rs485_helper_sdm630_ids);
            return;

        case METER_TYPE_SDM72DMV2:
            *value_ids = rs485_helper_sdm72v2_ids;
            *value_ids_len = ARRAY_SIZE(rs485_helper_sdm72v2_ids);
            return;

        case METER_TYPE_SDM72DM:
            *value_ids = rs485_helper_sdm72_ids;
            *value_ids_len = ARRAY_SIZE(rs485_helper_sdm72_ids);
            return;

        case METER_TYPE_DSZ15DZMOD:
            *value_ids = rs485_helper_dsz15dzmod_ids;
            *value_ids_len = ARRAY_SIZE(rs485_helper_dsz15dzmod_ids);
            return;

        case METER_TYPE_DSZ16DZE:
            *value_ids = rs485_helper_dsz16dze_ids;
            *value_ids_len = ARRAY_SIZE(rs485_helper_dsz16dze_ids);
            return;

        case METER_TYPE_DEM4A:
            *value_ids = rs485_helper_dem4a_ids;
            *value_ids_len = ARRAY_SIZE(rs485_helper_dem4a_ids);
            return;

        case METER_TYPE_DMED341MID7ER:
            *value_ids = rs485_helper_dmed341mid7er_ids;
            *value_ids_len = ARRAY_SIZE(rs485_helper_dmed341mid7er_ids);
            return;

        case METER_TYPE_WM3M4C:
            *value_ids = rs485_helper_wm3m4c_ids;
            *value_ids_len = ARRAY_SIZE(rs485_helper_wm3m4c_ids);
            return;

        default:
            *value_ids = nullptr;
            *value_ids_len = 0;
            logger.printfln("Unsupported meter type %lu.", meter_type);
    }
}