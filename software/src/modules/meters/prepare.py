import sys
import csv
import json
from collections import OrderedDict
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

def escape(s):
    return json.dumps(s)

value_id_enum = []
value_id_name = []
value_id_list = []
value_id_infos = []
value_id_order = []
value_id_tree = OrderedDict()

groupings = [
    ('L1', 'L2', 'L3'),
    ('L1 N', 'L2 N', 'L3 N'),
    ('L1 L2', 'L2 L3', 'L3 L1'),
]

translation_values = {'en': [], 'de': []}
translation_groups = {'en': [], 'de': []}
translation_fragments = {'en': [], 'de': []}

with open('meter_value_group.csv', newline='', encoding='utf-8') as f:
    for row in csv.DictReader(f):
        if len(row['measurand']) == 0:
            # skip empty
            continue

        name = ' '.join([part for part in list(row.values())[:4] if len(part) > 0])
        identifier = name.replace(' ', '_').lower()
        display_name_en       = escape(row['display_name_en'].replace('\"', '\\"') if len(row['display_name_en']) > 0 else ("TRANSLATION_MISSING " + name))
        display_name_en_muted = escape(row['display_name_en_muted'].replace('\"', '\\"'))
        display_name_de       = escape(row['display_name_de'].replace('\"', '\\"') if len(row['display_name_en']) > 0 else ("TRANSLATION_MISSING " + name))
        display_name_de_muted = escape(row['display_name_de_muted'].replace('\"', '\\"'))

        translation_groups['en'].append(f'"group_{identifier}": {display_name_en}')
        translation_groups['en'].append(f'"group_{identifier}_muted": {display_name_en_muted}')
        translation_groups['de'].append(f'"group_{identifier}": {display_name_de}')
        translation_groups['de'].append(f'"group_{identifier}_muted": {display_name_de_muted}')

with open('meter_value_fragment.csv', newline='', encoding='utf-8') as f:
    for row in csv.DictReader(f):
        if len(row['fragment']) == 0:
            # skip empty
            continue

        identifier = row['fragment'].replace(' ', '_').lower()
        display_name_en = escape(row['display_name_en'].replace('\"', '\\"') if len(row['display_name_en']) > 0 else ("TRANSLATION_MISSING " + row['fragment']))
        display_name_de = escape(row['display_name_de'].replace('\"', '\\"') if len(row['display_name_en']) > 0 else ("TRANSLATION_MISSING " + row['fragment']))

        translation_fragments['en'].append(f'"fragment_{identifier}": {display_name_en}')
        translation_fragments['de'].append(f'"fragment_{identifier}": {display_name_de}')

def update_value_id_tree(sub_tree, sub_id):
    key = sub_id[0]

    if len(sub_id) > 2:
        update_value_id_tree(sub_tree.setdefault(key, OrderedDict()), sub_id[1:])
    else:
        sub_tree[key] = sub_id[1]

tree_paths = []

measurands = ['None']
measurand_map = []
submeasurands = ['None']
submeasurand_map = []
phases = ['None']
phase_map = []
directions = ['None']
direction_map = []
kinds = ['None']
kind_map = []

