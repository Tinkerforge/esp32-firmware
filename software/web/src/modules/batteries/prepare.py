import sys
import re
from pathlib import PurePath
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

with open('../../options.ts', 'r', encoding='utf-8') as f:
    content = f.read()
    match = re.search(r"export const BATTERIES_MAX_SLOTS = (\d+);", content)
    if match is None:
        print("Failed to find BATTERIES_MAX_SLOTS in options.ts!")
        sys.exit(1)

    batteries_max_slots = int(match.group(1))

imports = []
inits = []
configs = []

for plugin in util.find_frontend_plugins('Batteries', 'Config'):
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

    for i in range(batteries_max_slots):
        f.write(tfutil.specialize_template('api.ts.template_fragment', None, {
            "{{{battery_id}}}": str(i),
        }) + '\n')

tfutil.specialize_template("plugins.tsx.template", "plugins.tsx", {
    "{{{imports}}}": '\n'.join(imports),
    "{{{inits}}}": '\n    '.join(inits),
})
