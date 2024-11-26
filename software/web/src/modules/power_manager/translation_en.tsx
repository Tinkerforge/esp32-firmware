/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "power_manager": {
        "status": {
            "mode": "Charging mode",
            "mode_fast": "Fast",
            "mode_off": "Off",
            "mode_pv": "PV",
            "mode_min_pv": "Min + PV",
            "phase_switching": "Phase switching",
            "single_phase": "Single-phase",
            "three_phase":"Three-phase",
            "config_error_phase_switching": "Phase switching or contactor not configured",
            "config_error_no_max_current": "No maximum current configured for chargers",
            "config_error_no_chargers": "No chargers configured",
            "config_error_excess_no_meter": "Excess charging enabled but no meter configured or meter does not provide total power",
            "config_error_dlm_no_meter": "Dynamic load management enabled but no meter configured or meter does not provide directional phase currents"
        },
        "navbar": {
            "pv_excess_settings": "PV Excess Charging"
        },
        "content": {
            "em_proxy_warning": "When the Energy Manager is controlled by another charge manager, PV excess charging must be enabled on the charge manager, not here.",
            "cm_requirements_warning": null,

            "phase_switching_mode": "Phase switching",
            "automatic": "Automatic",
            "external_control": "External control (EVCC)",
            "external_control_unavailable": "External control (EVCC) – no managed charger for phase switching",
            "external_control_notification": "External control for phase switching is active. This will disable all settings related to PV excess charging.",

            "header_excess_charging": "Photovoltaic Excess Charging",
            "enable_excess_charging": "Excess charging enabled",
            "enable_excess_charging_muted": <><a href="{{{manual_url}}}">see manual for details</a></>,
            "enable_excess_charging_desc": "Will adjust the power consumption of controlled chargers, depending on the power generation of a photovoltaic system and the charge mode.",
            "default_mode": "Default charging mode",
            "default_mode_muted": "will be used after reboot",
            "meter_slot_grid_power": "Power meter",
            "meter_slot_grid_power_muted": "typically at the grid connection",
            "meter_slot_grid_power_select": "Select...",
            "meter_slot_grid_power_none": "No power meter configured",
            "meter_slot_grid_power_missing_value": "sum of directional active power is missing",
            "meter_slot_grid_power_in_use_by_battery": "selected as battery storage power meter",
            "guaranteed_power": "Min + PV: Minimum charging power",
            "guaranteed_power_muted": "Charging power that is allowed to be drawn from the grid to charge vehicles.",
            "target_power_from_grid": "Target grid power draw",
            "target_power_from_grid_muted": "target grid draw for excess charging",
            "control_behavior": "Control behavior",
            "control_behavior_muted": <><a href="{{{manual_url}}}">see manual for details</a></>,
            "target_power_n200": "Very conservative (− 200 W)",
            "target_power_n100": "Conservative (− 100 W)",
            "target_power_n50_with_battery_meter": "Slightly conservative (− 50 W)",
            "target_power_n50_without_battery_meter": "Slightly conservative (− 50 W) – charges battery storage first",
            "target_power_0_with_battery_meter": "Balanced (± 0 W)",
            "target_power_0_without_battery_meter": "Balanced (± 0 W) – recommended with no battery storage",
            "target_power_p50_with_battery_meter": "Slightly aggressive (+ 50 W)",
            "target_power_p50_without_battery_meter": "Slightly aggressive (+ 50 W) – uses battery storage to charge vehicle",
            "target_power_p100": "Aggressive (+ 100 W)",
            "target_power_p200": "Very aggressive (+ 200 W)",
            "cloud_filter": "Cloud filter",
            "cloud_filter_muted": "reduces influence of partly cloudy weather",
            "cloud_filter_off": "Off",
            "cloud_filter_weak": "Weak",
            "cloud_filter_medium": "Medium",
            "cloud_filter_strong": "Strong",

            "header_battery_storage": "Battery storage",
            "meter_slot_battery_power": "Power meter",
            "meter_slot_battery_power_muted": "of the battery",
            "meter_slot_battery_power_none": "No battery",
            "meter_slot_battery_power_missing_value": "sum of directional active power or directional DC power are missing",
            "meter_slot_battery_power_in_use_by_grid": "selected as grid power meter",
            "battery_mode": "Battery priority",
            "battery_mode_prefer_chargers": "Prefer chargers, charge battery storage with any excess power",
            "battery_mode_prefer_battery": "Prefer battery, charge vehicles with any excess power",
            "battery_inverted": "Battery power direction",
            "battery_inverted_n": "Normal: positive power while charging, negative power while discharging",
            "battery_inverted_i": "Inverted: negative power while charging, positive power while discharging",
            "battery_deadzone": "Import and export tolerance",
            "battery_deadzone_muted": "at the grid connection, while the battery is active",
            "battery_deadzone_help": "Most battery storages will allow some export and import while charging or discharging. The tolerance should be set to 1.5 times the expected export and import. If the battery permits an export of -50 W and an import of 50 W, the tolerance should be set to 75 W."
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
                    return <>When <b>power is fed</b> to the grid, </>;
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
            "single_phase": "to single phase",
            "three_phase": "to three phases",
            "automation_action_text": /*FFN*/(phases: number) => <>switch to <b>{phases == 1 ? "single phase" : "three phases"}</b></>/*NF*/
        },
        "script": {
            "save_failed": "Failed to save power manager settings.",
            "reboot_content_changed": "power manager settings",
            "mode_change_failed": "Failed to change charging mode.",

            "meter_slots_internal": "charger only",
            "meter_slots_no_values": "no values yet"
        }
    }
}
