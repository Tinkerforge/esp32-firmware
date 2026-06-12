import sys
import tinkerforge_util as tfutil
import victron_energy
import deye
import alpha_ess
import hailei
import sungrow
import sma
import solis
import sax_power
import growatt
import kostal

tfutil.create_parent_module(__file__, 'software')

from software import util

modules = [
    victron_energy,
    deye,
    alpha_ess,
    hailei,
    sungrow,
    sma,
    solis,
    sax_power,
    growatt,
    kostal,
]

display_names = {}
table_ids = []
table_prototypes = []
test_table_prototypes = []
standard_table_members = {}
table_typedefs = []
table_typenames = []
table_news = []
table_imports = []
default_device_addresses = []
repeat_intervals = {}
specs = []
all_table_prototypes = {}
all_modes = {}
translation_tables = {}
variants = {}

for module in modules:
    for display_name in module.display_names:
        display_names[display_name[0]] = display_name[1]

        table_id = util.FlavoredName(display_name[0]).get()

        for lang in display_name[1]:
            translation_tables.setdefault(lang, []).append(f'"table_{table_id.under}": "{display_name[1][lang]}"')

    for table_prototype in module.table_prototypes:
        table_id = util.FlavoredName(table_prototype[0]).get()

        table_ids.append(table_id)

        if table_prototype[1] == None:
            table_prototypes.append(f'\n    table_prototypes->push_back({{BatteryModbusTCPTableID::{table_id.camel}, *Config::Null()}});')

            test_table_prototypes.append(f'\n    test_table_prototypes->push_back({{BatteryModbusTCPTableID::{table_id.camel}, Config::Object({{')
            test_table_prototypes.append('        {"mode", Config::Enum(BatteryMode::Block)},')
            test_table_prototypes.append('    })});')
        else:
            table_prototypes.append(f'\n    table_prototypes->push_back({{BatteryModbusTCPTableID::{table_id.camel}, Config::Object({{')
            test_table_prototypes.append(f'\n    test_table_prototypes->push_back({{BatteryModbusTCPTableID::{table_id.camel}, Config::Object({{')
            test_table_prototypes.append('        {"mode", Config::Enum(BatteryMode::Block)},')

            for member in table_prototype[1]:
                if isinstance(member, dict):
                    member_name = member['name']
                else:
                    member_name = member

                if member_name == 'device_address':
                    member_prototype = f'Config::Uint8(DefaultDeviceAddress::{table_id.camel})'
                    standard_table_members.setdefault(table_id.space, []).append(member_name)
                elif isinstance(member, dict):
                    member_prototype = f"Config::{member['type']}({member['default']})"
                else:
                    print(f'Error: Table prototype {table_id.space} has unknown member {member_name}')
                    sys.exit(1)

                table_prototypes.append(f'        {{"{member_name}", {member_prototype}}},')
                test_table_prototypes.append(f'        {{"{member_name}", {member_prototype}}},')

            table_prototypes.append('    })});')
            test_table_prototypes.append('    })});')

        table_typedefs.append(f'type TableConfig{table_id.camel} = [\n'
                              f'    BatteryModbusTCPTableID.{table_id.camel},\n    {{')

        table_new = []
        table_import_type_check = []
        table_import_assignment = []

        if table_prototype[1] == None:
            table_new.append('null')
        else:
            for member in table_prototype[1]:
                if isinstance(member, dict):
                    member_name = member['name']
                else:
                    member_name = member

                table_typedefs.append(f'        {member_name}: number;')

                if member_name == 'device_address':
                    member_default = (f'DefaultDeviceAddress.{table_id.camel}')
                elif isinstance(member, dict):
                    member_default = member['default']
                else:
                    print(f'Error: Table prototype {table_id.space} has unknown member {member_name}')
                    sys.exit(1)

                table_new.append(f'{member_name}: {member_default}')

                table_import_type_check.append(f'        if (typeof table_in[1].{member_name} != "number") {{\n'
                                               f'            console.log("Batteries Modbus/TCP: Imported config {member_name} is not a number");\n'
                                                '            return null;\n'
                                                '        }')

                table_import_assignment.append(f'            {member_name}: table_in[1].{member_name},\r')

                if isinstance(member, dict):
                    all_table_prototypes.setdefault(table_prototype[0], []).append((member['name'], member['type']))

        table_typedefs.append('    },\n];\n')

        table_typenames.append(f'TableConfig{table_id.camel}')

        table_news.append(f'    case BatteryModbusTCPTableID.{table_id.camel}:\r')
        table_news.append(f'        return [BatteryModbusTCPTableID.{table_id.camel}, {{{", ".join(table_new)}}}];'.replace('{null}', 'null'))

        table_imports.append(f'    case BatteryModbusTCPTableID.{table_id.camel}:\r')
        table_imports += table_import_type_check
        table_imports.append(f'        table_out = [BatteryModbusTCPTableID.{table_id.camel}, {{\r')
        table_imports += table_import_assignment
        table_imports.append('        }];\n\n'
                             '        break;')

    default_device_addresses += module.default_device_addresses

    for group_, repeat_interval in module.repeat_intervals:
        group = util.FlavoredName(group_).get()

        assert group not in repeat_intervals, (group_, repeat_interval)

        repeat_intervals[group] = repeat_interval

    for group_, variant_list in module.variants:
        group = util.FlavoredName(group_).get()

        assert group not in variants, (group_, variant_list)

        variants[group] = [util.FlavoredName(variant).get() for variant in variant_list]

    specs += module.specs

