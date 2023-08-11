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

with open('meter_value_id.csv', newline='') as f:
    for row in csv.reader(f):
        if row[0] == 'id':
            # skip header
            continue

        if len(row[0]) == 0:
            # skip empty
            continue

        id_ = row[0]
        name = ''.join(row[1:6]).replace(' ', '')
        unit = row[6]

        enum_values.append(f'    {name} = {id_}, // {unit}\n')

with open('meter_value_id.h', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class MeterValueID {\n')
    f.write(''.join(enum_values))
    f.write('};\n')

with open('../../../web/src/modules/meters/meter_value_id.ts', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export enum MeterValueID {\n')
    f.write(''.join(enum_values))
    f.write('}\n')
