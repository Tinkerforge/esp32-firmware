import os
import sys
import importlib.util
import importlib.machinery

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util


def make_modbus_value_type(register_count, is_signed, is_float, register_order_is_le):
    assert 1 <= register_count <= 4, register_count

    #      bit [0..2]              bit [3]                          bit [4]                         bit [5]
    return register_count | ((1 if is_signed else 0) << 3) | ((1 if is_float else 0) << 4) | ((1 if register_order_is_le else 0) << 5)


# NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
tables = [
    ('None', 0),
    ('Custom', 1),
    ('Sungrow Hybrid Inverter', 2),
    ('Sungrow String Inverter', 3),
    ('Solarmax Max Storage', 4),
    ('Victron Energy GX', 5),
    ('Deye Hybrid Inverter Low Voltage', 6),
    ('Deye Hybrid Inverter High Voltage', 7),
]

enums = [
    {
        'name': 'Modbus Register Type',
        # NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
        'values': [
            ('Holding Register', 0),
            ('Input Register', 1),
        ],
    },
    {
        'name': 'Modbus Register Address Mode',
        # NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
        'values': [
            ('Address', 0),
            ('Number', 1),
        ],
    },
    {
        'name': 'Modbus Value Type',
        # NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
        'values': [
            ('U16',   make_modbus_value_type(1, False, False, False)),
            ('S16',   make_modbus_value_type(1, True,  False, False)),
            ('U32BE', make_modbus_value_type(2, False, False, False)),
            ('U32LE', make_modbus_value_type(2, False, False, True)),
            ('S32BE', make_modbus_value_type(2, True,  False, False)),
            ('S32LE', make_modbus_value_type(2, True,  False, True)),
            ('U64BE', make_modbus_value_type(4, False, False, False)),
            ('U64LE', make_modbus_value_type(4, False, False, True)),
            ('S64BE', make_modbus_value_type(4, True,  False, False)),
            ('S64LE', make_modbus_value_type(4, True,  False, True)),
            ('F32BE', make_modbus_value_type(2, True,  True,  False)),
            ('F32LE', make_modbus_value_type(2, True,  True,  True)),
            ('F64BE', make_modbus_value_type(4, True,  True,  False)),
            ('F64LE', make_modbus_value_type(4, True,  True,  True)),
        ],
    },
    {
        'name': 'Sungrow Hybrid Inverter Virtual Meter ID',
        # NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
        'values': [
            ('None', 0),
            ('Inverter', 1),
            ('Grid', 2),
            ('Battery', 3),
            ('Load', 4),
        ],
    },
    {
        'name': 'Sungrow String Inverter Virtual Meter ID',
        # NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
        'values': [
            ('None', 0),
            ('Inverter', 1),
            ('Grid', 2),
            ('Load', 3),
        ],
    },
    {
        'name': 'Solarmax Max Storage Virtual Meter ID',
        # NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
        'values': [
            ('None', 0),
            ('Inverter', 1),
            ('Grid', 2),
            ('Battery', 3),
        ],
    },
    {
        'name': 'Victron Energy GX Virtual Meter ID',
        # NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
        'values': [
            ('None', 0),
            ('Inverter', 1),
            ('Grid', 2),
            ('Battery', 3),
            ('Load', 4),
        ],
    },
    {
        'name': 'Deye Hybrid Inverter Virtual Meter ID',
        # NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
        'values': [
            ('None', 0),
            ('Inverter', 1),
            ('Grid', 2),
            ('Battery', 3),
            ('Load', 4),
        ],
    },
]

table_values = []
table_names = []

for table in tables:
    table_values.append('    {0} = {1},\n'.format(util.FlavoredName(table[0]).get().camel, table[1]))
    table_names.append('    case MeterModbusTCPTableID::{0}: return "{1}";\n'.format(util.FlavoredName(table[0]).get().camel, table[0]))

