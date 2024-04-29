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
tables = [
    ('None', 0),
    #('Custom', 1),
    ('Sungrow Hybrid Inverter', 2),
    #('Sungrow String Inverter', 5),
    #('Sungrow String Inverter Grid', 6),
    #('Solarmax Max Storage Inverter', 7),
    #('Solarmax Max Storage Grid', 8),
    #('Solarmax Max Storage Battery', 9),
]

# NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
sungrow_hybrid_inverter_virtual_meters = [
    ('None', 0),
    ('Inverter', 1),
    ('Grid', 2),
    ('Battery', 3),
    ('Load', 4),
]

table_values = []
table_names = []

for table in tables:
    table_values.append('    {0} = {1},\n'.format(util.FlavoredName(table[0]).get().camel, table[1]))
    table_names.append('    case MeterModbusTCPTableID::{0}: return "{1}";\n'.format(util.FlavoredName(table[0]).get().camel, table[0]))

sungrow_hybrid_inverter_virtual_meter_values = []

for virtual_meter in sungrow_hybrid_inverter_virtual_meters:
    sungrow_hybrid_inverter_virtual_meter_values.append('    {0} = {1},\n'.format(util.FlavoredName(virtual_meter[0]).get().camel, virtual_meter[1]))

with open('meters_modbus_tcp_defs.h', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include <stdint.h>\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class MeterModbusTCPTableID : uint8_t {\n')
    f.write(''.join(table_values))
    f.write('};\n\n')
    f.write('enum class SungrowHybridInverterVirtualMeterID : uint8_t {\n')
    f.write(''.join(sungrow_hybrid_inverter_virtual_meter_values))
    f.write('};\n')

with open('../../../web/src/modules/meters_modbus_tcp/meters_modbus_tcp_defs.ts', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export const enum MeterModbusTCPTableID {\n')
    f.write(''.join(table_values))
    f.write('}\n\n')
    f.write('export const enum SungrowHybridInverterVirtualMeterID {\n')
    f.write(''.join(sungrow_hybrid_inverter_virtual_meter_values))
    f.write('}\n')

VALUE_ID_META  = 0xFFFFFFFF - 1
VALUE_ID_DEBUG = 0xFFFFFFFF - 2

START_ADDRESS_VIRTUAL = 0xFFFFFFFF - 1

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
        'name': 'MPPT 1 Voltage [0.1 V]',
        'value_id': VALUE_ID_META,
        'start_address': 5011,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 1 Current [0.1 A]',
        'value_id': VALUE_ID_META,
        'start_address': 5012,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 2 Voltage [0.1 V]',
        'value_id': VALUE_ID_META,
        'start_address': 5013,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 2 Current [0.1 A]',
        'value_id': VALUE_ID_META,
        'start_address': 5014,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 1&2 Voltage [V]',
        'value_id': 'VoltageDC',
        'start_address': START_ADDRESS_VIRTUAL,
        'value_type': 'U16',
        'scale_factor': 1.0,
    },
    {
        'name': 'MPPT 1&2 Current [A]',
        'value_id': 'CurrentDC',
        'start_address': START_ADDRESS_VIRTUAL,
        'value_type': 'U16',
        'scale_factor': 1.0,
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
        'name': 'Total PV Generation [0.1 kWh]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 13003,
        'value_type': 'U32',
        'scale_factor': 0.1,
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
        'scale_factor': -0.1,  # FIXME: is the current always positive?
    },
    {
        'name': 'Phase B Current [0.1 A]',
        'value_id': 'CurrentL2ImExDiff',
        'start_address': 13032,
        'value_type': 'S16',
        'scale_factor': -0.1,  # FIXME: is the current always positive?
    },
    {
        'name': 'Phase C Current [0.1 A]',
        'value_id': 'CurrentL3ImExDiff',
        'start_address': 13033,
        'value_type': 'S16',
        'scale_factor': -0.1,  # FIXME: is the current always positive?
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
                'value_id': VALUE_ID_META,
                'start_address': 13001,
                'value_type': 'U16',
                'scale_factor': 1.0,
            },
            {
                'name': 'Total Battery Charge Energy From PV [0.1 kWh]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 13013,
                'value_type': 'U32',
                'scale_factor': 0.1,
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
                'name': 'Battery State Of Health [0.1 %]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 13024,
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
                'name': 'Total Battery Discharge Energy [0.1 kWh]',
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
            f'        {value["start_address"] if value["start_address"] != START_ADDRESS_VIRTUAL else "START_ADDRESS_VIRTUAL"},\n'
            f'        MeterModbusTCP::ValueType::{value["value_type"]},\n'
            f'        {value["scale_factor"]}f,\n'
            '    },'
        )

        if value["value_id"] == VALUE_ID_META:
            value_index.append('    VALUE_INDEX_META,')
        elif value["value_id"] == VALUE_ID_DEBUG:
            value_index.append('    VALUE_INDEX_DEBUG,')
        else:
            value_ids.append(f'    MeterValueID::{value["value_id"]},')
            value_index.append(f'    {current_index},')
            current_index += 1

    name = util.FlavoredName(spec["name"]).get()

    spec_values.append(f'static const MeterModbusTCP::ValueSpec {name.under}_specs[] = {{\n' + '\n'.join(value_specs) + '\n};')
    spec_values.append(f'static const MeterValueID {name.under}_ids[] = {{\n' + '\n'.join(value_ids) + '\n};')
    spec_values.append(f'static const uint32_t {name.under}_index[] = {{\n' + '\n'.join(value_index) + '\n};')

with open('meters_modbus_tcp_defs.inc', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write(f'#define VALUE_INDEX_META  {VALUE_ID_META}u\n')
    f.write(f'#define VALUE_INDEX_DEBUG {VALUE_ID_DEBUG}u\n\n')
    f.write(f'#define START_ADDRESS_VIRTUAL {START_ADDRESS_VIRTUAL}u\n\n')
    f.write('\n\n'.join(spec_values) + '\n\n')
    f.write('\n\nstatic const char *get_table_name(MeterModbusTCPTableID table)\n')
    f.write('{\n')
    f.write('    switch (table) {\n')
    f.write(''.join(table_names))
    f.write('    default: return "Unknown table";\n')
    f.write('    }\n')
    f.write('}\n')
