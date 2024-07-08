from pathlib import PurePath
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

imports = []
already_imported = set({})
triggers = []
actions = []
inits = []

for plugin in util.find_frontend_plugins('Automation', 'Trigger'):
    import_path = PurePath('..', plugin.module_name, plugin.import_name).as_posix()

    if import_path not in already_imported:
        imports.append("import * as {0}_trigger from '{1}'".format(plugin.module_name, import_path))
        already_imported.add(plugin.module_name)
        inits.append("result.push({0}_trigger.init());".format(plugin.module_name))

    for interface_name in plugin.interface_names:
        triggers.append('{0}_trigger.{1}'.format(plugin.module_name, interface_name))

for plugin in util.find_frontend_plugins('Automation', 'Action'):
    import_path = PurePath('..', plugin.module_name, plugin.import_name).as_posix()

    if import_path not in already_imported:
        imports.append("import * as {0}_action from '{1}'".format(plugin.module_name, import_path))
        already_imported.add(plugin.module_name)
        inits.append("result.push({0}_action.init());".format(plugin.module_name))

    for interface_name in plugin.interface_names:
        actions.append('{0}_action.{1}'.format(plugin.module_name, interface_name))

tfutil.specialize_template("api.ts.template", "api.ts", {
    "{{{imports}}}": '\n'.join(imports),
    "{{{triggers}}}": ('\n    | ' if len(triggers) > 0 else '') + '\n    | '.join(triggers),
    "{{{actions}}}": ('\n    | ' if len(actions) > 0 else '') + '\n    | '.join(actions),
})

tfutil.specialize_template("plugins.tsx.template", "plugins.tsx", {
    "{{{imports}}}": '\n'.join(imports),
    "{{{inits}}}": '\n    '.join(inits),
})
