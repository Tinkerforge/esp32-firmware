import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util
from software.src.modules.meters.meter_value_id import MeterValueID

presets = [
    ('none', 'Unknown', [
    ]),

    ('pve', 'Grid', [
        MeterValueID.PowerActiveLSumImExDiff,
    ]),

    ('dlm', 'Grid', [
        MeterValueID.CurrentL1ImExDiff,
        MeterValueID.CurrentL2ImExDiff,
        MeterValueID.CurrentL3ImExDiff,
    ]),

    ('pve_dlm', 'Grid', [
        MeterValueID.PowerActiveLSumImExDiff,
        MeterValueID.CurrentL1ImExDiff,
        MeterValueID.CurrentL2ImExDiff,
        MeterValueID.CurrentL3ImExDiff,
    ]),

    ('eastron_sdm72', 'Unknown', [
        MeterValueID.PowerActiveLSumImExDiff,
        MeterValueID.EnergyActiveLSumImport,
        MeterValueID.EnergyActiveLSumExport,
        MeterValueID.EnergyActiveLSumImExSum,
        MeterValueID.EnergyActiveLSumImExSumResettable,
        MeterValueID.EnergyActiveLSumImportResettable,
        MeterValueID.EnergyActiveLSumExportResettable,
    ]),

    ('eastron_sdm630', 'Unknown', [
        MeterValueID.VoltageL1N,
        MeterValueID.VoltageL2N,
        MeterValueID.VoltageL3N,
        MeterValueID.CurrentL1ImExSum,
        MeterValueID.CurrentL2ImExSum,
        MeterValueID.CurrentL3ImExSum,
        MeterValueID.PowerActiveL1ImExDiff,
        MeterValueID.PowerActiveL2ImExDiff,
        MeterValueID.PowerActiveL3ImExDiff,
        MeterValueID.PowerApparentL1ImExSum,
        MeterValueID.PowerApparentL2ImExSum,
        MeterValueID.PowerApparentL3ImExSum,
        MeterValueID.PowerReactiveL1IndCapDiff,
        MeterValueID.PowerReactiveL2IndCapDiff,
        MeterValueID.PowerReactiveL3IndCapDiff,
        MeterValueID.PowerFactorL1Directional,
        MeterValueID.PowerFactorL2Directional,
        MeterValueID.PowerFactorL3Directional,
        MeterValueID.PhaseAngleL1,
        MeterValueID.PhaseAngleL2,
        MeterValueID.PhaseAngleL3,
        MeterValueID.VoltageLNAvg,
        MeterValueID.CurrentLAvgImExSum,
        MeterValueID.CurrentLSumImExSum,
        MeterValueID.PowerActiveLSumImExDiff,
        MeterValueID.PowerApparentLSumImExSum,
        MeterValueID.PowerReactiveLSumIndCapDiff,
        MeterValueID.PowerFactorLSumDirectional,
        MeterValueID.PhaseAngleLSum,
        MeterValueID.FrequencyLAvg,
        MeterValueID.EnergyActiveLSumImport,
        MeterValueID.EnergyActiveLSumExport,
        MeterValueID.EnergyReactiveLSumInductive,
        MeterValueID.EnergyReactiveLSumCapacitive,
        MeterValueID.EnergyApparentLSumImExSum,
        MeterValueID.ElectricCharge,
        MeterValueID.VoltageL1L2,
        MeterValueID.VoltageL2L3,
        MeterValueID.VoltageL3L1,
        MeterValueID.VoltageLLAvg,
        MeterValueID.CurrentNImExSum,
        MeterValueID.VoltageTHDL1N,
        MeterValueID.VoltageTHDL2N,
        MeterValueID.VoltageTHDL3N,
        MeterValueID.CurrentTHDL1,
        MeterValueID.CurrentTHDL2,
        MeterValueID.CurrentTHDL3,
        MeterValueID.VoltageTHDLNAvg,
        MeterValueID.CurrentTHDLAvg,
        MeterValueID.VoltageTHDL1L2,
        MeterValueID.VoltageTHDL2L3,
        MeterValueID.VoltageTHDL3L1,
        MeterValueID.VoltageTHDLLAvg,
        MeterValueID.EnergyActiveLSumImExSum,
        MeterValueID.EnergyReactiveLSumIndCapSum,
        MeterValueID.EnergyActiveL1Import,
        MeterValueID.EnergyActiveL2Import,
        MeterValueID.EnergyActiveL3Import,
        MeterValueID.EnergyActiveL1Export,
        MeterValueID.EnergyActiveL2Export,
        MeterValueID.EnergyActiveL3Export,
        MeterValueID.EnergyActiveL1ImExSum,
        MeterValueID.EnergyActiveL2ImExSum,
        MeterValueID.EnergyActiveL3ImExSum,
        MeterValueID.EnergyReactiveL1Inductive,
        MeterValueID.EnergyReactiveL2Inductive,
        MeterValueID.EnergyReactiveL3Inductive,
        MeterValueID.EnergyReactiveL1Capacitive,
        MeterValueID.EnergyReactiveL2Capacitive,
        MeterValueID.EnergyReactiveL3Capacitive,
        MeterValueID.EnergyReactiveL1IndCapSum,
        MeterValueID.EnergyReactiveL2IndCapSum,
        MeterValueID.EnergyReactiveL3IndCapSum,
        MeterValueID.EnergyActiveLSumImExSumResettable,
        MeterValueID.EnergyActiveLSumImportResettable,
        MeterValueID.EnergyActiveLSumExportResettable,
    ]),

    ('eastron_sdm72v2', 'Unknown', [
        MeterValueID.VoltageL1N,
        MeterValueID.VoltageL2N,
        MeterValueID.VoltageL3N,
        MeterValueID.CurrentL1ImExSum,
        MeterValueID.CurrentL2ImExSum,
        MeterValueID.CurrentL3ImExSum,
        MeterValueID.PowerActiveL1ImExDiff,
        MeterValueID.PowerActiveL2ImExDiff,
        MeterValueID.PowerActiveL3ImExDiff,
        MeterValueID.PowerApparentL1ImExSum,
        MeterValueID.PowerApparentL2ImExSum,
        MeterValueID.PowerApparentL3ImExSum,
        MeterValueID.PowerReactiveL1IndCapDiff,
        MeterValueID.PowerReactiveL2IndCapDiff,
        MeterValueID.PowerReactiveL3IndCapDiff,
        MeterValueID.PowerFactorL1Directional,
        MeterValueID.PowerFactorL2Directional,
        MeterValueID.PowerFactorL3Directional,
        MeterValueID.VoltageLNAvg,
        MeterValueID.CurrentLAvgImExSum,
        MeterValueID.CurrentLSumImExSum,
        MeterValueID.PowerActiveLSumImExDiff,
        MeterValueID.PowerApparentLSumImExSum,
        MeterValueID.PowerReactiveLSumIndCapDiff,
        MeterValueID.PowerFactorLSumDirectional,
        MeterValueID.FrequencyLAvg,
        MeterValueID.EnergyActiveLSumImport,
        MeterValueID.EnergyActiveLSumExport,
        MeterValueID.VoltageL1L2,
        MeterValueID.VoltageL2L3,
        MeterValueID.VoltageL3L1,
        MeterValueID.VoltageLLAvg,
        MeterValueID.CurrentNImExSum,
        MeterValueID.EnergyActiveLSumImExSum,
        MeterValueID.EnergyReactiveLSumIndCapSum,
        MeterValueID.EnergyActiveLSumImExSumResettable,
        MeterValueID.EnergyActiveLSumImportResettable,
        MeterValueID.EnergyActiveLSumExportResettable,
    ]),
]