with open('meters_modbus_tcp_defs.h', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include <stdint.h>\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class MeterModbusTCPTableID : uint8_t {\n')
    f.write(''.join(table_values))
    f.write('};\n')

    for enum in enums:
        f.write(f'\nenum class {util.FlavoredName(enum["name"]).get().camel} : uint8_t {{\n')

        for value in enum['values']:
            f.write('    {0} = {1},\n'.format(util.FlavoredName(value[0]).get().camel, value[1]))

        f.write('};\n')

with open('../../../web/src/modules/meters_modbus_tcp/meters_modbus_tcp_defs.ts', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export const enum MeterModbusTCPTableID {\n')
    f.write(''.join(table_values))
    f.write('}\n')

    for enum in enums:
        f.write(f'\nexport const enum {util.FlavoredName(enum["name"]).get().camel} {{\n')

        for value in enum['values']:
            f.write('    {0} = {1},\n'.format(util.FlavoredName(value[0]).get().camel, value[1]))

        f.write('}\n')

VALUE_ID_META  = 0xFFFFFFFF - 1
VALUE_ID_DEBUG = 0xFFFFFFFF - 2

START_ADDRESS_VIRTUAL = 0xFFFFFFFF - 1

sungrow_hybrid_string_inverter_base_values = [
    {
        'name': 'Device Type',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5000,
        'value_type': 'U16',
    },
    {
        'name': 'Nominal Output Power [0.1 kW]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5001,
        'value_type': 'U16',
    },
    {
        'name': 'Total Output Energy [0.1 kWh]',
        'value_id': 'EnergyActiveLSumExport',
        'start_address': 5004,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variant': 'Hybrid',
    },
    {
        'name': 'Total Output Energy [kWh]',
        'value_id': 'EnergyActiveLSumExport',
        'start_address': 5004,
        'value_type': 'U32LE',
        'variant': 'String',
    },
    {
        'name': 'Total Running Time [h]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5006,
        'value_type': 'U32LE',
        'variant': 'String',
    },
    {
        'name': 'Inside Temperature [0.1 °C]',
        'value_id': 'TemperatureCabinet',
        'start_address': 5008,
        'value_type': 'S16',
        'scale_factor': 0.1,
    },
    {
        'name': 'Total Apparent Power [VA]',  # FIXME: not available for all device types
        'value_id': 'PowerApparentLSumImExDiff',
        'start_address': 5009,
        'value_type': 'U32LE',
        'variant': 'String',
    },
    {
        'name': 'MPPT 1 Voltage [0.1 V]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5011,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 1 Current [0.1 A]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5012,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 2 Voltage [0.1 V]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5013,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 2 Current [0.1 A]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5014,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 3 Voltage [0.1 V]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5015,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 3 Current [0.1 A]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5016,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'Total DC Power [W]',
        'value_id': 'PowerDCExport',
        'start_address': 5017,
        'value_type': 'U32LE',
    },
    {
        'name': 'Phase A Current [0.1 A]',  # FIXME: not available for all device types
        'value_id': 'CurrentL1Export',
        'start_address': 5022,
        'value_type': 'U16',
        'scale_factor': 0.1,
        'variant': 'String',
    },
    {
        'name': 'Phase B Current [0.1 A]',  # FIXME: not available for all device types
        'value_id': 'CurrentL2Export',
        'start_address': 5023,
        'value_type': 'U16',
        'scale_factor': 0.1,
        'variant': 'String',
    },
    {
        'name': 'Phase C Current [0.1 A]',  # FIXME: not available for all device types
        'value_id': 'CurrentL3Export',
        'start_address': 5024,
        'value_type': 'U16',
        'scale_factor': 0.1,
        'variant': 'String',
    },
    {
        'name': 'Total Active Power [W]',
        'value_id': 'PowerActiveLSumExport',
        'start_address': 5031,
        'value_type': 'U32LE',
        'variant': 'String',
    },
    {
        'name': 'Total Active Power [W]',
        'value_id': 'PowerActiveLSumImExDiff',
        'start_address': START_ADDRESS_VIRTUAL,
        'value_type': 'U32LE',
        'variant': 'String',
    },
    {
        'name': 'Reactive Power [var]',
        'value_id': 'PowerReactiveLSumIndCapDiff',
        'start_address': 5033,
        'value_type': 'S32LE',
    },
    {
        'name': 'Power Factor [0.001]',
        'value_id': 'PowerFactorLSumDirectional',
        'start_address': 5035,
        'value_type': 'S16',
        'scale_factor': 0.001,
    },
    {
        'name': 'Nominal Reactive Power [0.1 kvar]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5049,
        'value_type': 'U16',
        'variant': 'String',
    },
    {
        'name': 'Total Direct Energy Consumption [0.1 kWh]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5103,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variant': 'String',
    },
    {
        'name': 'Total Output Energy [0.1 kWh]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 5144,  # FIXME: use this instead of 5004, if available
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variant': 'String',
    },
    {
        'name': 'Total PV Generation [0.1 kWh]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 13003,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variant': 'Hybrid',
    },
    {
        'name': 'Total Direct Energy Consumption [0.1 kWh]',
        'value_id': VALUE_ID_DEBUG,
        'start_address': 13018,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variant': 'Hybrid',
    },
    {
        'name': 'Total Active Power [W]',
        'value_id': 'PowerActiveLSumImExDiff',
        'start_address': 13034,
        'value_type': 'S32LE',
        'scale_factor': -1.0,
        'variant': 'Hybrid',
    },
]

sungrow_hybrid_string_inverter_phase_voltages = [
    {
        'name': 'A-N Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL1N',
        'start_address': 5019,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'B-N Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL2N',
        'start_address': 5020,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'C-N Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL3N',
        'start_address': 5021,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
]

sungrow_hybrid_string_inverter_line_voltages = [
    {
        'name': 'A-B Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL1L2',
        'start_address': 5019,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'B-C Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL2L3',
        'start_address': 5020,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'C-A Voltage [0.1 V]',  # FIXME: not available for all string device types
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
        'variant': 'Hybrid',
    },
    {
        'name': 'Phase B Current [0.1 A]',
        'value_id': 'CurrentL2ImExDiff',
        'start_address': 13032,
        'value_type': 'S16',
        'scale_factor': -0.1,  # FIXME: is the current always positive?
        'variant': 'Hybrid',
    },
    {
        'name': 'Phase C Current [0.1 A]',
        'value_id': 'CurrentL3ImExDiff',
        'start_address': 13033,
        'value_type': 'S16',
        'scale_factor': -0.1,  # FIXME: is the current always positive?
        'variant': 'Hybrid',
    },
]

specs = [
    {
        'name': 'Sungrow {variant} Inverter 1P2L',  # output type 1
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'values': sungrow_hybrid_string_inverter_base_values + sungrow_hybrid_string_inverter_phase_voltages[:1] + sungrow_hybrid_inverter_phase_currents[:1],
    },
    {
        'name': 'Sungrow {variant} Inverter 3P4L',  # output type 2
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'values': sungrow_hybrid_string_inverter_base_values + sungrow_hybrid_string_inverter_phase_voltages + sungrow_hybrid_inverter_phase_currents,
    },
    {
        'name': 'Sungrow {variant} Inverter 3P3L',  # output type 3
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'values': sungrow_hybrid_string_inverter_base_values + sungrow_hybrid_string_inverter_line_voltages + sungrow_hybrid_inverter_phase_currents,
    },
    {
        'name': 'Sungrow {variant} Inverter Grid',
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Grid Frequency [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 5036,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Frequency [0.01 Hz]',  # FIXME: use this instead of 5036, if available
                'value_id': VALUE_ID_DEBUG,
                'start_address': 5148,
                'value_type': 'U16',
                'scale_factor': 0.01,
                'variant': 'String',
            },
            {
                'name': 'Meter Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 5083,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variant': 'String',
            },
            {
                'name': 'Meter Phase A Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 5085,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variant': 'String',
            },
            {
                'name': 'Meter Phase B Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 5087,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variant': 'String',
            },
            {
                'name': 'Meter Phase C Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 5089,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variant': 'String',
            },
            {
                'name': 'Total Export Energy [0.1 kWh]',  # FIXME: not available for all device types
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 5095,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variant': 'String',
            },
            {
                'name': 'Total Import Energy [0.1 kWh]',  # FIXME: not available for all device types
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 5099,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variant': 'String',
            },
            {
                'name': 'Export Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 13010,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variant': 'Hybrid',
            },
            {
                'name': 'Total Import Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 13037,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variant': 'Hybrid',
            },
            {
                'name': 'Total Export Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 13046,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variant': 'Hybrid',
            },
        ],
    },
    {
        'name': 'Sungrow Hybrid Inverter Battery',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Running State',
                'value_id': VALUE_ID_META,
                'start_address': 13001,
                'value_type': 'U16',
            },
            {
                'name': 'Total Battery Charge Energy From PV [0.1 kWh]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 13013,
                'value_type': 'U32LE',
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
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 13021,
                'value_type': 'U16',
                'scale_factor': 0.1,  # sign depends on running state
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
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
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Charge Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 13041,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Sungrow {variant} Inverter Load',
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Load Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 5091,
                'value_type': 'S32LE',
                'variant': 'String',
            },
            {
                'name': 'Load Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 13008,
                'value_type': 'S32LE',
                'variant': 'Hybrid',
            },
        ],
    },
    {
        'name': 'Solarmax Max Storage Inverter',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Inverter DC Power [W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 110,
                'value_type': 'U32LE',
            },
            {
                'name': 'Total DC Power [W]',
                'value_id': 'PowerDCExport',
                'start_address': 112,
                'value_type': 'U32LE',
            },
            {
                'name': 'Direct Power Usage [W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 116,
                'value_type': 'U32LE',
            },
            {
                'name': 'Active Power [W]',
                'value_id': 'PowerActiveLSumExport',
                'start_address': 120,
                'value_type': 'U32LE',
            },
        ],
    },
    {
        'name': 'Solarmax Max Storage Grid',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Export Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 118,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
            },
        ],
    },
    {
        'name': 'Solarmax Max Storage Battery',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 114,
                'value_type': 'S32LE',
            },
            {
                'name': 'Battery SoC [%]',
                'value_id': 'StateOfCharge',
                'start_address': 122,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'Victron Energy GX Inverter',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'AC Coupled PV On Output L1 [W]',
                'value_id': 'PowerActiveL1Export',
                'start_address': 808,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV On Output L2 [W]',
                'value_id': 'PowerActiveL2Export',
                'start_address': 809,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV On Output L3 [W]',
                'value_id': 'PowerActiveL3Export',
                'start_address': 810,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV On Output L1+L2+L3 [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': START_ADDRESS_VIRTUAL,
                'value_type': 'S32LE',
            },
        ],
    },
    {
        'name': 'Victron Energy GX Grid',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Grid L1 [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 820,
                'value_type': 'S16',
            },
            {
                'name': 'Grid L2 [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 821,
                'value_type': 'S16',
            },
            {
                'name': 'Grid L3 [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 822,
                'value_type': 'S16',
            },
            {
                'name': 'Grid L1+L2+L3 [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': START_ADDRESS_VIRTUAL,
                'value_type': 'S32LE',
            },
        ],
    },
    {
        'name': 'Victron Energy GX Battery',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Battery Voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 840,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Current [0.1 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 841,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 842,
                'value_type': 'S16',
            },
            {
                'name': 'Battery State Of Charge [%]',
                'value_id': 'StateOfCharge',
                'start_address': 843,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'Victron Energy GX Load',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'AC Consumption L1 [W]',
                'value_id': 'PowerActiveL1Import',
                'start_address': 817,
                'value_type': 'U16',
            },
            {
                'name': 'AC Consumption L2 [W]',
                'value_id': 'PowerActiveL2Import',
                'start_address': 818,
                'value_type': 'U16',
            },
            {
                'name': 'AC Consumption L3 [W]',
                'value_id': 'PowerActiveL3Import',
                'start_address': 819,
                'value_type': 'U16',
            },
            {
                'name': 'AC Consumption L1+L2+L3 [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': START_ADDRESS_VIRTUAL,
                'value_type': 'S32LE',
            },
        ],
    },
    {
        'name': 'Deye Hybrid Inverter',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Device Type',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 0,
                'value_type': 'U16',
            },
            {
                'name': 'Rated Power [0.1 W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 20,
                'value_type': 'U32LE',
            },
            {
                'name': 'Total Active Power Generation [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 504,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Reactive Power Generation [0.1 kWh]',
                'value_id': 'EnergyReactiveLSumIndCapSum',  # FIXME: sum vs diff?
                'start_address': 506,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total PV Power [0.1 kWh]',
                'value_id': 'EnergyDCExport',
                'start_address': 534,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Transformer Temperature [0.1 °C]',
                'value_id': 'TemperatureTransformer',
                'start_address': 540,
                'value_type': 'U16',
                'offset': -1000.0,
                'scale_factor': 0.1,
            },
            {
                'name': 'Heat Sink Temperature [0.1 °C]',
                'value_id': 'TemperatureHeatSink',
                'start_address': 541,
                'value_type': 'U16',
                'offset': -1000.0,
                'scale_factor': 0.1,
            },
            {
                'name': 'Output Phase Voltage A [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 627,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Output Phase Voltage B [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 628,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Output Phase Voltage C [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 629,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Output Phase Current A [0.01 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 630,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Output Phase Current B [0.01 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 631,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Output Phase Current C [0.01 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 632,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Output Phase Power A [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 633,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Output Phase Power B [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 634,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Output Phase Power C [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 635,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Output Total Active Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 636,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Output Total Apparent Power [VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 637,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 638,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
        ],
    },
    {
        'name': 'Deye Hybrid Inverter Grid',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Total Grid Buy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 522,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Grid Sell [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 524,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Phase Voltage A [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 598,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Phase Voltage B [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 599,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Phase Voltage C [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 600,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Line Voltage A-B [0.1 V]',
                'value_id': VALUE_ID_DEBUG,  # FIXME: there seems to be no way to detect 3P4L vs 3P3L, ignore this voltage for now
                'start_address': 601,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Line Voltage B-C [0.1 V]',
                'value_id': VALUE_ID_DEBUG,  # FIXME: there seems to be no way to detect 3P4L vs 3P3L, ignore this voltage for now
                'start_address': 602,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Line Voltage C-A [0.1 V]',
                'value_id': VALUE_ID_DEBUG,  # FIXME: there seems to be no way to detect 3P4L vs 3P3L, ignore this voltage for now
                'start_address': 603,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Internal Power A [W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 604,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Power B [W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 605,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Power C [W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 606,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Total Active Power [W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 607,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Total Apparent Power [VA]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 608,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 609,
                'value_type': 'U16',
                'scale_factor': 0.01,  # FIXME: undocumented
            },
            {
                'name': 'Grid Side Internal Current A [0.01 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 610,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Current B [0.01 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 611,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Current C [0.01 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 612,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Current A [0.01 A]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 613,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Current B [0.01 A]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 614,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Current C [0.01 A]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 615,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Power A [W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 616,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Power B [W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 617,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Power C [W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 618,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Total Active Power [W]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 619,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Total Apparent Power [VA]',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 620,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Power Factor',
                'value_id': VALUE_ID_DEBUG,
                'start_address': 621,
                'value_type': 'U16',
                'scale_factor': 0.001,
            },
            {
                'name': 'Grid Side Power A [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 622,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Power B [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 623,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Power C [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 624,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Total Active Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 625,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
        ],
    },
    {
        'name': 'Deye Hybrid Inverter Battery {variant}',
        'variants': ['Low Voltage', 'High Voltage'],
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Total Battery Charge [0.1 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 516,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Battery Discharge [0.1 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 518,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 586,
                'value_type': 'U16',
                'offset': -1000.0,
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Voltage [0.01 V]',
                'value_id': 'VoltageDC',
                'start_address': 587,
                'value_type': 'U16',
                'scale_factor': 0.01,
                'variant': 'Low Voltage',
            },
            {
                'name': 'Battery Voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 587,
                'value_type': 'U16',
                'scale_factor': 0.1,
                'variant': 'High Voltage',
            },
            {
                'name': 'Battery Capacity [%]',
                'value_id': 'StateOfCharge',
                'start_address': 588,
                'value_type': 'U16',
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 590,
                'value_type': 'S16',
                'scale_factor': -1.0,
                'variant': 'Low Voltage',
            },
            {
                'name': 'Battery Power [10 W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 590,
                'value_type': 'S16',
                'scale_factor': -10.0,
                'variant': 'High Voltage',
            },
            {
                'name': 'Battery Current [0.01 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 591,
                'value_type': 'S16',
                'scale_factor': -0.01,
            },
        ],
    },
    {
        'name': 'Deye Hybrid Inverter Load',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Total Load Power [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 527,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Load Phase Voltage A [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 644,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Load Phase Voltage B [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 645,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Load Phase Voltage C [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 646,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Load Phase Current A [0.01 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 647,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Load Phase Current B [0.01 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 648,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Load Phase Current C [0.01 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 649,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Load Phase Power A [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 650,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Load Phase Power B [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 651,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Load Phase Power C [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 652,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Load Total Active Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 653,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Load Total Apparent Power [VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 654,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Load Frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 655,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
        ],
    },
]

spec_values = []

for spec in specs:
    for variant_spec in spec.get('variants', [None]):
        value_specs = []
        value_ids = []
        value_index = []
        current_index = 0

        for value in spec['values']:
            variant_value = value.get('variant')

            if variant_value != None and variant_value not in variant_spec:
                continue

            value_specs.append(
                '    {\n'
                f'        "{value["name"]}",\n'
                f'        ModbusRegisterType::{value.get("register_type", spec["register_type"])},\n'
                f'        {value["start_address"] if value["start_address"] != START_ADDRESS_VIRTUAL else "START_ADDRESS_VIRTUAL"},\n'
                f'        ModbusValueType::{value["value_type"]},\n'
                f'        {value.get("offset", 0.0)}f,\n'
                f'        {value.get("scale_factor", 1.0)}f,\n'
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

        name = util.FlavoredName(spec["name"].format(variant=variant_spec)).get()

        spec_values.append(f'static const MeterModbusTCP::ValueSpec {name.under}_specs[] = {{\n' + '\n'.join(value_specs) + '\n};')
        spec_values.append(f'static const MeterValueID {name.under}_ids[] = {{\n' + '\n'.join(value_ids) + '\n};')
        spec_values.append(f'static const uint32_t {name.under}_index[] = {{\n' + '\n'.join(value_index) + '\n};')
        spec_values.append(f'static const MeterModbusTCP::ValueTable {name.under}_table = {{\n'
                           f'    {name.under}_specs,\n'
                           f'    ARRAY_SIZE({name.under}_specs),\n'
                           f'    {name.under}_ids,\n'
                           f'    ARRAY_SIZE({name.under}_ids),\n'
                           f'    {name.under}_index,\n'
                           '};')

with open('meters_modbus_tcp_defs.inc', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write(f'#define VALUE_INDEX_META  {VALUE_ID_META}u\n')
    f.write(f'#define VALUE_INDEX_DEBUG {VALUE_ID_DEBUG}u\n\n')
    f.write(f'#define START_ADDRESS_VIRTUAL {START_ADDRESS_VIRTUAL}u\n\n')
    f.write('\n\n'.join(spec_values) + '\n\n')
    f.write('static const char *get_table_name(MeterModbusTCPTableID table)\n')
    f.write('{\n')
    f.write('    switch (table) {\n')
    f.write(''.join(table_names))
    f.write('    default: return "Unknown table";\n')
    f.write('    }\n')
    f.write('}\n')
