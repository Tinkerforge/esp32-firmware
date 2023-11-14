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

imports = []
inits = []
configs = []

for plugin in util.find_frontend_plugins('Meters', 'Config'):
    import_path = '../' + plugin.module_name + '/' + plugin.import_name
    imports.append("import * as {0}_config from '{1}'".format(plugin.module_name, import_path))
    inits.append("result.push({0}_config.init());".format(plugin.module_name))

    for interface_name in plugin.interface_names:
        configs.append('{0}_config.{1}'.format(plugin.module_name, interface_name))

with open('api.ts', 'w', encoding='utf-8') as f:
    f.write(util.specialize_template('api.ts.template_header', None, {
        "{{{imports}}}": '\n'.join(imports),
        "{{{configs}}}": ('\n    | ' if len(configs) > 0 else '') + '\n    | '.join(configs),
    }) + '\n')

    for i in range(meter_count):
        f.write(util.specialize_template('api.ts.template_fragment', None, {
            "{{{meter_id}}}": str(i),
        }) + '\n')

util.specialize_template("plugins.tsx.template", "plugins.tsx", {
    "{{{imports}}}": '\n'.join(imports),
    "{{{inits}}}": '\n    '.join(inits),
})