last_id = 0
with open('meter_value_id.csv', newline='', encoding='utf-8') as f:
    for row in csv.DictReader(f):
        if len(row['id']) == 0:
            # skip empty
            continue

        id_ = row['id']

        if int(id_) != last_id + 1:
            print(f"meter_value_id.csv is not sorted by ID! Expected {last_id + 1}, got {id_}")
            sys.exit(1)
        last_id = int(id_)

        def sanitize(s):
            result = s.replace(' ', '').replace('*', '')
            if len(result) == 0:
                return "None"
            return result

        for name, keys, values in zip(['measurand', 'submeasurand', 'phase', 'direction', 'kind'],
                                      [measurands, submeasurands, phases, directions, kinds],
                                      [measurand_map, submeasurand_map, phase_map, direction_map, kind_map]):
            x = sanitize(row[name])
            if x not in keys:
                keys.append(x)
            values.append(x)

        name_list = [row[x] for x in ['measurand', 'submeasurand', 'phase', 'direction', 'kind']]
        name = ' '.join([x for x in name_list if len(x) > 0 and not x.startswith('*')])
        name_without_phase = ' '.join([row[x] for x in ['measurand', 'submeasurand', 'direction', 'kind'] if len(row[x]) > 0 and not row[x].startswith('*')])
        phase = row['phase']
        identifier = name.replace(' ', '')
        identifier_without_phase = name_without_phase.replace(' ', '_').lower()
        unit = row['unit']
        digits = row['digits']
        display_name_en       = escape(row['display_name_en'].replace('\"', '\\"') if len(row['display_name_en']) > 0 else ("TRANSLATION_MISSING " + name))
        display_name_en_muted = escape(row['display_name_en_muted'].replace('\"', '\\"'))
        display_name_de       = escape(row['display_name_de'].replace('\"', '\\"') if len(row['display_name_de']) > 0 else ("TRANSLATION_MISSING " + name))
        display_name_de_muted = escape(row['display_name_de_muted'].replace('\"', '\\"'))

        tree_path = [part.replace(' ', '_').replace('*', '').lower() for part in name_list if len(part) > 0] + [identifier]
        tree_path_flat = '.'.join(tree_path[:-1])

        update_value_id_tree(value_id_tree, tree_path)

        if tree_path_flat in tree_paths:
            raise Exception(f'tree path {tree_path_flat} is not unique')

        tree_paths.append(tree_path_flat)

        value_id_enum.append(f'    {identifier} = {id_}, // {unit}\n')
        value_id_name.append(name)
        value_id_list.append(f'    MeterValueID.{identifier},\n')
        value_id_infos.append(f'    /* {identifier} */ {id_}: {{unit: "{unit}", digits: {digits}, tree_path: {tree_path[:-1]}}},\n')
        translation_values['en'].append(f'"value_{id_}": {display_name_en}')
        translation_values['en'].append(f'"value_{id_}_muted": {display_name_en_muted}')
        translation_values['de'].append(f'"value_{id_}": {display_name_de}')
        translation_values['de'].append(f'"value_{id_}_muted": {display_name_de_muted}')

        for phases_ in groupings:
            if phase in phases_:
                for foobar in value_id_order:
                    if foobar[1] == identifier_without_phase and foobar[2] == phases_:
                        foobar[0].append(identifier)
                        break
                else:
                    value_id_order.append([[identifier], identifier_without_phase, phases_])

                break
        else:
            value_id_order.append([[identifier], None, None])

for i, tree_path in enumerate(tree_paths):
    for k, other in enumerate(tree_paths):
        if i == k:
            continue

        if other.startswith(tree_path):
            raise Exception(f'tree path {tree_path} is prefix of {other}')

value_id_order_str = []

for foobar in value_id_order:
    foobar_str = f'    {{ids: [{", ".join([f"MeterValueID.{id_}" for id_ in foobar[0]])}], group: '

    if foobar[1] != None:
        foobar_str += f'"{foobar[1]}"'
    else:
        foobar_str += 'null'

    foobar_str += ', phases: '

    if foobar[2] != None:
        foobar_str += f'"{", ".join([x.replace(" ", "-") for x in foobar[2]])}"'
    else:
        foobar_str += 'null'

    foobar_str += '},\n'

    value_id_order_str.append(foobar_str)

