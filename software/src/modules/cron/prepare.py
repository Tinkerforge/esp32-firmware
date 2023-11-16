import os
import sys
import importlib.util
import importlib.machinery
import csv

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util

# NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
triggers = [
    ('None', 0),
    ('Cron', 1),
    ('IEC Change', 2),
    ('MQTT', 3),
    ('EVSE Button', 4),
    ('NFC', 5),
    ('Charge Limits', 6),
    ('EVSE Shutdown Input', 7),
    ('EVSE GP Input', 8),
    ('EVSE External Current Wd', 9),
    ('Require Meter', 10),
    ('Charge Manager Wd', 11),
    ('EM Input Three', 12),
    ('EM Input Four', 13),
    ('EM Phase Switch', 14),
    ('EM Contactor Monitoring', 15),
    ('EM Power Available', 16),
    ('EM Grid Power Draw', 17),
]

# NEVER EVER EDIT OR REMOVE IDS. Only append new ones. Changing or removing IDs is a breaking API and config change!
actions = [
    ('None', 0),
    ('Print', 1),
    ('MQTT', 2),
    ('Set Current', 3),
    ('LED', 4),
    ('Meter Reset', 5),
    ('Set Manager Current', 6),
    ('NFC Inject Tag', 7),
    ('Charge Limits', 8),
    ('EVSE GP Output', 9),
    ('Charge Tracker Reset', 10),
    ('EM Phase Switch', 11),
    ('EM Charge Mode Switch', 12),
    ('EM Relay Switch', 13),
    ('EM Limit Max Current', 14),
    ('EM Block Charge', 15),
]

trigger_values = []
action_values = []

for trigger in triggers:
    trigger_values.append('    {0} = {1},\n'.format(util.FlavoredName(trigger[0]).get().camel, trigger[1]))

for action in actions:
    action_values.append('    {0} = {1},\n'.format(util.FlavoredName(action[0]).get().camel, action[1]))

with open('cron_defs.h', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include <stdint.h>\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class CronTriggerID : uint8_t {\n')
    f.write(''.join(trigger_values))
    f.write('};\n\n')
    f.write('enum class CronActionID : uint8_t {\n')
    f.write(''.join(action_values))
    f.write('};\n')

with open('../../../web/src/modules/cron/cron_defs.ts', 'w') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export const enum CronTriggerID {\n')
    f.write(''.join(trigger_values))
    f.write('}\n\n')
    f.write('export const enum CronActionID {\n')
    f.write(''.join(action_values))
    f.write('}\n')
