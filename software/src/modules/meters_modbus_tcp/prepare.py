import sys
import collections
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
import janitza

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
    janitza,
]

table_prototypes = []
table_typedefs = []
table_typenames = []
table_news = []
default_device_addresses = []
specs = []
virtual_meters = collections.OrderedDict()

for module in modules:
    for table_prototype in module.table_prototypes:
        table_id = util.FlavoredName(table_prototype[0]).get()

        if table_prototype[1] == None:
            table_prototypes.append(f'\n    table_prototypes->push_back({{MeterModbusTCPTableID::{table_id.camel}, *Config::Null()}});')
        else:
            table_prototypes.append(f'\n    table_prototypes->push_back({{MeterModbusTCPTableID::{table_id.camel}, Config::Object({{')

            for member in table_prototype[1]:
                if member == 'virtual_meter':
                    table_prototypes.append(f'        {{"virtual_meter", Config::Enum({table_id.camel}VirtualMeter::None)}},')
                elif member == 'device_address':
                    table_prototypes.append(f'        {{"device_address", Config::Uint8(DefaultDeviceAddress::{table_id.camel})}},')
                elif member == 'device_address_no_default':
                    table_prototypes.append('        {"device_address", Config::Uint8(0)},')
                elif isinstance(member, tuple):
                    table_prototypes.append(f'        {{"{member[0]}", {member[1]}}},')
                else:
                    print(f'Error: Table prototype {table_id.space} has unknown member {member}')
                    sys.exit(1)

            table_prototypes.append('    })});')

        table_typedefs.append(f'type TableConfig{table_id.camel} = [\n'
                              f'    MeterModbusTCPTableID.{table_id.camel},\n    {{')

        table_new = []

        if table_prototype[1] == None:
            table_new.append('null')
        else:
            for member in table_prototype[1]:
                if isinstance(member, tuple):
                    member_name = member[0]
                else:
                    member_name = member

                if member_name == 'device_address_no_default':
                    table_typedefs.append('        device_address: number;')
                else:
                    table_typedefs.append(f'        {member_name}: number;')

                if member_name == 'device_address':
                    table_new.append(f'device_address: DefaultDeviceAddress.{table_id.camel}')
                elif member_name == 'device_address_no_default':
                    table_new.append(f'device_address: null')
                else:
                    table_new.append(f'{member_name}: null')

        table_typedefs.append('    },\n];\n')

        table_typenames.append(f'TableConfig{table_id.camel}')

        table_news.append(f'    case MeterModbusTCPTableID.{table_id.camel}:')
        table_news.append(f'        return [MeterModbusTCPTableID.{table_id.camel}, {{{", ".join(table_new)}}}];\n'.replace('{null}', 'null'))

    default_device_addresses += module.default_device_addresses
    specs += module.specs

specs_ts = []
specs_h = []
specs_cpp = []

specs_h.append('namespace DefaultDeviceAddress {')
specs_h.append(f'enum {{\n{"\n".join(["    {0} = {1},".format(util.FlavoredName(name).get().camel, value) for name, value in default_device_addresses])}\n}};')
specs_h.append('}')

for spec in specs:
    for variant_spec in spec.get('variants', [None]):
        spec_name = util.FlavoredName(spec['name'].format(variant=variant_spec)).get()

        if 'virtual_meter' in spec:
            virtual_meter_group = util.FlavoredName(spec['virtual_meter'][0].format(variant=variant_spec)).get()

            if spec['virtual_meter'][1] != None:
                virtual_meter_member = util.FlavoredName(spec['virtual_meter'][1]).get()
            else:
                virtual_meter_member = None

            virtual_meters.setdefault(virtual_meter_group, []).append((virtual_meter_member, spec_name, spec['default_location']))

        if spec['values'] == None:
            continue

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

            start_number = value.get('start_number')

            if start_number != None:
                if start_number == 'START_NUMBER_VIRTUAL':
                    start_address = 'START_ADDRESS_VIRTUAL'
                else:
                    start_address = start_number - 1
            else:
                start_address = value['start_address']

            if start_address != 'START_ADDRESS_VIRTUAL':
                address_name_words = value['name'].split('|', 1)[-1].split('[')[0].replace('+', ' ').replace('-', ' ').replace('/', ' ').replace('.', ' ').replace(',', ' ').split(' ')
                address_name = ''.join([word[0].upper() + word[1:] for word in address_name_words if len(word) > 0])

                value_addresses.append(f'    {address_name} = {start_address},')

            value_specs.append('    {\n'
                              f'        "{value["name"]}",\n'
                              f'        ModbusRegisterType::{value.get("register_type", spec["register_type"])},\n'
                              f'        {start_address},\n'
                              f'        ModbusValueType::{value.get("value_type", "None")},\n'
                              f'        {"true" if value.get("drop_sign", False) else "false"},\n'
                              f'        {value.get("offset", 0.0)}f,\n'
                              f'        {value.get("scale_factor", 1.0)}f,\n'
                               '    },')

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

        specs_h.append(f'namespace {spec_name.camel}Address {{\n\n'
                       f'enum {{\n{"\n".join(value_addresses)}\n}};\n\n'
                        '}')

        specs_h.append(f'extern const MeterModbusTCP::TableSpec {spec_name.under}_table;')

        specs_cpp.append(f'static const MeterModbusTCP::ValueSpec {spec_name.under}_specs[] = {{\n{"\n".join(value_specs)}\n}};')

        if len(value_ids) > 0:
            specs_cpp.append(f'static const MeterValueID {spec_name.under}_ids[] = {{\n{"\n".join(value_ids)}\n}};')

        specs_cpp.append(f'static const uint32_t {spec_name.under}_index[] = {{\n{"\n".join(value_index)}\n}};')

        specs_cpp.append(f'extern const MeterModbusTCP::TableSpec {spec_name.under}_table = {{\n'
                         f'    MeterLocation::{spec['default_location']},\n'
                         f'    {spec_name.under}_specs,\n'
                         f'    ARRAY_SIZE({spec_name.under}_specs),\r')

        if len(value_ids) > 0:
            specs_cpp.append(f'    {spec_name.under}_ids,\n'
                             f'    ARRAY_SIZE({spec_name.under}_ids),\r')
        else:
            specs_cpp.append('    nullptr,\n'
                             '    0,\r')

        specs_cpp.append(f'    {spec_name.under}_index,\n'
                         f'    {f32_negative_max_as_nan},\n'
                          '};')

