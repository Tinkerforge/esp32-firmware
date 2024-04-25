import os
import sys
import importlib.util
import importlib.machinery
import csv

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util

# NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
presets = [
    #('Custom', 0),
    ('Sungrow Hybrid Inverter', 1),
    ('Sungrow Hybrid Inverter Grid', 2),
    ('Sungrow Hybrid Inverter Battery', 3),
    ('Sungrow Hybrid Inverter Load', 4),
    #('Sungrow String Inverter', 5),
    #('Sungrow String Inverter Grid', 6),
    #('Solarmax Max Storage Inverter', 7),
    #('Solarmax Max Storage Grid', 8),
    #('Solarmax Max Storage Battery', 9),
]

preset_values = []
preset_names = []

for preset in presets:
    preset_values.append('    {0} = {1},\n'.format(util.FlavoredName(preset[0]).get().camel, preset[1]))
    preset_names.append('    case MeterModbusTCPPreset::{0}: return "{1}";\n'.format(util.FlavoredName(preset[0]).get().camel, preset[0]))

with open('meters_modbus_tcp_defs.h', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include <stdint.h>\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class MeterModbusTCPPreset : uint8_t {\n')
    f.write(''.join(preset_values))
    f.write('};\n')

with open('../../../web/src/modules/meters_modbus_tcp/meters_modbus_tcp_defs.ts', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export const enum MeterModbusTCPPreset {\n')
    f.write(''.join(preset_values))
    f.write('}\n')

sungrow_hybrid_inverter_base_values = [
    {
        'name': 'Total Output Energy [0.1 kWh]',
        'value_id': 'EnergyActiveLSumExport',
        'start_address': 5004,
        'value_type': 'U32',
        'scale_factor': 0.1,
    },
    {
        'name': 'Inside Temperature [0.1 °C]',
        'value_id': 'TemperatureCabinet',
        'start_address': 5008,
        'value_type': 'S16',
        'scale_factor': 0.1,
    },
    {
        'name': 'Total DC Power [W]',
        'value_id': 'PowerDC',
        'start_address': 5017,
        'value_type': 'U32',
        'scale_factor': 1.0,
    },
    {
        'name': 'Reactive Power [var]',
        'value_id': 'PowerReactiveLSumIndCapDiff',
        'start_address': 5033,
        'value_type': 'S32',
        'scale_factor': 1.0,
    },
    {
        'name': 'Power Factor [0.001]',
        'value_id': 'PowerFactorLSumDirectional',
        'start_address': 5035,
        'value_type': 'S16',
        'scale_factor': 0.001,
    },
    {
        'name': 'Total Active Power [W]',
        'value_id': 'PowerActiveLSumImExDiff',
        'start_address': 13034,
        'value_type': 'S32',
        'scale_factor': -1.0,
    },
]

sungrow_hybrid_inverter_phase_voltages = [
    {
        'name': 'A-N Voltage [0.1 V]',
        'value_id': 'VoltageL1N',
        'start_address': 5019,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'B-N Voltage [0.1 V]',
        'value_id': 'VoltageL2N',
        'start_address': 5020,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'C-N Voltage [0.1 V]',
        'value_id': 'VoltageL3N',
        'start_address': 5021,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
]

sungrow_hybrid_inverter_line_voltages = [
    {
        'name': 'A-B Voltage [0.1 V]',
        'value_id': 'VoltageL1L2',
        'start_address': 5019,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'B-C Voltage [0.1 V]',
        'value_id': 'VoltageL2L3',
        'start_address': 5020,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'C-A Voltage [0.1 V]',
        'value_id': 'VoltageL3L1',
        'start_address': 5021,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
]

sungrow_hybrid_inverter_phase_currents = [
    {
        'name': 'Phase A Current [0.1 A]',
        'value_id': 'CurrentL1ImExDiff',
        'start_address': 13031,
        'value_type': 'S16',
        'scale_factor': -0.1,
    },
    {
        'name': 'Phase B Current [0.1 A]',
        'value_id': 'CurrentL2ImExDiff',
        'start_address': 13032,
        'value_type': 'S16',
        'scale_factor': -0.1,
    },
    {
        'name': 'Phase C Current [0.1 A]',
        'value_id': 'CurrentL3ImExDiff',
        'start_address': 13033,
        'value_type': 'S16',
        'scale_factor': -0.1,
    },
]

specs = [
    {
        'name': 'Sungrow Hybrid Inverter 1P2L',  # output type 1
        'values': sungrow_hybrid_inverter_base_values + sungrow_hybrid_inverter_phase_voltages[:1] + sungrow_hybrid_inverter_phase_currents[:1],
    },
    {
        'name': 'Sungrow Hybrid Inverter 3P4L',  # output type 2
        'values': sungrow_hybrid_inverter_base_values + sungrow_hybrid_inverter_phase_voltages + sungrow_hybrid_inverter_phase_currents,
    },
    {
        'name': 'Sungrow Hybrid Inverter 3P3L',  # output type 3
        'values': sungrow_hybrid_inverter_base_values + sungrow_hybrid_inverter_line_voltages + sungrow_hybrid_inverter_phase_currents,
    },
    {
        'name': 'Sungrow Hybrid Inverter Grid',
        'values': [
            {
                'name': 'Grid Frequency [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 5036,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Export Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 13010,
                'value_type': 'S32',
                'scale_factor': -1.0,
            },
            {
                'name': 'Total Import Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 13037,
                'value_type': 'U32',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Export Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 13046,
                'value_type': 'U32',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Sungrow Hybrid Inverter Battery',
        'values': [
            {
                'name': 'Running State',
                'value_id': None,
                'start_address': 13001,
                'value_type': 'U16',
                'scale_factor': 1.0,
            },
            {
                'name': 'Battery Voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 13020,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Current [0.1 A]',
                'value_id': 'CurrentDC',
                'start_address': 13021,
                'value_type': 'U16',
                'scale_factor': 0.1,  # sign depends on running state
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 13022,
                'value_type': 'U16',
                'scale_factor': 1.0,  # sign depends on running state
            },
            {
                'name': 'Battery Level [0.1 %]',
                'value_id': 'StateOfCharge',
                'start_address': 13023,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 13025,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Battery Energy Discharge [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 13027,
                'value_type': 'U32',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Charge Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 13041,
                'value_type': 'U32',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Sungrow Hybrid Inverter Load',
        'values': [
            {
                'name': 'Load Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 13008,
                'value_type': 'S32',
                'scale_factor': 1.0,
            },
        ],
    },
]

spec_values = []

for spec in specs:
    value_specs = []
    value_ids = []
    value_index = []
    current_index = 0

    for value in spec['values']:
        value_specs.append(
            '    {\n'
            f'        "{value["name"]}",\n'
            f'        {value["start_address"]},\n'
            f'        MeterModbusTCP::ValueType::{value["value_type"]},\n'
            f'        {value["scale_factor"]}f,\n'
            '    },'
        )

        if value["value_id"] != None:
            value_ids.append(f'    MeterValueID::{value["value_id"]},')
            value_index.append(f'    {current_index},')
            current_index += 1
        else:
            value_index.append('    UINT32_MAX,')

    name = util.FlavoredName(spec["name"]).get()

    spec_values.append(f'static MeterModbusTCP::ValueSpec {name.under}_specs[] = {{\n' + '\n'.join(value_specs) + '\n};')
    spec_values.append(f'static MeterValueID {name.under}_ids[] = {{\n' + '\n'.join(value_ids) + '\n};')
    spec_values.append(f'static uint32_t {name.under}_index[] = {{\n' + '\n'.join(value_index) + '\n};')

with open('meters_modbus_tcp_defs.h', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include <stdint.h>\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class MeterModbusTCPPreset : uint8_t {\n')
    f.write(''.join(preset_values))
    f.write('};\n')

with open('meters_modbus_tcp_defs.inc', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('\n\n'.join(spec_values) + '\n\n')
    f.write('\n\nstatic const char *get_preset_name(MeterModbusTCPPreset preset)\n')
    f.write('{\n')
    f.write('    switch (preset) {\n')
    f.write(''.join(preset_names))
    f.write('    default: return "Unknown preset";\n')
    f.write('    }\n')
    f.write('}\n')
