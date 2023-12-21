import os
import sys
import importlib.util
import importlib.machinery
from pathlib import PurePath

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

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

util.specialize_template("api.ts.template", "api.ts", {
    "{{{imports}}}": '\n'.join(imports),
    "{{{triggers}}}": ('\n    | ' if len(triggers) > 0 else '') + '\n    | '.join(triggers),
    "{{{actions}}}": ('\n    | ' if len(actions) > 0 else '') + '\n    | '.join(actions),
})

util.specialize_template("plugins.tsx.template", "plugins.tsx", {
    "{{{imports}}}": '\n'.join(imports),
    "{{{inits}}}": '\n    '.join(inits),
})
