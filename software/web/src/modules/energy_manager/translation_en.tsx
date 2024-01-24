/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "energy_manager": {
        "status": {
            "mode": "Charging mode",
            "mode_fast": "Fast",
            "mode_off": "Off",
            "mode_pv": "PV",
            "mode_min_pv": "Min + PV",
            "phase_switching": "Phase switching",
            "single_phase": "Single-phase",
            "three_phase":"Three-phase",
            "external_control_state": "External control",
            "external_control_state_available": "Available",
            "external_control_state_disabled": "Disabled",
            "external_control_state_unavailable": "Unavailable",
            "external_control_state_switching": "Switching now",
            "external_control_request": "External phase request",
            "external_control_request_none": "None",
            "status": "Energy manager",
            "error_ok": "OK",
            "error_network": "Network error",
            "error_contactor": "Contactor error",
            "error_config": "Incomplete configuration",
            "error_internal": "Internal error",
            "config_error_phase_switching": "Phase switching or contactor not configured",
            "config_error_no_max_current": "No maximum current configured for chargers",
            "config_error_no_chargers": "No chargers configured",
            "config_error_excess_no_meter": "Excess charging enabled but no meter configured",

            "no_bricklet": "Internal error, bricklet not found"
        },
        "navbar": {
            "energy_manager": "Settings"
        },
        "content": {
            "page_header": "Energy Manager",

            "header_phase_switching": "Phase switching",
            "contactor_installed": "Contactor installed",
            "contactor_installed_desc": "The energy manager is connected to a contactor that it can use to switch its managed chargers between single-phase and three-phase connection.",
            "phase_switching_mode": "Switching mode",
            "fixed_single_phase": "Fixed single-phase connection",
            "fixed_three_phases": "Fixed three-phase connection",
            "automatic": "Automatic",
            "always_single_phase": "Always single-phase",
            "always_three_phases": "Always three-phase",
            "pv1p_fast3p": "Single-phase PV mode, three-phase Fast mode",
            "external_control": "External control (EVCC)",
            "external_control_notification": "External control for phase switching is active. This will disable all settings related to PV excess charging.",

            "header_load_management": "Dynamic load management",
            "load_management_explainer": "The energy manager adjusts the power consumption of controlled chargers to avoid exceeding the maximum allowed current on any phase of the building's mains connection. (Coming soon: This feature is currently in development.)",

            "header_excess_charging": "Photovoltaic excess charging",
            "enable_excess_charging": "Excess charging enabled",
            "enable_excess_charging_muted": <><a href="{{{manual_url}}}">see manual for details</a></>,
            "enable_excess_charging_desc": "The energy manager will adjust the power consumption of controlled chargers, depending on the power generation of a photovoltaic system and the charge mode.",
            "default_mode": "Default charging mode",
            "default_mode_muted": "will be used after energy manager reboot",
            "auto_reset_charging_mode": "Daily reset",
            "auto_reset_charging_mode_disabled": "Found different Automation rule for setting the charging mode, please check the Automation settings.",
            "auto_reset_charging_mode_desc": "Automatically sets the charging mode back to the default.",
            "meter_slot_grid_power": "Power meter",
            "meter_slot_grid_power_muted": "for excess charging",
            "meter_slot_grid_power_select": "Select...",
            "meter_slot_grid_power_none": "No power meter configured",
            "guaranteed_power": "Min + PV: Minimum charging power",
            "guaranteed_power_muted": "Charging power that is allowed to be drawn from the grid to charge vehicles.",
            "target_power_from_grid": "Target grid power draw",
            "target_power_from_grid_muted": "target grid draw for excess charging",
            "control_behavior": "Control behavior",
            "control_behavior_muted": <><a href="{{{manual_url}}}">see manual for details</a></>,
            "target_power_n200": "Very conservative (− 200 W)",
            "target_power_n100": "Conservative (− 100 W)",
            "target_power_n50": "Slightly conservative (− 50 W) – charges battery bank first",
            "target_power_0": "Balanced (± 0 W) – recommended with no battery bank",
            "target_power_p50": "Slightly aggressive (+ 50 W) – uses battery bank to charge vehicle",
            "target_power_p100": "Aggressive (+ 100 W)",
            "target_power_p200": "Very aggressive (+ 200 W)",
            "cloud_filter": "Cloud filter",
            "cloud_filter_muted": "reduces influence of partly cloudy weather",
            "cloud_filter_off": "None",
            "cloud_filter_weak": "Weak",
            "cloud_filter_medium": "Medium",
            "cloud_filter_strong": "Strong",

            "header_expert_settings": "Expert settings",
            "hysteresis_time": "Hysteresis time",
            "hysteresis_time_muted": "Minimum delay before phase switches or starting or stopping a charge, to avoid excessive wear on the vehicle's charge electronics by switching too often."
        },
        "automation": {
            "slot": "Blocking slot",
            "block_charge": "Block charging",
            "unblock_charge": "Unblock charging",
            "block_mode": "Mode",
            "automation_block_charge_action_text": /*FFN*/(slot: number, block: boolean) => {
                if (block) {
                    return <><b>block</b> charging with slot <b>{slot}</b>.</>
                }
                return <><b>unblock</b> charging with slot <b>{slot}</b>.</>
            }/*NF*/,
            "limit_max_current": "Limit maximum total current",
            "limit_mode": "Mode",
            "reset_limit_max_current": "Reset maximum total current limit",
            "max_current": "Maximum total current",
            "automation_limit_max_current_action_text": /*FFN*/(current: number, default_current: number) => {
                if (current === -1) {
                    return <>reset maximum total current limit to the configured default current (<b>{default_current / 1000} A</b>).</>
                }
                return <>limit maximum total current to <b>{current / 1000} A</b>.</>
            }/*NF*/,
            "grid_power_draw": "Grid power draw",
            "drawing": "Drawing power from the grid",
            "feeding": "Feeding power to the grid",
            "automation_grid_power_draw_text": /*FFN*/(drawing_power: boolean) => {
                if (drawing_power) {
                    return <>When <b>power is drawn</b> from the grid, </>;
                } else {
                    return <>When <b>power is fed</b> the grid, </>;
                }
            }/*NF*/,
            "power_available": "Power available",
            "not_available": "not available",
            "available": "available",
            "power": "Power",
            "automation_power_available_text": /*FFN*/(power: boolean) => {
                let not = <></>
                if (!power) {
                    not = <><b>not </b></>
                }
                return <>When {not}<b>enough</b> power for charging is available, </>
            }/*NF*/,
            "contactor_monitoring": "Contactor monitoring triggered",
            "automation_contactor_monitoring_text": /*FFN*/(contactor: boolean) => {
                if (contactor) {
                    return <>When <b>no contactor error</b> is detected on boot, </>
                } else {
                    return <>When <b>a contactor error</b> is detected, </>
                }
            }/*NF*/,
            "contactor_monitoring_state": "State",
            "contactor_error": "Contactor error",
            "contactor_okay": "No contactor error on boot",
            "phase_switch": "Phases switched",
            "phase": "Phase",
            "automation_phase_switch_text": /*FFN*/(phase: number) => {
                let ret = <></>;
                switch (phase) {
                    case 1:
                        ret = <><b>single-phase</b></>
                        break;

                    case 3:
                        ret = <><b>three-phase</b></>
                        break;
                }
                return <>When the contactor switched to {ret}, </>
            }/*NF*/,
            "input": /*SFN*/(input: number) => "Input " + input + " switched"/*NF*/,
            "state": "State",
            "closed": "Closed",
            "open": "Open",
            "automation_input_text": /*FFN*/(input: number, state: boolean) => {
                let ret = state ? <><b>closed</b></> : <><b>open</b></>
                return <>If <b>Input {input}</b> switches to state {ret}, </>
            }/*NF*/,
            "switch_relay": "Switch relay",
            "relay_state": "Swtich to",
            "relay_state_open": "Open",
            "relay_state_closed": "Closed",
            "relay_action_text": /*FFN*/(state: boolean) => {
                let ret = state ? <><b>close</b></> : <><b>open</b></>
                return <>{ret} relay.</>
            }/*NF*/,
            "charge_mode_default": "Default mode",
            "charge_mode_switch": "Switch charge mode",
            "charge_mode": "Charge mode",
            "fast": "Fast",
            "disabled": "Disabled",
            "pv_excess": "PV-Excess",
            "guaranteed_power": "Min + PV: Minimum charging power",
            "charge_mode_switch_action_text": /*FFN*/(mode: number, default_mode: number) => {
                const modes = ["Fast", "Disabled", "PV-Excess", "Min + PV"];

                let ret = <></>;
                switch (mode) {
                    case 0:
                        ret = <><b>Fast</b></>
                        break;

                    case 1:
                        ret = <><b>Disabled</b></>
                        break;

                    case 2:
                        ret = <><b>PV-Excess</b></>
                        break;

                    case 3:
                        ret = <><b>Min + PV</b></>
                        break;

                    default:
                        ret = <><b>Default mode ({modes[default_mode]})</b></>
                        break;
                }
                return <>switch charge mode to {ret}.</>
            }/*NF*/,
            "set_phases": "Switch phases",
            "phases_wanted": "Switch to",
            "single_phase": "Single-phase",
            "three_phase": "Three-phase",
            "automation_action_text": /*FFN*/(phases: number) => <>switch to <b>{phases == 1 ? "Single-phase" : "Three-phase"}</b></>/*NF*/
        },
        "script": {
            "save_failed": "Failed to save energy manager settings.",
            "reboot_content_changed": "energy manager settings",
            "mode_change_failed": "Failed to change charging mode.",
            "meter": /*SFN*/(slot: number|"?") => "Meter #" + slot/*NF*/
        }
    }
}
