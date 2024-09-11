import re
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

util.embed_bricklet_firmware_bin()

debug_log_variables = [
    "contactor_check_tripped",
    "em_common.consecutive_bricklet_errors",
    "",
    "all_data.common.power",
    "all_data.common.current[0]",
    "all_data.common.current[1]",
    "all_data.common.current[2]",
    "",
    "all_data.common.energy_meter_type",
    "all_data.common.error_count[0]",
    "all_data.common.error_count[1]",
    "all_data.common.error_count[2]",
    "all_data.common.error_count[3]",
    "all_data.common.error_count[4]",
    "all_data.common.error_count[5]",
    "",
    "all_data.contactor_value",
    "",
    "all_data.rgb_value_r",
    "all_data.rgb_value_g",
    "all_data.rgb_value_b",
    "",
    "all_data.input[0]",
    "all_data.input[1]",
    "",
    "all_data.relay",
    "",
    "all_data.common.voltage",
    "",
    "all_data.contactor_check_state",
]

formats = 'fmt(' + '),\n        fmt('.join(debug_log_variables) + '),'
header  = '    "' + ',"\n    "'.join([re.sub(r'[^.]+\.', '', v).replace('[', '_').replace(']', '') for v in debug_log_variables]) + '"'
data    = ',\n               '.join(filter(None, debug_log_variables))

tfutil.specialize_template("energy_manager_debug.cpp.template", "energy_manager_debug.cpp", {
    "{{{varcount}}}": str(len(debug_log_variables)),
    "{{{formats}}}": formats,
    "{{{header}}}": header,
    "{{{data}}}": data,
})
