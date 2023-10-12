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
from collections import OrderedDict

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
    for row in csv.reader(f):
        if row[0] == 'measurand':
            # skip header
            continue

        if len(row[0]) == 0:
            # skip empty
            continue

        name = ' '.join([part for part in row[:4] if len(part) > 0])
        identifier = name.replace(' ', '_').lower()
        display_name_en = row[4].replace('\"', '\\"')
        display_name_de = row[5].replace('\"', '\\"')

        translation_groups['en'].append(f'"group_{identifier}": "{display_name_en}"')
        translation_groups['de'].append(f'"group_{identifier}": "{display_name_de}"')

with open('meter_value_id.csv', newline='') as f:
    for row in csv.reader(f):
        if row[0] == 'id':
            # skip header
            continue

        if len(row[0]) == 0:
            # skip empty
            continue

        id_ = row[0]
        name = ' '.join([part for part in row[1:6] if len(part) > 0])
        name_without_phase = ' '.join([part for part in (row[1:3] + row[4:6]) if len(part) > 0])
        phase = row[3]
        identifier = name.replace(' ', '')
        identifier_without_phase = name_without_phase.replace(' ', '_').lower()
        unit = row[6]
        digits = row[7]
        display_name_en = row[8].replace('\"', '\\"') if len(row[8]) > 0 else name
        display_name_de = row[9].replace('\"', '\\"') if len(row[9]) > 0 else name

        # FIXME: I need to clean this up.
        if row[1] not in value_id_dict:
            value_id_dict[row[1]] = OrderedDict()[row[3]] = OrderedDict()
        elif row[3] not in value_id_dict[row[1]]:
            value_id_dict[row[1]].update({row[3]: OrderedDict()})

        if not value_id_dict[row[1]].get(row[3]):
            value_id_dict[row[1]][row[3]] = OrderedDict()[row[2]] = OrderedDict()
        elif row[2] not in value_id_dict[row[1]][row[3]]:
            value_id_dict[row[1]][row[3]].update({row[2]: OrderedDict()})

        if not value_id_dict[row[1]][row[3]].get(row[2]):
            value_id_dict[row[1]][row[3]][row[2]] = OrderedDict()[row[4]] = OrderedDict()
        elif row[4] not in value_id_dict[row[1]][row[3]][row[2]]:
            value_id_dict[row[1]][row[3]][row[2]].update({row[4]: OrderedDict()})

        if row[5] == "":
            index = "Acute"
        else:
            index = row[5]
        if not value_id_dict[row[1]][row[3]][row[2]].get(row[4]):
            value_id_dict[row[1]][row[3]][row[2]][row[4]] = OrderedDict()[index] = OrderedDict()
            value_id_dict[row[1]][row[3]][row[2]][row[4]][index] = identifier
        elif index not in value_id_dict[row[1]][row[3]][row[2]][row[4]]:
            value_id_dict[row[1]][row[3]][row[2]][row[4]].update({index: identifier})

        value_id_enum.append(f'    {identifier} = {id_}, // {unit}\n')
        value_id_list.append(f'    MeterValueID.{identifier},\n')
        value_id_infos.append(f'    /* {identifier} */ {id_}: {{unit: "{unit}", digits: {digits}}},\n')
        translation_values['en'].append(f'"value_{id_}": "{display_name_en}"')
        translation_values['de'].append(f'"value_{id_}": "{display_name_de}"')

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
        foobar_str += f'"{"; ".join(foobar[2])}"'
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
    f.write(']\n')

    text = 'export const METER_VALUE_ITEMS = {\n'
    for a in value_id_dict:
        text += "    '" + a + "': {\n"
        for b in value_id_dict[a]:
            if (b != ""):
                text += "        '" + b + "': {\n"
            for c in value_id_dict[a][b]:
                if (c != ""):
                    text += "            '" + c + "': {\n"
                for d in value_id_dict[a][b][c]:
                    if (d != ""):
                        text += "                '" + d + "': {\n"
                    for e in value_id_dict[a][b][c][d]:
                        print(value_id_dict[a][b][c][d])
                        text += "                    '" + e + "': MeterValueID." + value_id_dict[a][b][c][d][e] + ",\n"
                    if (d != ""):
                        text += "                },\n"
                if (c != ""):
                    text += "            },\n"
            if (b != ""):
                text += "        },\n"
        text += "    },\n"

    f.write(text + '}\n')

for lang in translation_values:
    util.specialize_template(f'../../../web/src/modules/meters/translation_{lang}.json.template', f'../../../web/src/modules/meters/translation_{lang}.json', {
        '{{{values}}}': ',\n            '.join(translation_values[lang]),
        '{{{groups}}}': ',\n            '.join(translation_groups[lang]),
    })

# NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
classes = [
    'None',
    'RS485 Bricklet',
    'EVSE V2',
    'EM',
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
