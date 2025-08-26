import sys
import tinkerforge_util as tfutil
import sungrow
import solarmax
import victron_energy
import deye
import alpha_ess
import shelly
import goodwe
import solax
import fronius
import hailei
import fox_ess
import siemens
import carlo_gavazzi
import solaredge
import eastron
import tinkerforge
import sax_power
import e3dc
import huawei
import sma
import varta
import chisage_ess

tfutil.create_parent_module(__file__, 'software')

from software import util

modules = [
    sungrow,
    solarmax,
    victron_energy,
    deye,
    alpha_ess,
    shelly,
    goodwe,
    solax,
    fronius,
    hailei,
    fox_ess,
    siemens,
    carlo_gavazzi,
    solaredge,
    eastron,
    tinkerforge,
    sax_power,
    e3dc,
    huawei,
    sma,
    varta,
    chisage_ess,
]

default_device_addresses = []
specs = []

for module in modules:
    default_device_addresses += module.default_device_addresses
    specs += module.specs

specs_h = []
specs_cpp = []

specs_h.append('namespace DefaultDeviceAddress {')
specs_h.append(f'enum {{\n{"\n".join(["    {0} = {1},".format(util.FlavoredName(name).get().camel, value) for name, value in default_device_addresses])}\n}};')
specs_h.append('}')

for spec in specs:
    for variant_spec in spec.get('variants', [None]):
        spec_name = util.FlavoredName(spec['name'].format(variant=variant_spec)).get()

        value_names = []
        value_addresses = []
        value_specs = []
        value_ids_raw = []
        value_ids = []
        value_index = []
        f32_negative_max_as_nan = 'true' if spec.get('f32_negative_max_as_nan', False) else 'false'
        current_index = 0

        for value in spec['values']:
            variants_value = value.get('variants')

            if variants_value != None and variant_spec not in variants_value:
                continue

            if value['name'] in value_names:
                print(f'Error: Value {spec_name.space} / {value["name"]} is duplicate')
                sys.exit(1)

            value_names.append(value['name'])

            start_address_offset = value.get('start_address_offset', spec.get('start_address_offset', 0))
            start_address = value['start_address'] - start_address_offset if value['start_address'] != 'START_ADDRESS_VIRTUAL' else 'START_ADDRESS_VIRTUAL'

            if start_address != "START_ADDRESS_VIRTUAL":
                address_name_words = value['name'].split('|', 1)[-1].split('[')[0].replace('-', ' ').replace('/', ' ').replace('.', ' ').split(' ')
                address_name = ''.join([word[0].upper() + word[1:] for word in address_name_words if len(word) > 0])

                value_addresses.append(f'    {address_name} = {start_address},')

            value_specs.append(
                '    {\n'
                f'        "{value["name"]}",\n'
                f'        ModbusRegisterType::{value.get("register_type", spec["register_type"])},\n'
                f'        {start_address},\n'
                f'        ModbusValueType::{value.get("value_type", "None")},\n'
                f'        {"true" if value.get("drop_sign", False) else "false"},\n'
                f'        {value.get("offset", 0.0)}f,\n'
                f'        {value.get("scale_factor", 1.0)}f,\n'
                '    },'
            )

            if value['value_id'] == 'VALUE_ID_META':
                value_index.append('    VALUE_INDEX_META,')
            elif value['value_id'] == 'VALUE_ID_DEBUG':
                value_index.append('    VALUE_INDEX_DEBUG,')
            else:
                if value['value_id'] in value_ids_raw:
                    print(f'Error: Value {spec_name.space} / {value["name"]} has duplicate ID {value["value_id"]}')
                    sys.exit(1)

                value_ids_raw.append(value['value_id'])
                value_ids.append(f'    MeterValueID::{value["value_id"]},')
                value_index.append(f'    {current_index},')
                current_index += 1

        specs_h.append(f'namespace {spec_name.camel}Address {{')
        specs_h.append(f'enum {{\n{"\n".join(value_addresses)}\n}};')
        specs_h.append('}')

        specs_cpp.append(f'static const MeterModbusTCP::ValueSpec {spec_name.under}_specs[] = {{\n' + '\n'.join(value_specs) + '\n};')

        if len(value_ids) > 0:
            specs_cpp.append(f'static const MeterValueID {spec_name.under}_ids[] = {{\n' + '\n'.join(value_ids) + '\n};')

        specs_cpp.append(f'static const uint32_t {spec_name.under}_index[] = {{\n' + '\n'.join(value_index) + '\n};')

        specs_cpp.append(f'static const MeterModbusTCP::TableSpec {spec_name.under}_table_ = {{\n'
                         f'    MeterLocation::{spec['default_location']},\n'
                         f'    {spec_name.under}_specs,\n'
                         f'    ARRAY_SIZE({spec_name.under}_specs),\r')

        specs_h.append(f'extern const MeterModbusTCP::TableSpec *{spec_name.under}_table;')

        if len(value_ids) > 0:
            specs_cpp.append(f'    {spec_name.under}_ids,\n'
                             f'    ARRAY_SIZE({spec_name.under}_ids),\r')
        else:
            specs_cpp.append('    nullptr,\n'
                             '    0,\r')

        specs_cpp.append(f'    {spec_name.under}_index,\n'
                         f'    {f32_negative_max_as_nan},\n'
                         '};')

        specs_cpp.append(f'const MeterModbusTCP::TableSpec *{spec_name.under}_table = &{spec_name.under}_table_;')

ts = '// WARNING: This file is generated.\n\n'
ts += 'import { MeterModbusTCPTableID } from "./meter_modbus_tcp_table_id.enum";\n\n'
ts += 'export const enum DefaultDeviceAddress {\n'
ts += '\n'.join([f'    {util.FlavoredName(name).get().camel} = {value},' for name, value in default_device_addresses]) + '\n'
ts += '}\n\n'
ts += 'export function get_default_device_address(table_id: number)\n'
ts += '{\n'
ts += '    switch (table_id) {\n'
ts += '\n'.join([f'    case MeterModbusTCPTableID.{util.FlavoredName(name).get().camel}: return DefaultDeviceAddress.{util.FlavoredName(name).get().camel};' for name, value in default_device_addresses]) + '\n'
ts += '    default: return undefined;\n'
ts += '    }\n'
ts += '}\n'

tfutil.write_file_if_different('../../../web/src/modules/meters_modbus_tcp/meter_modbus_tcp_specs.ts', ts)

h = '// WARNING: This file is generated.\n\n'
h += '#include "meter_modbus_tcp.h"\n\n'
h += '#define VALUE_INDEX_META  0xFFFFFFFEu\n'
h += '#define VALUE_INDEX_DEBUG 0xFFFFFFFDu\n\n'
h += '#define START_ADDRESS_VIRTUAL 0xFFFFFFFEu\n\n'
h += '\n\n'.join(specs_h).replace('\r\n', '') + '\n'

tfutil.write_file_if_different('meter_modbus_tcp_specs.h', h)

cpp = '// WARNING: This file is generated.\n\n'
cpp += '#include "meter_modbus_tcp_specs.h"\n\n'
cpp += '\n\n'.join(specs_cpp).replace('\r\n', '') + '\n'

tfutil.write_file_if_different('meter_modbus_tcp_specs.cpp', cpp)
