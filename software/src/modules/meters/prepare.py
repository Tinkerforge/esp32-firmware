import os
import sys
import importlib.util
import importlib.machinery
import csv
import json

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util
from collections import OrderedDict

def escape(s):
    return json.dumps(s)

value_id_enum = []
value_id_list = []
value_id_infos = []
value_id_order = []
value_id_dict = OrderedDict()

groupings = [
    ('L1', 'L2', 'L3'),
    ('L1 N', 'L2 N', 'L3 N'),
    ('L1 L2', 'L2 L3', 'L3 L1'),
]

translation_values = {'en': [], 'de': []}
translation_groups = {'en': [], 'de': []}

with open('meter_value_group.csv', newline='') as f:
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

def update_value_id_dict(sub_dict, sub_id):
    key = sub_id[0]

    if len(sub_id) > 2:
        update_value_id_dict(sub_dict.setdefault(key, OrderedDict()), sub_id[1:])
    else:
        sub_dict[key] = sub_id[1]

with open('meter_value_id.csv', newline='') as f:
    for row in csv.DictReader(f):
        if len(row['id']) == 0:
            # skip empty
            continue

        id_ = row['id']
        name_list = [row[x] for x in ['measurand', 'submeasurand', 'phase', 'direction', 'kind']]
        name = ' '.join([x for x in name_list if len(x) > 0])
        name_without_phase = ' '.join([row[x] for x in ['measurand', 'submeasurand', 'direction', 'kind'] if len(row[x]) > 0])
        phase = row['phase']
        identifier = name.replace(' ', '')
        identifier_without_phase = name_without_phase.replace(' ', '_').lower()
        unit = row['unit']
        digits = row['digits']
        display_name_en       = escape(row['display_name_en'].replace('\"', '\\"') if len(row['display_name_en']) > 0 else ("TRANSLATION_MISSING " + name))
        display_name_en_muted = escape(row['display_name_en_muted'].replace('\"', '\\"'))
        display_name_de       = escape(row['display_name_de'].replace('\"', '\\"') if len(row['display_name_de']) > 0 else ("TRANSLATION_MISSING " + name))
        display_name_de_muted = escape(row['display_name_de_muted'].replace('\"', '\\"'))

        if name_list[4] == "":
            name_list[4] = "Register"

        update_value_id_dict(value_id_dict, [part for part in name_list if len(part) > 0] + [identifier])

        value_id_enum.append(f'    {identifier} = {id_}, // {unit}\n')
        value_id_list.append(f'    MeterValueID.{identifier},\n')
        value_id_infos.append(f'    /* {identifier} */ {id_}: {{unit: "{unit}", digits: {digits}}},\n')
        translation_values['en'].append(f'"value_{id_}": {display_name_en}')
        translation_values['en'].append(f'"value_{id_}_muted": {display_name_en_muted}')
        translation_values['de'].append(f'"value_{id_}": {display_name_de}')
        translation_values['de'].append(f'"value_{id_}_muted": {display_name_de_muted}')

        for phases in groupings:
            if phase in phases:
                for foobar in value_id_order:
                    if foobar[1] == identifier_without_phase and foobar[2] == phases:
                        foobar[0].append(identifier)
                        break
                else:
                    value_id_order.append([[identifier], identifier_without_phase, phases])

                break
        else:
            value_id_order.append([[identifier], None, None])

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

with open('meter_value_id.h', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class MeterValueID {\n')
    f.write(''.join(value_id_enum))
    f.write('};\n')

def format_value_id_dict(sub_dict, indent):
    result = ''

    for sub_id in sub_dict.items():
        result += f"{'    ' * (indent + 1)}'{sub_id[0]}': "

        if isinstance(sub_id[1], dict):
            result += f"{{\n{format_value_id_dict(sub_id[1], indent + 1)}{'    ' * (indent + 1)}}},\n"
        else:
            result += f"MeterValueID.{sub_id[1]},\n"

    return result

with open('../../../web/src/modules/meters/meter_value_id.ts', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export const enum MeterValueID {\n')
    f.write(''.join(value_id_enum))
    f.write('}\n\n')
    f.write('export const METER_VALUE_IDS: MeterValueID[] = [\n')
    f.write(''.join(value_id_list))
    f.write(']\n\n')
    f.write('export const METER_VALUE_INFOS: {[id: number]: {unit: string, digits: 0|1|2|3}} = {\n')
    f.write(''.join(value_id_infos))
    f.write('}\n\n')
    f.write('export const METER_VALUE_ORDER: {ids: MeterValueID[], group: string, phases: string}[] = [\n')
    f.write(''.join(value_id_order_str))
    f.write(']\n\n')
    f.write('export const METER_VALUE_ITEMS = {\n')
    f.write(format_value_id_dict(value_id_dict, 0))
    f.write('}\n')

for lang in translation_values:
    util.specialize_template(f'../../../web/src/modules/meters/translation_{lang}.tsx.template', f'../../../web/src/modules/meters/translation_{lang}.tsx', {
        '{{{values}}}': ',\n            '.join(translation_values[lang]),
        '{{{groups}}}': ',\n            '.join(translation_groups[lang]),
    })

# NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
classes = [
    'None',
    'RS485 Bricklet',
    'EVSE V2',
    'Energy Manager',
    'Push API',
    'Sun Spec',
    'Modbus TCP',
    'MQTT Subscription',
]

class_values = []

for i, name in enumerate(classes):
    class_values.append('    {0} = {1},\n'.format(util.FlavoredName(name).get().camel, i))

with open('meters_defs.h', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include <stdint.h>\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class MeterClassID : uint8_t {\n')
    f.write(''.join(class_values))
    f.write('};\n\n')
    f.write(f'#define METER_CLASSES {len(class_values)}')

with open('../../../web/src/modules/meters/meters_defs.ts', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export const enum MeterClassID {\n')
    f.write(''.join(class_values))
    f.write('}\n')
