import sys
import re
from pathlib import PurePath
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

with open('../../build.ts', 'r', encoding='utf-8') as f:
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
    import_path = PurePath('..', plugin.module_name, plugin.import_name).as_posix()
    imports.append("import * as {0}_config from '{1}'".format(plugin.module_name, import_path))
    inits.append("result.push({0}_config.init());".format(plugin.module_name))

    for interface_name in plugin.interface_names:
        configs.append('{0}_config.{1}'.format(plugin.module_name, interface_name))

with open('api.ts', 'w', encoding='utf-8') as f:
    f.write(tfutil.specialize_template('api.ts.template_header', None, {
        "{{{imports}}}": '\n'.join(imports),
        "{{{configs}}}": ('\n    | ' if len(configs) > 0 else '') + '\n    | '.join(configs),
    }) + '\n')

    for i in range(meter_count):
        f.write(tfutil.specialize_template('api.ts.template_fragment', None, {
            "{{{meter_id}}}": str(i),
        }) + '\n')

tfutil.specialize_template("plugins.tsx.template", "plugins.tsx", {
    "{{{imports}}}": '\n'.join(imports),
    "{{{inits}}}": '\n    '.join(inits),
})