for group, variant_list in variants.items():
    values = []

    for i, variant in enumerate(variant_list):
        values.append(util.EnumValue(variant, i))

    util.generate_enum(f'{group.space} Variant.uint8.enum', util.FlavoredName('Batteries Modbus TCP').get(), util.FlavoredName(f'{group.space} Variant').get(), 'uint8_t', values)

specs_h = []
specs_cpp = []
setup_inc = []
load_tables_inc = []
test_setup_inc = []
test_load_table_inc = []

specs_h.append('namespace DefaultDeviceAddress {')
specs_h.append(f'enum {{\n{"\n".join(["    {0} = {1},".format(util.FlavoredName(name).get().camel, value) for name, value in default_device_addresses])}\n}};')
specs_h.append('}')

for spec in specs:
    group = util.FlavoredName(spec['group']).get()
    variant = spec.get('variant')
    variant_name_under = ''

    if variant != None:
        variant = util.FlavoredName(variant).get()
        variant_name_under = f'_{variant.under}'
        variant_name_slash = f'/ {variant.space}'

    mode = util.FlavoredName(spec['mode']).get()
    effective_mode = util.FlavoredName(spec.get('effective_mode', spec['mode'])).get()

    register_block_descs = []
    total_buffer_length = 0
    has_mapping = False

    for i, register_block in enumerate(spec['register_blocks']):
        if register_block['description'] in register_block_descs:
            print(f'Error: Register block {group.space} / {mode.space} / {register_block['description']} is duplicate')
            sys.exit(1)

        register_block_descs.append(register_block['description'])

        values_count = len(register_block['values'])

        if values_count == 0:
            print(f'Error: Register block {group.space} / {mode.space} / {register_block['description']} has no values')
            sys.exit(1)

        function_code = register_block['function_code']

        if function_code == 'WriteSingleCoil' or function_code == 'WriteMultipleCoils':
            total_buffer_length += (values_count + 7) // 8
        else:
            total_buffer_length += values_count * 2

        for value in register_block['values']:
            if isinstance(value, str):
                has_mapping = True

    all_modes.setdefault(group, []).append((variant, mode, has_mapping))

    if has_mapping:
        specs_cpp.append(f'static BatteryModbusTCP::TableSpec *load_{group.under}{variant_name_under}_{mode.under}_table(const Config *config)\n'
                          '{\n'
                         f'    BatteryModbusTCP::RegisterBlockSpec *register_blocks = static_cast<BatteryModbusTCP::RegisterBlockSpec *>(malloc_psram_or_dram(sizeof(BatteryModbusTCP::RegisterBlockSpec) * {len(spec['register_blocks'])}));\n'
                         f'    uint8_t *total_buffer = static_cast<uint8_t *>(malloc_psram_or_dram({total_buffer_length}));')

        total_buffer_offset = 0

        for i, register_block in enumerate(spec['register_blocks']):
            start_number = register_block.get('start_number')

            if start_number != None:
                start_address = start_number - 1
            else:
                start_address = register_block['start_address']

            values_count = len(register_block['values'])

            if values_count == 0:
                print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has no values')
                sys.exit(1)

            if start_address < 0 or start_address + values_count > 65535 + 1:
                print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has invalid start address {start_address}')
                sys.exit(1)

            function_code = register_block['function_code']

            specs_cpp.append(f'    register_blocks[{i}].function_code = ModbusFunctionCode::{function_code};\n'
                             f'    register_blocks[{i}].start_address = {start_address};\n'
                             f'    register_blocks[{i}].values_count = {values_count};\n'
                             f'    register_blocks[{i}].buffer = total_buffer + {total_buffer_offset};\n\n'
                              '    {\r')

            for name_type in all_table_prototypes[spec['group']]:
                for value in register_block['values']:
                    if isinstance(value, str) and name_type[0] in value:
                        specs_cpp.append(f'        {name_type[1].lower()}_t {name_type[0]} = config->get("{name_type[0]}")->as{name_type[1].replace('32', '')}();\r')
                        break

            if function_code == 'WriteSingleCoil' or function_code == 'WriteMultipleCoils':
                buffer_length = (values_count + 7) // 8
            else:
                buffer_length = values_count * 2

            if function_code == 'WriteSingleCoil':
                specs_cpp.append('        uint8_t values[1];')

                if values_count != 1:
                    print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has invalid value count')
                    sys.exit(1)

                if not isinstance(register_block['values'][0], str) and register_block['values'][0] not in [0, 1]:
                    print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has invalid value')
                    sys.exit(1)

                specs_cpp.append(f'        values[0] = {register_block["values"][0]};\r')
            elif function_code == 'WriteMultipleCoils':
                specs_cpp.append(f'        uint8_t values[{values_count}];')

                for k, _ in enumerate(register_block['values']):
                    specs_cpp.append(f'        values[{k}] = {register_block["values"][k]};\r')
            elif function_code in ['WriteSingleRegister', 'WriteMultipleRegisters']:
                specs_cpp.append(f'        uint16_t *values = static_cast<uint16_t *>(register_blocks[{i}].buffer);')

                for k, _ in enumerate(register_block['values']):
                    specs_cpp.append(f'        values[{k}] = {register_block["values"][k]};\r')
            elif function_code in ['MaskWriteRegister', 'ReadMaskWriteSingleRegister']:
                specs_cpp.append(f'        uint16_t *values = static_cast<uint16_t *>(register_blocks[{i}].buffer);')

                for k, _ in enumerate(register_block['values']):
                    specs_cpp.append(f'        values[{k}] = {register_block["values"][k]};\r')
            elif function_code == 'ReadMaskWriteMultipleRegisters':
                assert False  # FIXME
            else:
                print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has invalid function code: {function_code}')
                sys.exit(1)

            #specs_cpp[-1] = specs_cpp[-1].rstrip()

            if function_code == 'WriteSingleCoil':
                specs_cpp.append(f'        uint8_t *coils_buffer = static_cast<uint8_t *>(register_blocks[{i}].buffer);\n\n'
                                  '        coils_buffer[0] = values[0] ? 1 : 0;')
            elif function_code == 'WriteMultipleCoils':
                specs_cpp.append(f'        uint8_t *coils_buffer = static_cast<uint8_t *>(register_blocks[{i}].buffer);\n\n'
                                 f'        coils_buffer[{buffer_length - 1}] = 0;\n\n'
                                 f'        for (uint16_t k = 0; k < {values_count}; ++k) {{\n'
                                 f'            uint8_t mask = static_cast<uint8_t>(1u << (k % 8));\n\n'
                                  '            if (values[k] != 0) {\n'
                                  '                coils_buffer[k / 8] |= mask;\n'
                                  '            }\n'
                                  '            else {\n'
                                  '                coils_buffer[k / 8] &= ~mask;\n'
                                  '            }\n'
                                  '        }')

            specs_cpp[-1] += '\r'
            specs_cpp.append('    }')

            total_buffer_offset += buffer_length

        specs_cpp.append('    BatteryModbusTCP::TableSpec *table = static_cast<BatteryModbusTCP::TableSpec *>(malloc_psram_or_dram(sizeof(BatteryModbusTCP::TableSpec)));\n\n'
                        f'    table->effective_mode = BatteryMode::{effective_mode.camel};\n'
                        f'    table->register_blocks = register_blocks;\n'
                        f'    table->register_blocks_count = {len(spec['register_blocks'])};\n\n'
                         '    return table;\n'
                         '}')
    else:
        register_block_specs = []

        for i, register_block in enumerate(spec['register_blocks']):
            start_number = register_block.get('start_number')

            if start_number != None:
                start_address = start_number - 1
            else:
                start_address = register_block['start_address']

            if len(register_block['values']) == 0:
                print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has no values')
                sys.exit(1)

            function_code = register_block['function_code']
            values_count_scale = 1

            if function_code == 'WriteSingleCoil':
                if len(register_block['values']) != 1:
                    print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has invalid value count')
                    sys.exit(1)

                if register_block['values'][0] not in [0, 1]:
                    print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has invalid value')
                    sys.exit(1)

                specs_cpp.append(f'static const uint8_t {group.under}{variant_name_under}_{mode.under}_register_block_{i}_values[] = {{ {register_block['values'][0]} }};')
            elif function_code == 'WriteMultipleCoils':
                values = [0] * ((len(register_block['values']) + 7) // 8)

                for k, bit in enumerate(register_block['values']):
                    if bit not in [0, 1]:
                        print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has invalid value')
                        sys.exit(1)

                    values[k // 8] |= bit << (k % 8)

                specs_cpp.append(f'static const uint8_t {group.under}{variant_name_under}_{mode.under}_register_block_{i}_values[] = {{\n{"\n".join([f"    {value}," for value in values])}\n}};')
            elif function_code in ['WriteSingleRegister', 'WriteMultipleRegisters']:
                specs_cpp.append(f'static const uint16_t {group.under}{variant_name_under}_{mode.under}_register_block_{i}_values[] = {{\n{"\n".join([f"    {value}," for value in register_block["values"]])}\n}};')
            elif function_code == 'MaskWriteRegister':
                if len(register_block['values']) != 2:
                    print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has invalid value count')
                    sys.exit(1)

                specs_cpp.append(f'static const uint16_t {group.under}{variant_name_under}_{mode.under}_register_block_{i}_values[] = {{ {", ".join([str(value) for value in register_block["values"]])} }};')
            elif function_code == 'ReadMaskWriteSingleRegister':
                if len(register_block['values']) != 2:
                    print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has invalid value count')
                    sys.exit(1)

                specs_cpp.append(f'static const uint16_t {group.under}{variant_name_under}_{mode.under}_register_block_{i}_values[] = {{ {", ".join([str(value) for value in register_block["values"]])} }};')

                values_count_scale = 2
            elif function_code == 'ReadMaskWriteMultipleRegisters':
                assert False  # FIXME
            else:
                print(f'Error: Register block {group.space}{variant_name_slash} / {mode.space} / {register_block['description']} has invalid function code: {function_code}')
                sys.exit(1)

            register_block_specs.append('    {\n'
                                       f'        ModbusFunctionCode::{register_block["function_code"]},\n'
                                       f'        {start_address},\n'
                                       f'        const_cast<void *>(static_cast<const void *>({group.under}_{mode.under}_register_block_{i}_values)),\n'
                                       f'        {len(register_block['values']) // values_count_scale},\n'
                                        '    },')

        if len(register_block_specs) > 0:
            specs_cpp.append(f'static const BatteryModbusTCP::RegisterBlockSpec {group.under}{variant_name_under}_{mode.under}_register_blocks[] = {{\n{"\n".join(register_block_specs)}\n}};')

        specs_cpp.append(f'static const BatteryModbusTCP::TableSpec {group.under}{variant_name_under}_{mode.under}_table = {{\n'
                         f'    BatteryMode::{effective_mode.camel},\r')

        if len(register_block_specs) > 0:
            specs_cpp.append(f'    const_cast<BatteryModbusTCP::RegisterBlockSpec *>({group.under}{variant_name_under}_{mode.under}_register_blocks),\n'
                             f'    ARRAY_SIZE({group.under}{variant_name_under}_{mode.under}_register_blocks),\r')
        else:
            specs_cpp.append('    nullptr,\n'
                             '    0,\r')

        specs_cpp.append('};')

for group, modes in all_modes.items():
    has_any_variant = len(variants.get(group, [])) > 0
    has_any_mapping = any([x[2] for x in modes])

    if has_any_variant:
        variant_suffix = f', {group.camel}Variant variant'
    else:
        variant_suffix = ''

    if has_any_mapping:
        mapping_suffix = ', const Config *config'
    else:
        mapping_suffix = ''

    specs_h.append(f'void load_{group.under}_table(BatteryModbusTCP::TableSpec **table_ptr{variant_suffix}, BatteryMode mode{mapping_suffix});')
    specs_h.append(f'void load_{group.under}_tables(BatteryModbusTCP::TableSpec *tables[6]{variant_suffix}{mapping_suffix});')

    specs_cpp.append(f'void load_{group.under}_table(BatteryModbusTCP::TableSpec **table_ptr{variant_suffix}, BatteryMode mode{mapping_suffix})\n'
                      '{\r')

    if has_any_variant:
        indent = '    '

        specs_cpp.append('    switch (variant) {\r')
    else:
        indent = ''

    for variant in variants.get(group, [None]):
        if variant != None:
            specs_cpp.append(f'    case {group.camel}Variant::{variant.camel}:\r')

        specs_cpp.append(f'{indent}    switch (mode) {{\n'
                         f'{indent}    case BatteryMode::Discover:\n'
                         f'{indent}    case BatteryMode::None:\n'
                         f'{indent}        esp_system_abortf<64>("Invalid battery mode for loading table: %d", static_cast<int>(mode));')

        expected_modes = [util.FlavoredName(x).get() for x in ['Block', 'Normal', 'Block Discharge', 'Force Charge', 'Block Charge', 'Force Discharge']]

        for variant_mode_has_mapping in modes:
            variant_, mode, has_mapping = variant_mode_has_mapping

            if variant_ != variant:
                continue

            expected_modes.remove(mode)

            specs_cpp.append(f'{indent}    case BatteryMode::{mode.camel}:\r')

            if variant != None:
                variant_name_under = f'_{variant.under}'
            else:
                variant_name_under = ''

            if has_mapping:
                specs_cpp.append(f'{indent}        *table_ptr = load_{group.under}{variant_name_under}_{mode.under}_table(config);\n'
                                 f'{indent}        break;')
            else:
                specs_cpp.append(f'{indent}        *table_ptr = const_cast<BatteryModbusTCP::TableSpec *>(&{group.under}{variant_name_under}_{mode.under}_table);\n'
                                 f'{indent}        break;')

        for mode in expected_modes:
            specs_cpp.append(f'{indent}    case BatteryMode::{mode.camel}:\n'
                             f'{indent}        *table_ptr = nullptr;\n'
                             f'{indent}        break;')

        specs_cpp.append(f'{indent}    default:\n'
                         f'{indent}        esp_system_abortf<64>("Unknown battery mode in loading table call: %d", static_cast<int>(mode));\n'
                         f'{indent}    }}\r')

        if variant != None:
            specs_cpp[-1] = specs_cpp[-1].rstrip()
            specs_cpp.append('        break;')

    if has_any_variant:
        specs_cpp.append(f'    default:\n'
                          '        *table_ptr = nullptr;\n'
                          '        break;\n'
                          '    }\r')

    specs_cpp.append('}')
    specs_cpp.append(f'void load_{group.under}_tables(BatteryModbusTCP::TableSpec *tables[6]{variant_suffix}{mapping_suffix})\n'
                      '{\r')

    expected_modes = [util.FlavoredName(x).get() for x in ['Block', 'Normal', 'Block Discharge', 'Force Charge', 'Block Charge', 'Force Discharge']]

    for variant_mode_has_mapping in modes:
        variant_, mode, has_mapping = variant_mode_has_mapping

        if mode not in expected_modes:
            continue

        expected_modes.remove(mode)

        specs_cpp.append(f'    load_{group.under}_table(&tables[static_cast<size_t>(BatteryMode::{mode.camel})]{", variant" if has_any_variant else ""}, BatteryMode::{mode.camel}{", config" if has_any_mapping else ""});\r')

    for mode in expected_modes:
        specs_cpp.append(f'    tables[static_cast<size_t>(BatteryMode::{mode.camel})] = nullptr;\r')

    specs_cpp.append('}')

    setup_inc.append(f'    case BatteryModbusTCPTableID::{group.camel}:\n')

    for member_name in standard_table_members.get(group.space, []):
        if member_name == 'device_address':
            setup_inc.append('        device_address = table_config->get("device_address")->asUint8();\n')
        else:
            print(f'Error: Table prototype {group.space} has unknown member {member_name}')
            sys.exit(1)

    setup_inc.append(f'        repeat_interval = {repeat_intervals[group]};\n'
                      '        break;\n\n')

    load_tables_inc.append(f'    case BatteryModbusTCPTableID::{group.camel}:\n'
                           f'        load_{group.under}_tables(tables{f", {group.under}_variant" if has_any_variant else ""}{", table_config" if has_any_mapping else ""});\n'
                            '        break;\n\n')

    test_setup_inc.append(f'    case BatteryModbusTCPTableID::{group.camel}:\n'
                           '        test->mode = table_config->get("mode")->asEnum<BatteryMode>();\n')

    for member_name in standard_table_members.get(group.space, []):
        if member_name == 'device_address':
            test_setup_inc.append('        test->device_address = table_config->get("device_address")->asUint8();\n')
        else:
            print(f'Error: Table prototype {group.space} has unknown member {member_name}')
            sys.exit(1)

    test_setup_inc.append(f'        test->repeat_interval = {repeat_intervals[group]};\n'
                           '        break;\n\n')

    test_load_table_inc.append(f'    case BatteryModbusTCPTableID::{group.camel}:\n'
                               f'        load_{group.under}_table(&test->table{f", test->{group.under}_variant" if has_any_variant else ""}, test->mode{", table_config" if has_any_mapping else ""});\n'
                                '        break;\n\n')

ts  = '// WARNING: This file is generated.\n\n'
ts += 'import { BatteryModbusTCPTableID } from "./battery_modbus_tcp_table_id.enum";\n\n'
ts += 'import { BatteryMode } from "../../batteries/generated/battery_mode.enum";\n\n'
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
ts += '    effective_mode: number; // BatteryMode\n'
ts += '    register_blocks: RegisterBlock[];\n'
ts += '};\n\n'
ts += 'export type TableConfigCustom = [\n'
ts += '    BatteryModbusTCPTableID.Custom,\n'
ts += '    {\n'
ts += '        device_address: number,\n'
ts += '        repeat_interval: number, // seconds\n'
ts += '        register_address_mode: number, // ModbusRegisterAddressMode\n'
ts += '        battery_modes: RegisterTable[],\n'
ts += '    },\n'
ts += '];\n\n'
ts += '\n'.join(table_typedefs) + '\n'
ts += 'export type TableConfig = TableConfigNone |\n'
ts += '                          TableConfigCustom |\n'
ts += '                          ' + ' |\n                          '.join(table_typenames) + ';\n\n'
ts += 'export function new_table_config(table_id: BatteryModbusTCPTableID): TableConfig {\n'
ts += '    switch (table_id) {\n'
ts += '    case BatteryModbusTCPTableID.Custom:\n'
ts += '        return [BatteryModbusTCPTableID.Custom, {\n'
ts += '            device_address: 1,\n'
ts += '            repeat_interval: 60,\n'
ts += '            register_address_mode: null,\n'
ts += '            battery_modes: [\n'
ts += '                {effective_mode: BatteryMode.Block, register_blocks: []},\n'
ts += '                {effective_mode: BatteryMode.Normal, register_blocks: []},\n'
ts += '                {effective_mode: BatteryMode.BlockDischarge, register_blocks: []},\n'
ts += '                {effective_mode: BatteryMode.ForceCharge, register_blocks: []},\n'
ts += '                {effective_mode: BatteryMode.BlockCharge, register_blocks: []},\n'
ts += '                {effective_mode: BatteryMode.ForceDischarge, register_blocks: []},\n'
ts += '            ],\n'
ts += '        }];\n\n'
ts += '\n\n'.join(table_news).replace('\r\n', '') + '\n\n'
ts += '    default:\n'
ts += '        return [BatteryModbusTCPTableID.None, null];\n'
ts += '    }\n'
ts += '}\n\n'
ts += 'export function import_table_config(table_in: TableConfig): TableConfig {\n'
ts += '    let table_out: TableConfig;\n\n'
ts += '    switch (table_in[0]) {\n'
ts += '\n\n'.join(table_imports).replace('\r\n', '') + '\n\n'
ts += '    default:\n'
ts += '        console.log("Batteries Modbus/TCP: Imported config table has unknown class:", table_in[0]);\n'
ts += '        return null;\n'
ts += '    }\n\n'
ts += '    return table_out;\n'
ts += '}\n'

util.write_generated_file('../../../web/src/modules/batteries_modbus_tcp/generated/battery_modbus_tcp_specs.ts', ts)

h  = '// WARNING: This file is generated.\n\n'
h += '#include "config.h"\n'
h += '#include "../battery_modbus_tcp.h"\n'
h += '\n'.join([f'#include "{group.under}_variant.enum.h"' for group in variants.keys()]) + '\n\n'
h += 'void get_battery_modbus_tcp_table_prototypes(std::vector<ConfUnionPrototype<BatteryModbusTCPTableID>> *table_prototypes);\n\n'
h += 'void get_battery_modbus_tcp_test_table_prototypes(std::vector<ConfUnionPrototype<BatteryModbusTCPTableID>> *test_table_prototypes);\n\n'
h += '\n\n'.join(specs_h).replace('\r\n', '') + '\n'

util.write_generated_file('generated/battery_modbus_tcp_specs.h', h)

cpp  = '// WARNING: This file is generated.\n\n'
cpp += '#include "battery_modbus_tcp_specs.h"\n\n'
cpp += '#include <bit>\n\n'
cpp += '#include "tools.h"\n'
cpp += '#include "tools/malloc.h"\n\n'
cpp += '#include "gcc_warnings.h"\n\n'
cpp += 'void get_battery_modbus_tcp_table_prototypes(std::vector<ConfUnionPrototype<BatteryModbusTCPTableID>> *table_prototypes)\n'
cpp += '{'
cpp += '\n'.join(table_prototypes) + '\n'
cpp += '}\n\n'
cpp += 'void get_battery_modbus_tcp_test_table_prototypes(std::vector<ConfUnionPrototype<BatteryModbusTCPTableID>> *test_table_prototypes)\n'
cpp += '{'
cpp += '\n'.join(test_table_prototypes) + '\n'
cpp += '}\n\n'
cpp += '\n\n'.join(specs_cpp).replace('\r\n', '') + '\n'

util.write_generated_file('generated/battery_modbus_tcp_specs.cpp', cpp)

util.write_generated_file('generated/battery_modbus_tcp_setup.inc', ''.join(setup_inc))

util.write_generated_file('generated/battery_modbus_tcp_load_tables.inc', ''.join(load_tables_inc))

util.write_generated_file('generated/batteries_modbus_tcp_test_setup.inc', ''.join(test_setup_inc))

util.write_generated_file('generated/batteries_modbus_tcp_test_load_table.inc', ''.join(test_load_table_inc))

table_ids_items_rpl = ', '.join([f'[BatteryModbusTCPTableID.{table_id.camel}.toString(), __("batteries_modbus_tcp.content.table_{table_id.under}")]' for table_id in table_ids])

util.write_generated_file('../../../web/src/modules/batteries_modbus_tcp/generated/battery_modbus_tcp_specific_table_ids_items.rpl', table_ids_items_rpl)

table_ids_condition_rpl = ' || '.join([f'config[1].table[0] == BatteryModbusTCPTableID.{table_id.camel}' for table_id in table_ids])

util.write_generated_file('../../../web/src/modules/batteries_modbus_tcp/generated/battery_modbus_tcp_specific_table_ids_condition.rpl', table_ids_condition_rpl)

for lang in translation_tables:
    tfutil.specialize_template(f'../../../web/src/modules/batteries_modbus_tcp/translation_{lang}.tsx.template', f'../../../web/src/modules/batteries_modbus_tcp/translation_{lang}.tsx', {
        '{{{tables}}}': ',\n            '.join(translation_tables[lang]),
    })
