import sys
import re
from pathlib import PurePath
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

with open('../../options.ts', 'r', encoding='utf-8') as f:
    content = f.read()
    match = re.search(r"export const METERS_MAX_SLOTS = (\d+);", content)
    if match is None:
        print("Failed to find METERS_MAX_SLOTS in options.ts!")
        sys.exit(1)

    meters_max_slots = int(match.group(1))

imports = []
config_modules = []
config_types = []

for plugin in util.find_frontend_plugins('Meters', 'Config'):
    import_path = PurePath('..', plugin.module_name, plugin.import_name).as_posix()
    imports.append(f"import * as {plugin.module_name}_config from '{import_path}'")
    config_modules.append(f'{plugin.module_name}_config')

    for interface_name in plugin.interface_names:
        config_types.append('{0}_config.{1}'.format(plugin.module_name, interface_name))

with open('api.ts', 'w', encoding='utf-8') as f:
    f.write(tfutil.specialize_template('api.ts.template_header', None, {
        "{{{imports}}}": '\n'.join(imports),
        "{{{config_types}}}": ('\n    | ' if len(config_types) > 0 else '') + '\n    | '.join(config_types),
    }) + '\n')

    for i in range(meters_max_slots):
        f.write(tfutil.specialize_template('api.ts.template_fragment', None, {
            "{{{meter_id}}}": str(i),
        }) + '\n')

tfutil.specialize_template('plugins.tsx.template', 'plugins.tsx', {
    '{{{imports}}}': '\n'.join(imports),
    '{{{config_modules}}}': f'[{', '.join(config_modules)}]',
})
