import os
import sys
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

import re

directory = os.path.realpath(os.path.join(os.path.dirname(__file__), '..'))


imports = ""
trigger = ""
action = ""
sideeffects_imports = ""

already_imported = set({})

for dir in os.listdir(directory):
    dir = os.path.realpath(os.path.join(directory, dir))
    for file in os.listdir(dir):
        if file.find("cron_trigger") != -1:
            file = os.path.realpath(os.path.join(dir, file))
            f = open(file, "r")
            dot_index = file.find('.')
            path_split = file.split('/')
            module_name = path_split[path_split.__len__() - 2] + "_trigger"
            if module_name not in already_imported:
                imports += "import * as {0} from '{1}'\n".format(module_name, file[:dot_index])
                sideeffects_imports += "import '{}'\n".format(file[:dot_index])
                already_imported.add(module_name)
            for line in f:
                if line.find("CronTrigger {") != -1:
                    split = line.split(' ')
                    trigger += "             {0}.{1} |\n".format(module_name,split[2])
        elif file.find("cron_action") != -1:
            file = os.path.realpath(os.path.join(dir, file))
            f = open(file, "r")
            dot_index = file.find('.')
            path_split = file.split('/')
            module_name = path_split[path_split.__len__() - 2] + "_actions"
            if module_name not in already_imported:
                imports += "import * as {0} from '{1}'\n".format(module_name, file[:dot_index])
                sideeffects_imports += "import '{}'\n".format(file[:dot_index])
                already_imported.add(module_name)
            for line in f:
                if line.find("CronAction {") != -1:
                    split = line.split(' ')
                    action += "             {0}.{1} |\n".format(module_name,split[2])



trigger = trigger[13:-len(' |\n')]
action = action[12:-len(' |\n')]

util.specialize_template("api.ts.template", "api.ts", {
    "{{{imports}}}": imports,
    "{{{trigger}}}": trigger,
    "{{{action}}}": action,
    })

util.specialize_template("sideeffects.tsx.template", "sideeffects.tsx", {
    "{{{imports}}}": sideeffects_imports
})