with open('meter_value_id.h', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated by esp32-firmware/software/src/modules/meters/prepare.py\n\n')
    f.write('#pragma once\n\n')
    f.write('#include <stdint.h>\n')
    f.write('#include <stddef.h>\n\n')
    f.write('enum class MeterValueID {\n')
    f.write('    NotSupported = 0,\n')
    f.write(''.join(value_id_enum))
    f.write('};\n\n')
    f.write('const char *getMeterValueName(MeterValueID id);\n\n')

    for name, keys, values in zip(['Measurand', 'Submeasurand', 'Phase', 'Direction', 'Kind'],
                                  [measurands, submeasurands, phases, directions, kinds],
                                  [measurand_map, submeasurand_map, phase_map, direction_map, kind_map]):
        f.write(f'enum class MeterValue{name} : uint8_t {{\n')
        f.write('    ' + ',\n    '.join(keys) + '\n')
        f.write('};\n\n')
        f.write(f'MeterValue{name} getMeterValue{name}(MeterValueID id);\n\n');

with open('meter_value_id.cpp', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated by esp32-firmware/software/src/modules/meters/prepare.py\n\n')
    f.write('#include "meter_value_id.h"\n\n')
    f.write(f'static const char *nameLUT[{len(value_id_name)}] = {{\n')
    f.write('\n'.join([f'    "{name}",' for name in value_id_name]) + '\n')
    f.write('};\n\n')
    f.write('const char *getMeterValueName(MeterValueID id)\n{\n')
    f.write('    size_t idx = (size_t)id;\n\n')
    f.write(f'    if (idx >= {len(value_id_name)}) {{\n')
    f.write('        return "Unknown";\n')
    f.write('    }\n\n')
    f.write('    return nameLUT[idx];\n')
    f.write('}')

    for name, keys, values in zip(['Measurand', 'Submeasurand', 'Phase', 'Direction', 'Kind'],
                                  [measurands, submeasurands, phases, directions, kinds],
                                  [measurand_map, submeasurand_map, phase_map, direction_map, kind_map]):
        f.write(f"""

static const uint8_t {name.lower()}LUT[{len(values)}] = {{{
    ", ".join([str(list(keys).index(x)) for x in values])
}}};

MeterValue{name} getMeterValue{name}(MeterValueID id)
{{
    size_t idx = (size_t)id;

    if (idx >= {len(values)}) {{
        return MeterValue{name}::None;
    }}

    return (MeterValue{name}){name.lower()}LUT[idx];
}}
""")

def format_value_id_tree(sub_tree, indent):
    result = ''

    for sub_id in sub_tree.items():
        result += f"{'    ' * (indent + 1)}'{sub_id[0]}': "

        if isinstance(sub_id[1], dict):
            result += f"{{\n{format_value_id_tree(sub_id[1], indent + 1)}{'    ' * (indent + 1)}}},\n"
        else:
            result += f"MeterValueID.{sub_id[1]},\n"

    return result

with open('../../../web/src/modules/meters/meter_value_id.ts', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export const enum MeterValueID {\n')
    f.write(''.join(value_id_enum))
    f.write('}\n\n')
    f.write('export const METER_VALUE_IDS: MeterValueID[] = [\n')
    f.write(''.join(value_id_list))
    f.write(']\n\n')
    f.write('export const METER_VALUE_INFOS: {[id: number]: {unit: string, digits: 0|1|2|3, tree_path: string[]}} = {\n')
    f.write(''.join(value_id_infos))
    f.write('}\n\n')
    f.write('export const METER_VALUE_ORDER: {ids: MeterValueID[], group: string, phases: string}[] = [\n')
    f.write(''.join(value_id_order_str))
    f.write(']\n\n')
    f.write('export type MeterValueTreeType = {[key: string]: MeterValueTreeType | MeterValueID};\n\n')
    f.write('export const METER_VALUE_TREE: MeterValueTreeType = {\n')
    f.write(format_value_id_tree(value_id_tree, 0))
    f.write('}\n')

for lang in translation_values:
    tfutil.specialize_template(f'../../../web/src/modules/meters/translation_{lang}.tsx.template', f'../../../web/src/modules/meters/translation_{lang}.tsx', {
        '{{{values}}}': ',\n            '.join(translation_values[lang]),
        '{{{groups}}}': ',\n            '.join(translation_groups[lang]),
        '{{{fragments}}}': ',\n            '.join(translation_fragments[lang]),
    })