for group, value in virtual_meters.items():
    if len(value) == 1 and value[0][0] == None:
        continue

    specs_h.append(f'const MeterModbusTCP::TableSpec *get_{group.under}_table(uint32_t slot, {group.camel}VirtualMeter virtual_meter);')

    specs_cpp.append(f'const MeterModbusTCP::TableSpec *get_{group.under}_table(uint32_t slot, {group.camel}VirtualMeter virtual_meter)\n'
                      '{\n'
                      '    switch (virtual_meter) {\n'
                     f'    case {group.camel}VirtualMeter::None:\n'
                     f'        logger.printfln_meter("No {group.space} Virtual Meter selected");\n'
                      '        return nullptr;')

    for member_spec_name_default_location in value:
        member, spec_name, _ = member_spec_name_default_location

        specs_cpp.append(f'    case {group.camel}VirtualMeter::{member.camel}:\n'
                         f'        return &{spec_name.under}_table;')

    specs_cpp.append(f'    default:\n'
                     f'        logger.printfln_meter("Unknown {group.space} Virtual Meter: %u", static_cast<uint8_t>(virtual_meter));\n'
                      '        return nullptr;\n'
                      '    }\n'
                      '}')

specs_ts.append('export function get_virtual_meter_items(table_id: number) {\n'
                '    let virtual_meter_items: [string, string][] = [];\n\n'
                '    switch (table_id) {\r')

for group, value in virtual_meters.items():
    if len(value) == 1 and value[0][0] == None:
        continue

    specs_ts.append(f'    case MeterModbusTCPTableID.{group.camel}:\n'
                     '        virtual_meter_items = [\r')

    for member_spec_name_default_location in value:
        member, _, _ = member_spec_name_default_location

        specs_ts.append(f'            [{group.camel}VirtualMeter.{member.camel}.toString(), __("meters_modbus_tcp.content.virtual_meter_{member.under}")],\r')

    specs_ts.append('        ];\n'
                    '\n'
                    '        break;')

specs_ts[-1] += '\r'
specs_ts.append('    }\n\n'
                '    return virtual_meter_items;\n'
                '}')

specs_ts.append('export function get_default_location(table_id: number, virtual_meter: number) {\n'
                '    switch (table_id) {\r')

for group, value in virtual_meters.items():
    if len(value) > 1 or value[0][0] != None:
        specs_ts.append(f'    case MeterModbusTCPTableID.{group.camel}:\n'
                         '        switch (virtual_meter) {\r')

        for member_spec_name_default_location in value:
            member, _, default_location = member_spec_name_default_location

            specs_ts.append(f'            case {group.camel}VirtualMeter.{member.camel}: return MeterLocation.{default_location};\r')

        specs_ts.append('        }\n'
                        '\n'
                        '        return MeterLocation.Unknown;')
    else:
        specs_ts.append(f'    case MeterModbusTCPTableID.{group.camel}:\n'
                        f'        return MeterLocation.{value[0][2]};')

specs_ts[-1] += '\r'
specs_ts.append('    }\n\n'
                '    return undefined;\n'
                '}')

