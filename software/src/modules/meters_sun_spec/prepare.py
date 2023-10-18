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

model_ids = [
    ('Inverter Single Phase (Integer)', 101, 50),
    ('Inverter Split Phase (Integer)', 102, 50),
    ('Inverter Three Phase (Integer)', 103, 50),
    ('Inverter Single Phase (Float)', 111, 60),
    ('Inverter Split Phase (Float)', 112, 60),
    ('Inverter Three Phase (Float)', 113, 60),
    ('AC Meter Single Phase (Integer)', 201, 105),
    ('AC Meter Split Phase (Integer)', 202, 105),
    ('AC Meter Wye Three Phase (Integer)', 203, 105),
    ('AC Meter Delta Three Phase (Integer)', 204, 105),
    ('AC Meter Single Phase (Float)', 211, 124),
    ('AC Meter Split Phase (Float)', 212, 124),
    ('AC Meter Wye Three Phase (Float)', 213, 124),
    ('AC Meter Delta Three Phase (Float)', 214, 124),
]

enum_values = []
spec_values = []

for model_id in model_ids:
    enum_key = model_id[0].replace(" ", "").replace("(", "").replace(")", "")

    enum_values.append(f'    {enum_key} = {model_id[1]},\n')
    spec_values.append(f'    {{\n        SunSpecModelID::{enum_key},\n        {model_id[2]},\n        "{model_id[0]}",\n    }},\n')

with open('sun_spec_model_id.h', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include <stdint.h>\n')
    f.write('#include <stdlib.h>\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class SunSpecModelID : uint16_t {\n')
    f.write(''.join(enum_values))
    f.write('};\n\n')
    f.write('struct SunSpecModelSpec {\n')
    f.write('    SunSpecModelID model_id;\n')
    f.write('    size_t block_length;\n')
    f.write('    const char *model_name;\n')
    f.write('};\n\n')
    f.write('extern SunSpecModelSpec sun_spec_model_specs[];\n\n')
    f.write(f'extern size_t sun_spec_model_specs_length;\n')

with open('sun_spec_model_id.cpp', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include "sun_spec_model_id.h"\n\n')
    f.write('SunSpecModelSpec sun_spec_model_specs[] = {\n')
    f.write(''.join(spec_values))
    f.write('};\n\n')
    f.write(f'size_t sun_spec_model_specs_length = {len(model_ids)};\n')