with open('presets.inc', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated\n\n')
    f.write('#include "../meters/meter_value_id.h"\n')
    f.write('#include "../meters/meter_location.enum.h"\n\n')

    for preset in presets:
        if preset[1] == 'Unknown':
            continue

        f.write(f'static const MeterValueID preset_value_ids_{preset[0]}[] = {{\n')

        for mvid in preset[2]:
            f.write(f'    MeterValueID::{mvid.name},\n')

        f.write('};\n\n')

    f.write('static const MeterValueID *preset_value_ids[] = {\n')

    for preset in presets:
        if preset[1] == 'Unknown':
            continue

        f.write(f"    preset_value_ids_{preset[0]},\n")

    f.write('};\n\n')
    f.write('static const size_t preset_value_ids_count[] = {\n')

    for preset in presets:
        if preset[1] == 'Unknown':
            continue

        f.write(f"    {len(preset[2])},\n")

    f.write('};\n\n')
    f.write('static const MeterLocation preset_default_locations[] = {\n')

    for preset in presets:
        if preset[1] == 'Unknown':
            continue

        f.write(f"    MeterLocation::{preset[1]},\n")

    f.write('};\n')

with open('../../../web/src/modules/meters_api/presets.ts', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated\n\n')
    f.write('import { MeterValueID } from "../meters/meter_value_id"\n')
    f.write('import { MeterLocation } from "../meters/meter_location.enum"\n\n')
    f.write('export const PRESET_VALUE_IDS: {[key: string]: MeterValueID[]} = {\n')

    for preset in presets:
        f.write(f"   '{preset[0]}': [\n")

        for mvid in preset[2]:
            f.write(f'       MeterValueID.{mvid.name},\n')

        f.write('   ],\n')

    f.write('};\n\n')
    f.write('export const PRESET_DEFAULT_LOCATIONS: {[key: string]: MeterLocation} = {\n')

    for preset in presets:
        f.write(f"   '{preset[0]}': MeterLocation.{preset[1]},\n")

    f.write('};\n')

