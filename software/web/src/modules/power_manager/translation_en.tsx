/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
import { toLocaleFixed } from "../../ts/util";
import { __, removeUnicodeHacks } from "../../ts/translation";
let x = {
    "power_manager": {
        "status": {
            "phase_switching": "Phase switching",
            "single_phase": "Single-phase",
            "three_phase":"Three-phase",
            "config_error_phase_switching": "Phase switching or contactor not configured",
            "config_error_no_max_current": "No maximum current configured for chargers",
            "config_error_no_chargers": "No chargers configured",
            "config_error_excess_no_meter": "Excess charging is enabled, but no meter is configured, or the meter does not provide total power",
            "config_error_dlm_no_meter": "Dynamic load management is enabled, but no meter is configured, or the meter does not provide directional phase currents"
        },
        "navbar": {
            "pv_excess_settings": "PV Excess Charging"
        },
        "content": {
            "em_proxy_warning": "When the Energy Manager is controlled by another charge manager, PV excess charging must be enabled on the charge manager, not here.",
            "cm_requirements_warning": <>To use the PV excess charging, the <a href="#charge_manager_chargers">{__("charge_manager.content.enable_charge_manager")}</a> must be set to “{__("charge_manager.content.mode_manager")}”.</>,

            "phase_switching_mode": "Phase switching",
            "automatic": "Automatic",
            "external_control": "External control (EVCC)",
            "external_control_unavailable": "External control (EVCC) – no managed charger for phase switching",
            "external_control_notification": "External control for phase switching is active. This will disable all settings related to PV excess charging.",

            "header_excess_charging": "Photovoltaic Excess Charging",
            "enable_excess_charging": "Excess charging enabled",
            "enable_excess_charging_help": /*FFN*/() => options.WARP_DOC_BASE_URL.length > 0 ? <p>The <a href={removeUnicodeHacks(`${options.WARP_DOC_BASE_URL}/docs/tutorials/pv_excess_charging`)}>tutorial PV-excess-charging</a> helps configuring this.</p> : undefined/*NF*/,
            "enable_excess_charging_desc": "Will adjust the power consumption of controlled chargers, depending on the power generation of a photovoltaic system and the charge mode.",
            "default_mode": "Default charging mode",
            "default_mode_muted": "will be used after reboot",
            "meter_slot_grid_power": "Power meter",
            "meter_slot_grid_power_muted": "typically at the grid connection",
            "meter_slot_grid_power_none": "No power meter configured",
            "meter_slot_grid_power_missing_value": "sum of directional active power is missing",
            "meter_slot_grid_power_in_use_by_battery": "selected as battery storage power meter",
            "guaranteed_power": "Min + PV: Minimum charging power",
            "guaranteed_power_muted": "Charging power that is allowed to be drawn from the grid to charge vehicles.",
            "control_behavior": "Control behavior",
            "control_behavior_help": <><p>Defines the target grid draw. This allows to prioritizing a battery storage over charging an electric vehicle.</p></>,
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
            "battery_mode_target_soc": "Prefer battery while it is below the target SoC",
            "battery_target_soc": "Storage target SoC",
            "battery_target_soc_muted": "desired minimum state of charge of the battery storage",
            "battery_target_soc_help": "Prioritize charging the battery while its state of charge is below this value. Once the desired state of charge is reached, chargig vehicles is prioritized over charging the battery more. 80 % is a common value.",
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
                    return <><b>block</b> charging with slot <b>{toLocaleFixed(slot)}</b>.</>
                }
                return <><b>unblock</b> charging with slot <b>{toLocaleFixed(slot)}</b>.</>
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

            "set_phases": "Switch phases",
            "phases_wanted": "Switch to",
            "single_phase": "to single phase",
            "three_phase": "to three phases",
            "automation_action_text": /*FFN*/(phases: number) => <>switch to <b>{phases == 1 ? "single phase" : "three phases"}</b></>/*NF*/
        },
        "script": {
            "save_failed": "Failed to save power manager settings.",
            "reboot_content_changed": "power manager settings",
            "phase_change_failed": "Failed to switch phases.",

            "meter_slots_internal": "charger only",
            "meter_slots_no_values": "no values yet"
        }
    }
}
