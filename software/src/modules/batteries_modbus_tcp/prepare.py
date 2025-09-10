import sys
import tinkerforge_util as tfutil
import victron_energy

tfutil.create_parent_module(__file__, 'software')

from software import util

modules = [
    victron_energy,
]

specs = []

for module in modules:
    specs += module.specs

specs_h = []
specs_cpp = []

all_actions = {}
default_device_addresses = []

for spec in specs:
    spec_group = util.FlavoredName(spec['group']).get()
    spec_action = util.FlavoredName(spec['action']).get()

    all_actions.setdefault(spec_group, []).append(spec_action)

    default_device_addresses.append((spec_group, spec_action, spec['default_device_address']))

    register_block_descs = []
    register_block_specs = []

    for i, register_block in enumerate(spec['register_blocks']):
        if register_block['description'] in register_block_descs:
            print(f'Error: Register block {spec_group.space} / {spec_action.space} / {register_block['description']} is duplicate')
            sys.exit(1)

        register_block_descs.append(register_block['description'])

        start_address_offset = register_block.get('start_address_offset', spec.get('start_address_offset', 0))
        start_address = register_block['start_address'] - start_address_offset

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


    specs_cpp.append(f'static const BatteryModbusTCP::RegisterBlockSpec {spec_group.under}_{spec_action.under}_register_blocks[] = {{\n{"\n".join(register_block_specs)}\n}};')

    specs_cpp.append(f'extern const BatteryModbusTCP::TableSpec {spec_group.under}_{spec_action.under}_table = {{\n'
                     f'    {spec_group.under}_{spec_action.under}_register_blocks,\n'
                     f'    ARRAY_SIZE({spec_group.under}_{spec_action.under}_register_blocks),\n'
                      '};')

for group, actions in all_actions.items():
    specs_h.append(f'const BatteryModbusTCP::TableSpec *get_{group.under}_table(BatteryAction action);')

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

specs_h.append('namespace DefaultDeviceAddress {')
specs_h.append(f'enum {{\n{"\n".join([f"    {group.camel}_{action.camel} = {value}," for group, action, value in default_device_addresses])}\n}};')
specs_h.append('}')

ts  = '// WARNING: This file is generated.\n\n'
ts += 'import { BatteryModbusTCPTableID } from "./battery_modbus_tcp_table_id.enum";\n'
ts += 'import { BatteryAction } from "../batteries/battery_action.enum";\n\n'
ts += 'export const enum DefaultDeviceAddress {\n'
ts += '\n'.join([f'    {group.camel}_{action.camel} = {value},' for group, action, value in default_device_addresses]) + '\n'
ts += '}\n\n'
ts += 'export function get_default_device_address(table_id: number, action: number)\n'
ts += '{\n'
ts += '    switch (table_id) {\n'

for group, actions in all_actions.items():
    ts += f'    case BatteryModbusTCPTableID.{group.camel}:\n'
    ts += '        switch (action) {\n'

    for action in actions:
        ts += f'        case BatteryAction.{action.camel}: return DefaultDeviceAddress.{group.camel}_{action.camel};\n'

    ts += '        default: return undefined;\n'
    ts += '        }\n\n'

ts += '    default: return undefined;\n'
ts += '    }\n'
ts += '}\n'

tfutil.write_file_if_different('../../../web/src/modules/batteries_modbus_tcp/battery_modbus_tcp_specs.ts', ts)

h  = '// WARNING: This file is generated.\n\n'
h += '#include "battery_modbus_tcp.h"\n\n'
h += '\n\n'.join(specs_h).replace('\r\n', '') + '\n'

tfutil.write_file_if_different('battery_modbus_tcp_specs.h', h)

cpp  = '// WARNING: This file is generated.\n\n'
cpp += '#include "battery_modbus_tcp_specs.h"\n\n'
cpp += '#include "tools.h"\n\n'
cpp += '#include "gcc_warnings.h"\n\n'
cpp += '\n\n'.join(specs_cpp).replace('\r\n', '') + '\n'

tfutil.write_file_if_different('battery_modbus_tcp_specs.cpp', cpp)
