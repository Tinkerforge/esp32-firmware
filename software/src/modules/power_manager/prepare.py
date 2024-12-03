import re
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

debug_log_variables = [
    "power_at_meter_raw_w",
    "power_available_w",
    #"charge_manager_available_current_ma",
    #"charge_manager_allocated_current_ma",
    "cm_allocated_currents->l1",
    "cm_allocated_currents->l2",
    "cm_allocated_currents->l3",
    "max_current_limited_ma",
    "",
    "mode",
    "current_phases",
    ##"wants_3phase",
    ##"wants_3phase_last",
    ##"is_on_last",
    ##"wants_on_last",
    "",
    "charging_blocked.combined",
    "excess_charging_enabled",
    ##"just_switched_phases",
    ##"uptime_past_hysteresis",
    ##"switching_state",
    "",
    ##"phase_state_change_blocked_until",
    ##"on_state_change_blocked_until",
]

formats = 'fmt(' + '),\n        fmt('.join(debug_log_variables) + '),'
header  = '    "' + ',"\n    "'.join([re.sub(r'[^.]+\.', '', v).replace('[', '_').replace(']', '') for v in debug_log_variables]) + '"'
data    = ',\n               '.join(filter(None, debug_log_variables))

tfutil.specialize_template("power_manager_debug.cpp.template", "power_manager_debug.cpp", {
    "{{{varcount}}}": str(len(debug_log_variables)),
    "{{{formats}}}": formats,
    "{{{header}}}": header,
    "{{{data}}}": data,
})