ts  = '// WARNING: This file is generated.\n\n'
ts += 'import { __ } from "../../ts/translation";\n'
ts += 'import { MeterModbusTCPTableID } from "./meter_modbus_tcp_table_id.enum";\n'
ts += 'import { MeterLocation } from "../meters/meter_location.enum";\n'
ts += '\n'.join([f'import {{ {group.camel}VirtualMeter }} from "./{group.under}_virtual_meter.enum";' for group, value in virtual_meters.items() if len(value) > 1 or value[0][0] != None]) + '\n\n'
ts += 'export const enum DefaultDeviceAddress {\n'
ts += '\n'.join([f'    {util.FlavoredName(name).get().camel} = {value},' for name, value in default_device_addresses]) + '\n'
ts += '}\n\n'
ts += 'export function get_default_device_address(table_id: number)\n'
ts += '{\n'
ts += '    switch (table_id) {\n'
ts += '\n'.join([f'    case MeterModbusTCPTableID.{util.FlavoredName(name).get().camel}: return DefaultDeviceAddress.{util.FlavoredName(name).get().camel};' for name, value in default_device_addresses]) + '\n'
ts += '    default: return undefined;\n'
ts += '    }\n'
ts += '}\n\n'
ts += 'type TableConfigNone = [\n'
ts += '    MeterModbusTCPTableID.None,\n'
ts += '    {},\n'
ts += '];\n\n'
ts += 'export type Register = {\n'
ts += '    rtype: number; // ModbusRegisterType\n'
ts += '    addr: number;\n'
ts += '    vtype: number; // ModbusValueType\n'
ts += '    off: number;\n'
ts += '    scale: number;\n'
ts += '    id: number; // MeterValueID\n'
ts += '};\n\n'
ts += 'export type TableConfigCustom = [\n'
ts += '    MeterModbusTCPTableID.Custom,\n'
ts += '    {\n'
ts += '        device_address: number;\n'
ts += '        register_address_mode: number; // ModbusRegisterAddressMode\n'
ts += '        registers: Register[];\n'
ts += '    },\n'
ts += '];\n\n'
ts += '\n'.join(table_typedefs) + '\n'
ts += 'export type TableConfig = TableConfigNone |\n'
ts += '                          TableConfigCustom |\n'
ts += '                          ' + ' |\n                          '.join(table_typenames) + ';\n\n'
ts += 'export function new_table_config(table: MeterModbusTCPTableID): TableConfig {\n'
ts += '    switch (table) {\n'
ts += '    case MeterModbusTCPTableID.Custom:\n'
ts += '        return [MeterModbusTCPTableID.Custom, {device_address: 1, register_address_mode: null, registers: []}];\n\n'
ts += '\n'.join(table_news) + '\n'
ts += '    default:\n'
ts += '        return [MeterModbusTCPTableID.None, null];\n'
ts += '    }\n'
ts += '}\n\n'
ts += '\n\n'.join(specs_ts).replace('\r\n', '') + '\n'

tfutil.write_file_if_different('../../../web/src/modules/meters_modbus_tcp/meter_modbus_tcp_specs.ts', ts)

h  = '// WARNING: This file is generated.\n\n'
h += '#pragma once\n\n'
h += '\n'.join([f'#include "{group.under}_virtual_meter.enum.h"' for group, value in virtual_meters.items() if len(value) > 1 or value[0][0] != None]) + '\n'

tfutil.write_file_if_different('meter_modbus_tcp_virtual_meter_enums.h', h)

h  = '// WARNING: This file is generated.\n\n'
h += '#pragma once\n\n'
h += '#include "config.h"\n'
h += '#include "meter_modbus_tcp.h"\n'
h += '#include "meter_modbus_tcp_table_id.enum.h"\n\n'
h += '#define VALUE_INDEX_META  0xFFFFFFFEu\n'
h += '#define VALUE_INDEX_DEBUG 0xFFFFFFFDu\n\n'
h += '#define START_ADDRESS_VIRTUAL 0xFFFFFFFEu\n\n'
h += 'void get_meter_modbus_tcp_table_prototypes(std::vector<ConfUnionPrototype<MeterModbusTCPTableID>> *table_prototypes);\n\n'
h += '\n\n'.join(specs_h).replace('\r\n', '') + '\n'

tfutil.write_file_if_different('meter_modbus_tcp_specs.h', h)

cpp  = '// WARNING: This file is generated.\n\n'
cpp += '#define EVENT_LOG_PREFIX "meters_mbtcp"\n'
cpp += '#define TRACE_LOG_PREFIX nullptr\n\n'
cpp += '#include "meter_modbus_tcp_specs.h"\n\n'
cpp += '#include "event_log_prefix.h"\n'
cpp += '#include "module_dependencies.h"\n'
cpp += '#include "meter_modbus_tcp_virtual_meter_enums.h"\n\n'
cpp += '#include "gcc_warnings.h"\n\n'
cpp += 'void get_meter_modbus_tcp_table_prototypes(std::vector<ConfUnionPrototype<MeterModbusTCPTableID>> *table_prototypes)\n'
cpp += '{'
cpp += '\n'.join(table_prototypes).rstrip() + '\n'
cpp += '}\n\n'
cpp += '\n\n'.join(specs_cpp).replace('\r\n', '') + '\n'

tfutil.write_file_if_different('meter_modbus_tcp_specs.cpp', cpp)
