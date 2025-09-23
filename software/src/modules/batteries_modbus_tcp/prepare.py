import sys
import tinkerforge_util as tfutil
import victron_energy
import deye
import alpha_ess
import hailei
import sungrow


tfutil.create_parent_module(__file__, 'software')

from software import util

modules = [
    victron_energy,
    deye,
    alpha_ess,
    hailei,
    sungrow,
]

table_prototypes = []
table_typedefs = []
table_typenames = []
table_news = []
default_device_addresses = []
specs = []
all_actions = {}

for module in modules:
    for table_prototype in module.table_prototypes:
        table_id = util.FlavoredName(table_prototype[0]).get()

        if table_prototype[1] == None:
            table_prototypes.append(f'\n    table_prototypes->push_back({{BatteryModbusTCPTableID::{table_id.camel}, *Config::Null()}});')
        else:
            table_prototypes.append(f'\n    table_prototypes->push_back({{BatteryModbusTCPTableID::{table_id.camel}, Config::Object({{')

            for member in table_prototype[1]:
                if member == 'device_address':
                    table_prototypes.append(f'        {{"device_address", Config::Uint8(DefaultDeviceAddress::{table_id.camel})}},')
                elif isinstance(member, tuple):
                    table_prototypes.append(f'        {{"{member[0]}", {member[1]}}},')
                else:
                    print(f'Error: Table prototype {table_id.space} has unknown member {member}')
                    sys.exit(1)

            table_prototypes.append('    })});')

        table_typedefs.append(f'type TableConfig{table_id.camel} = [\n'
                              f'    BatteryModbusTCPTableID.{table_id.camel},\n    {{')

        table_new = []

        if table_prototype[1] == None:
            table_new.append('null')
        else:
            for member in table_prototype[1]:
                if isinstance(member, tuple):
                    member_name = member[0]
                else:
                    member_name = member

                table_typedefs.append(f'        {member_name}: number;')

                if member_name == 'device_address':
                    table_new.append(f'device_address: DefaultDeviceAddress.{table_id.camel}')
                else:
                    table_new.append(f'{member_name}: null')

        table_typedefs.append('    },\n];\n')

        table_typenames.append(f'TableConfig{table_id.camel}')

        table_news.append(f'    case BatteryModbusTCPTableID.{table_id.camel}:')
        table_news.append(f'        return [BatteryModbusTCPTableID.{table_id.camel}, {{{", ".join(table_new)}}}];\n'.replace('{null}', 'null'))

    default_device_addresses += module.default_device_addresses
    specs += module.specs

specs_h = []
specs_cpp = []

specs_h.append('namespace DefaultDeviceAddress {')
specs_h.append(f'enum {{\n{"\n".join(["    {0} = {1},".format(util.FlavoredName(name).get().camel, value) for name, value in default_device_addresses])}\n}};')
specs_h.append('}')

