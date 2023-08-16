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

for dir in os.listdir(directory):
    dir = os.path.realpath(os.path.join(directory, dir))
    for file in os.listdir(dir):
        if file.find("cron_trigger") != -1:
            file = os.path.realpath(os.path.join(dir, file))
            f = open(file, "r")
            for line in f:
                if line.find("CronTrigger") != -1:
                    split = line.split(' ')
                    imports += "import {{{0}}} from '{1}'\n".format(split[2], file[:-3])
                    trigger += "             {} |\n".format(split[2])
        elif file.find("cron_action") != -1:
            file = os.path.realpath(os.path.join(dir, file))
            f = open(file, "r")
            for line in f:
                if line.find("CronAction") != -1:
                    split = line.split(' ')
                    imports += "import {{{0}}} from '{1}'\n".format(split[2], file[:-3])
                    action += "            {} |\n".format(split[2])



trigger = trigger[13:-len(' |\n')]
action = action[12:-len(' |\n')]

util.specialize_template("api.ts.template", "api.ts", {
    "{{{imports}}}": imports,
    "{{{trigger}}}": trigger,
    "{{{action}}}": action,
    })
