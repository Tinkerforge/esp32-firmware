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

enum_values = []
enum_names = []
enum_infos = []
detailed_values = {'en': [], 'de': []}

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
        identifier = name.replace(' ', '')
        unit = row[6]
        digits = row[7]

        enum_values.append(f'    {identifier} = {id_}, // {unit}\n')
        enum_names.append(f'    MeterValueID.{identifier},\n')
        enum_infos.append(f'    /* {identifier} */ {id_}: {{unit: "{unit}", digits: {digits}}},\n')
        detailed_values['en'].append(f'"detailed_{id_}": "{name}"')
        detailed_values['de'].append(f'"detailed_{id_}": "{name}"')

with open('meter_value_id.h', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class MeterValueID {\n')
    f.write(''.join(enum_values))
    f.write('};\n')

with open('../../../web/src/modules/meters/meter_value_id.ts', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export const enum MeterValueID {\n')
    f.write(''.join(enum_values))
    f.write('}\n\n')
    f.write('export const METER_VALUE_IDS: MeterValueID[] = [\n')
    f.write(''.join(enum_names))
    f.write('];\n\n')
    f.write('export const METER_VALUE_INFOS: {[id: number]: {unit: string, digits: 0|1|2|3}} = {\n')
    f.write(''.join(enum_infos))
    f.write('};\n')

for lang in detailed_values:
    util.specialize_template(f'../../../web/src/modules/meters/translation_{lang}.json.template', f'../../../web/src/modules/meters/translation_{lang}.json', {
        '{{{detailed_values}}}': ',\n            '.join(detailed_values[lang]),
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
    f.write('};\n')