for spec in specs:
    spec_group = util.FlavoredName(spec['group']).get()
    spec_action = util.FlavoredName(spec['action']).get()

    all_actions.setdefault(spec_group, []).append(spec_action)

    register_block_descs = []
    register_block_specs = []

    for i, register_block in enumerate(spec['register_blocks']):
        if register_block['description'] in register_block_descs:
            print(f'Error: Register block {spec_group.space} / {spec_action.space} / {register_block['description']} is duplicate')
            sys.exit(1)

        register_block_descs.append(register_block['description'])

        start_number = register_block.get('start_number')

        if start_number != None:
            start_address = start_number - 1
        else:
            start_address = register_block['start_address']

        if len(register_block['values']) == 0:
            print(f'Error: Register block {spec_group.space} / {spec_action.space} / {register_block['description']} has no values')
            sys.exit(1)

        register_block_specs.append('    {\n'
                                   f'        ModbusFunctionCode::{register_block["function_code"]},\n'
                                   f'        {start_address},\n'
                                   f'        static_cast<const void *>({spec_group.under}_{spec_action.under}_register_block_{i}_values),\n'
                                   f'        {len(register_block['values'])},\n'
                                    '    },')

        function_code = register_block['function_code']

        if function_code == 'WriteSingleCoil':
            if len(register_block['values']) != 1:
                print(f'Error: Register block {spec_group.space} / {spec_action.space} / {register_block['description']} has invalid value count')
                sys.exit(1)

            if register_block['values'][0] not in [0, 1]:
                print(f'Error: Register block {spec_group.space} / {spec_action.space} / {register_block['description']} has invalid value')
                sys.exit(1)

            specs_cpp.append(f'static const uint8_t {spec_group.under}_{spec_action.under}_register_block_{i}_values[] = {{\n{register_block['values'][0]}\n}};')
        elif function_code == 'WriteMultipleCoils':
            values = [0] * ((len(register_block['values']) + 7) // 8)

            for k, bit in enumerate(register_block['values']):
                if bit not in [0, 1]:
                    print(f'Error: Register block {spec_group.space} / {spec_action.space} / {register_block['description']} has invalid value')
                    sys.exit(1)

                values[k // 8] |= bit << (k % 8)

            specs_cpp.append(f'static const uint8_t {spec_group.under}_{spec_action.under}_register_block_{i}_values[] = {{\n{"\n".join([f"    {value}," for value in values])}\n}};')
        elif function_code in ['WriteSingleRegister', 'WriteMultipleRegisters']:
            specs_cpp.append(f'static const uint16_t {spec_group.under}_{spec_action.under}_register_block_{i}_values[] = {{\n{"\n".join([f"    {value}," for value in register_block["values"]])}\n}};')
        elif function_code == 'MaskWriteRegister':
            assert False  # FIXME
        elif function_code in ['ReadMaskWriteSingleRegister', 'ReadMaskWriteMultipleRegisters']:
            assert False  # FIXME
        else:
            print(f'Error: Register block {spec_group.space} / {spec_action.space} / {register_block['description']} has invalid function code: {function_code}')
            sys.exit(1)

    specs_h.append(f'extern const BatteryModbusTCP::TableSpec {spec_group.under}_{spec_action.under}_table;')

    if len(register_block_specs) > 0:
        specs_cpp.append(f'static const BatteryModbusTCP::RegisterBlockSpec {spec_group.under}_{spec_action.under}_register_blocks[] = {{\n{"\n".join(register_block_specs)}\n}};')

    specs_cpp.append(f'extern const BatteryModbusTCP::TableSpec {spec_group.under}_{spec_action.under}_table = {{\n'
                     f'    {spec["repeat_interval"]},\r')

    if len(register_block_specs) > 0:
        specs_cpp.append(f'    {spec_group.under}_{spec_action.under}_register_blocks,\n'
                         f'    ARRAY_SIZE({spec_group.under}_{spec_action.under}_register_blocks),\r')
    else:
        specs_cpp.append('    nullptr,\n'
                         '    0,\r')

    specs_cpp.append('};')

    """table_prototype = []
    table_typedef = []
    table_new = []

    if spec['table_prototype'] == None:
        table_prototype.append(f'        {{"{spec_action.under}", *Config::Null()}},')
    else:
        table_prototype.append(f'        {{"{spec_action.under}", Config::Object({{')

        for member in spec['table_prototype']:
            if member == 'device_address':
                table_prototype.append(f'            {{"device_address", Config::Uint8(DefaultDeviceAddress::{spec_group.camel}_{spec_action.camel})}},')
            elif isinstance(member, tuple):
                table_prototype.append(f'            {{"{member[0]}", {member[1]}}},')
            else:
                print(f'Error: Table prototype {spec_group.space} / {spec_action.space} has unknown member {member}')
                sys.exit(1)

        table_prototype.append('        })},')

    table_typedef.append(f'        {spec_action.under}: {{')
    table_new.append(f'            {spec_action.under}: {{')

    if spec['table_prototype'] == None:
        table_new.append('null')
    else:
        for member in spec['table_prototype']:
            if isinstance(member, tuple):
                member_name = member[0]
            else:
                member_name = member

            table_typedef.append(f'            {member_name}: number;')

            if member_name == 'device_address':
                table_new.append(f'                device_address: DefaultDeviceAddress.{spec_group.camel}_{spec_action.camel},')
            else:
                table_new.append(f'                {member_name}: null,')

    table_typedef.append('        },')
    table_new.append('            },')

    table_typename = f'TableConfig{spec_group.camel}'

    if table_typename not in table_typenames:
        table_typenames.append(table_typename)

    all_table_prototypes.setdefault(spec_group, []).extend(table_prototype)
    all_table_typedefs.setdefault(spec_group, []).extend(table_typedef)
    all_table_news.setdefault(spec_group, []).extend(table_new)"""

for group, actions in all_actions.items():
    specs_h.append(f'const BatteryModbusTCP::TableSpec *get_{group.under}_table(BatteryAction action);')
    specs_h.append(f'void get_{group.under}_tables(const BatteryModbusTCP::TableSpec *tables[6]);')

    specs_cpp.append('[[gnu::const]]\n'
                    f'const BatteryModbusTCP::TableSpec *get_{group.under}_table(BatteryAction action)\n'
                     '{\n'
                     '    switch (action) {\r')

    for action in actions:
        specs_cpp.append(f'    case BatteryAction::{action.camel}:\n'
                         f'        return &{group.under}_{action.under}_table;')

    specs_cpp.append(f'    default:\n'
                      '        return nullptr;\n'
                      '    }\n'
                      '}')

    specs_cpp.append(f'void get_{group.under}_tables(const BatteryModbusTCP::TableSpec *tables[6])\n'
                      '{\r')

    for action in actions:
        specs_cpp.append(f'    tables[static_cast<size_t>(BatteryAction::{action.camel})] = &{group.under}_{action.under}_table;\r')

    specs_cpp.append('}')

ts  = '// WARNING: This file is generated.\n\n'
ts += 'import { BatteryModbusTCPTableID } from "./battery_modbus_tcp_table_id.enum";\n\n'
ts += 'export const enum DefaultDeviceAddress {\n'
ts += '\n'.join([f'    {util.FlavoredName(name).get().camel} = {value},' for name, value in default_device_addresses]) + '\n'
ts += '}\n\n'
ts += 'export function get_default_device_address(table_id: number)\n'
ts += '{\n'
ts += '    switch (table_id) {\n'
ts += '\n'.join([f'    case BatteryModbusTCPTableID.{util.FlavoredName(name).get().camel}: return DefaultDeviceAddress.{util.FlavoredName(name).get().camel};' for name, value in default_device_addresses]) + '\n'
ts += '    default: return undefined;\n'
ts += '    }\n'
ts += '}\n\n'
ts += 'type TableConfigNone = [\n'
ts += '    BatteryModbusTCPTableID.None,\n'
ts += '    {},\n'
ts += '];\n\n'
ts += 'export type RegisterBlock = {\n'
ts += '    desc: string;\n'
ts += '    func: number; // ModbusFunctionCode\n'
ts += '    addr: number;\n'
ts += '    vals: number[];\n'
ts += '};\n\n'
ts += 'export type RegisterTable = {\n'
ts += '    repeat_interval: number;\n'
ts += '    register_blocks: RegisterBlock[];\n'
ts += '};\n\n'
ts += 'export type TableConfigCustom = [\n'
ts += '    BatteryModbusTCPTableID.Custom,\n'
ts += '    {\n'
ts += '        device_address: number;\n'
ts += '        register_address_mode: number, // ModbusRegisterAddressMode\n'
ts += '        permit_grid_charge: RegisterTable,\n'
ts += '        revoke_grid_charge_override: RegisterTable,\n'
ts += '        forbid_discharge: RegisterTable,\n'
ts += '        revoke_discharge_override: RegisterTable,\n'
ts += '        forbid_charge: RegisterTable,\n'
ts += '        revoke_charge_override: RegisterTable,\n'
ts += '    },\n'
ts += '];\n\n'
ts += '\n'.join(table_typedefs) + '\n'
ts += 'export type TableConfig = TableConfigNone |\n'
ts += '                          TableConfigCustom |\n'
ts += '                          ' + ' |\n                          '.join(table_typenames) + ';\n\n'
ts += 'export function new_table_config(table: BatteryModbusTCPTableID): TableConfig {\n'
ts += '    switch (table) {\n'
ts += '    case BatteryModbusTCPTableID.Custom:\n'
ts += '        return [BatteryModbusTCPTableID.Custom, {\n'
ts += '            device_address: 1,\n'
ts += '            register_address_mode: null,\n'
ts += '            permit_grid_charge: {repeat_interval: 60, register_blocks: []},\n'
ts += '            revoke_grid_charge_override: {repeat_interval: 60, register_blocks: []},\n'
ts += '            forbid_discharge: {repeat_interval: 60, register_blocks: []},\n'
ts += '            revoke_discharge_override: {repeat_interval: 60, register_blocks: []},\n'
ts += '            forbid_charge: {repeat_interval: 60, register_blocks: []},\n'
ts += '            revoke_charge_override: {repeat_interval: 60, register_blocks: []},\n'
ts += '        }];\n\n'
ts += '\n'.join(table_news) + '\n'
ts += '    default:\n'
ts += '        return [BatteryModbusTCPTableID.None, null];\n'
ts += '    }\n'
ts += '}\n'

tfutil.write_file_if_different('../../../web/src/modules/batteries_modbus_tcp/battery_modbus_tcp_specs.ts', ts)

h  = '// WARNING: This file is generated.\n\n'
h += '#include "config.h"\n'
h += '#include "battery_modbus_tcp.h"\n\n'
h += 'void get_battery_modbus_tcp_table_prototypes(std::vector<ConfUnionPrototype<BatteryModbusTCPTableID>> *table_prototypes);\n\n'
h += '\n\n'.join(specs_h).replace('\r\n', '') + '\n'

tfutil.write_file_if_different('battery_modbus_tcp_specs.h', h)

cpp  = '// WARNING: This file is generated.\n\n'
cpp += '#include "battery_modbus_tcp_specs.h"\n\n'
cpp += '#include "tools.h"\n\n'
cpp += '#include "gcc_warnings.h"\n\n'
cpp += 'void get_battery_modbus_tcp_table_prototypes(std::vector<ConfUnionPrototype<BatteryModbusTCPTableID>> *table_prototypes)\n'
cpp += '{'
cpp += '\n'.join(table_prototypes) + '\n'
cpp += '}\n\n'
cpp += '\n\n'.join(specs_cpp).replace('\r\n', '') + '\n'

tfutil.write_file_if_different('battery_modbus_tcp_specs.cpp', cpp)
