import os
import sys
import re
import importlib.util
import importlib.machinery

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util

with open(os.path.join(software_dir, "web", "src", "build.ts")) as f:
    content = f.read()
    match = re.search(r"export const METERS_SLOTS = (\d+);", content)
    if match is None:
        print("Failed to find METERS_SLOTS in build.ts!")
        sys.exit(1)

    meter_count = int(match.group(1))

with open('api.ts', 'w') as f:
    for i in range(meter_count):
        f.write(util.specialize_template('api.ts.template_fragment', None, {"{{{meter_id}}}": str(i)}))
