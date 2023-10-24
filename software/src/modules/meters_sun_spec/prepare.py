import os
import sys
import importlib.util
import importlib.machinery

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util
from collections import namedtuple

ModelSpec = namedtuple('ModelSpec', 'model_name display_name_en display_name_de model_id block_length')

model_specs = [
    ModelSpec('Inverter Single Phase Integer',   'Inverter Single Phase',   'Inverter Einphasig',            101, 50),
    ModelSpec('Inverter Split Phase Integer',    'Inverter Split Phase',    'Inverter Einphasig-Dreileiter', 102, 50),
    ModelSpec('Inverter Three Phase Integer',    'Inverter Three Phase',    'Inverter Dreiphasig',           103, 50),
    ModelSpec('Inverter Single Phase Float',     'Inverter Single Phase',   'Inverter Einphasig',            111, 60),
    ModelSpec('Inverter Split Phase Float',      'Inverter Split Phase',    'Inverter Einphasig-Dreileiter', 112, 60),
    ModelSpec('Inverter Three Phase Float',      'Inverter Three Phase',    'Inverter Dreiphasig',           113, 60),
    ModelSpec('Meter Single Phase Integer',      'Meter Single Phase ',     'Zähler Einphasig',              201, 105),
    ModelSpec('Meter Split Phase Integer',       'Meter Split Phase',       'Zähler Einphasig-Dreileiter',   202, 105),
    ModelSpec('Meter Wye Three Phase Integer',   'Meter Wye Three Phase',   'Zähler Stern-Dreiphasig',       203, 105),
    ModelSpec('Meter Delta Three Phase Integer', 'Meter Delta Three Phase', 'Zähler Delta-Dreiphasig',       204, 105),
    ModelSpec('Meter Single Phase Float',        'Meter Single Phase',      'Zähler Einphasig',              211, 124),
    ModelSpec('Meter Split Phase Float',         'Meter Split Phase',       'Zähler Einphasig-Dreileiter',   212, 124),
    ModelSpec('Meter Wye Three Phase Float',     'Meter Wye Three Phase',   'Zähler Stern-Dreiphasig',       213, 124),
    ModelSpec('Meter Delta Three Phase Float',   'Meter Delta Three Phase', 'Zähler Delta-Dreiphasig',       214, 124),
]

enum_values = []
spec_values = []
translation_values = {'en': [], 'de': []}
model_ids = []

for model_spec in model_specs:
    enum_key = model_spec.model_name.replace(' ', '')

    enum_values.append(f'    {enum_key} = {model_spec.model_id},\n')
    spec_values.append(f'    {{\n        SunSpecModelID::{enum_key},\n        {model_spec.block_length},\n        "{model_spec.model_name}",\n    }},\n')
    translation_values['en'].append(f'"model_{model_spec.model_id}": "{model_spec.display_name_en} [{model_spec.model_id}]"')
    translation_values['de'].append(f'"model_{model_spec.model_id}": "{model_spec.display_name_de} [{model_spec.model_id}]"')
    model_ids.append(f'    {model_spec.model_id},\n')

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
    f.write('extern size_t sun_spec_model_specs_length;\n')

with open('sun_spec_model_id.cpp', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include "sun_spec_model_id.h"\n\n')
    f.write('SunSpecModelSpec sun_spec_model_specs[] = {\n')
    f.write(''.join(spec_values))
    f.write('};\n\n')
    f.write(f'size_t sun_spec_model_specs_length = {len(model_specs)};\n')

for lang in translation_values:
    util.specialize_template(f'../../../web/src/modules/meters_sun_spec/translation_{lang}.tsx.template', f'../../../web/src/modules/meters_sun_spec/translation_{lang}.tsx', {
        '{{{models}}}': ',\n            '.join(translation_values[lang]),
    })

with open('../../../web/src/modules/meters_sun_spec/sun_spec_model_id.ts', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export const SUN_SPEC_MODEL_IDS: number[] = [\n')
    f.write(''.join(model_ids))
    f.write('];\n')
