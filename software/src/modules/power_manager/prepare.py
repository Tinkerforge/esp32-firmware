import os
import sys
import importlib.util
import importlib.machinery

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util

import re

debug_log_variables = [
    "power_at_meter_smooth_w",
    "power_at_meter_filtered_w",
    "power_available_w",
    "power_available_filtered_w",
    "charge_manager_available_current_ma",
    "charge_manager_allocated_current_ma",
    "max_current_limited_ma",
    "",
    "mode",
    "is_3phase",
    "wants_3phase",
    "wants_3phase_last",
    "is_on_last",
    "wants_on_last",
    "",
    "charging_blocked.combined",
    "excess_charging_enable",
    "just_switched_phases",
    "uptime_past_hysteresis",
    "switching_state",
    "",
    "phase_state_change_blocked_until",
    "on_state_change_blocked_until",
]

formats = 'fmt(' + '),\n        fmt('.join(debug_log_variables) + '),'
header  = '    "' + ',"\n    "'.join([re.sub(r'[^.]+\.', '', v).replace('[', '_').replace(']', '') for v in debug_log_variables]) + '"'
data    = ',\n               '.join(filter(None, debug_log_variables))

util.specialize_template("power_manager_debug.cpp.template", "power_manager_debug.cpp", {
    "{{{varcount}}}": str(len(debug_log_variables)),
    "{{{formats}}}": formats,
    "{{{header}}}": header,
    "{{{data}}}": data,
})
