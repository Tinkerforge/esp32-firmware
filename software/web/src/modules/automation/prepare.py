from pathlib import PurePath
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

imports = []
already_imported = set({})
trigger_modules = []
trigger_types = []
action_modules = []
action_types = []

for plugin in util.find_frontend_plugins('Automation', 'Trigger'):
    import_path = PurePath('..', plugin.module_name, plugin.import_name).as_posix()

    if import_path not in already_imported:
        imports.append("import * as {0}_trigger from '{1}'".format(plugin.module_name, import_path))
        already_imported.add(plugin.module_name)
        trigger_modules.append(f'{plugin.module_name}_trigger')

    for interface_name in plugin.interface_names:
        trigger_types.append('{0}_trigger.{1}'.format(plugin.module_name, interface_name))

for plugin in util.find_frontend_plugins('Automation', 'Action'):
    import_path = PurePath('..', plugin.module_name, plugin.import_name).as_posix()

    if import_path not in already_imported:
        imports.append("import * as {0}_action from '{1}'".format(plugin.module_name, import_path))
        already_imported.add(plugin.module_name)
        action_modules.append(f'{plugin.module_name}_action')

    for interface_name in plugin.interface_names:
        action_types.append('{0}_action.{1}'.format(plugin.module_name, interface_name))

tfutil.specialize_template("api.ts.template", "api.ts", {
    "{{{imports}}}": '\n'.join(imports),
    "{{{trigger_types}}}": ('\n    | ' if len(trigger_types) > 0 else '') + '\n    | '.join(trigger_types),
    "{{{action_types}}}": ('\n    | ' if len(action_types) > 0 else '') + '\n    | '.join(action_types),
})

tfutil.specialize_template("plugins.tsx.template", "plugins.tsx", {
    "{{{imports}}}": '\n'.join(imports),
    '{{{trigger_modules}}}': f'[{', '.join(trigger_modules)}]',
    '{{{action_modules}}}': f'[{', '.join(action_modules)}]',
})
