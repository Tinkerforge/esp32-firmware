/** @jsxImportSource preact */
import { h } from "preact";
import { BatteryMode } from "modules/batteries/generated/battery_mode.enum";
let x = {
    "batteries": {
        "status": {
            "batteries": "Battery Control"
        },
        "navbar": {
            "batteries": "Battery Control"
        },
        "content": {
            "batteries": "Battery Control",

            "experimental": "The battery control is experimental and subject to change.",

            "battery": /*SFN*/(slot: number|"?") => "Battery #" + slot/*NF*/,

            "test_warnings": "Tests",
            "test_warning": /*SFN*/(display_name: string, mode_name: string) => `A test is running for battery storage ${display_name}: ${mode_name}.`/*NF*/,

            "battery_mode_block": "Block",
            "battery_mode_normal": "Normal",
            "battery_mode_block_discharge": "Block discharge",
            "battery_mode_force_charge": "Force charge",
            "battery_mode_block_charge": "Block charge",
            "battery_mode_force_discharge": "Force discharge",

            "battery_mode_long_block": "Block charge, block discharge",
            "battery_mode_long_normal": "Charge normally, discharge normally",
            "battery_mode_long_block_discharge": "Charge normally, block discharge",
            "battery_mode_long_force_charge": "Force charge, block discharge",
            "battery_mode_long_block_charge": "Block charge, discharge normally",
            "battery_mode_long_force_discharge": "Block charge, force discharge",

            "battery_status_blocked": "Blocked",
            "battery_status_normal": "Normal",
            "battery_status_discharge_blocked": "Discharge blocked",
            "battery_status_charge_forced": "Charge forced",
            "battery_status_charge_blocked": "Charge blocked",
            "battery_status_discharge_forced": "Discharge forced",
            "battery_status_unknown": "Unbekannt",
            "battery_status_by_index": /*SFN*/(mode: BatteryMode) => {
                return [
                    __("batteries.content.battery_status_unknown"),
                    __("batteries.content.battery_status_blocked"),
                    __("batteries.content.battery_status_normal"),
                    __("batteries.content.battery_status_discharge_blocked"),
                    __("batteries.content.battery_status_charge_forced"),
                    __("batteries.content.battery_status_charge_blocked"),
                    __("batteries.content.battery_status_discharge_forced"),
                ][mode + 1];
            }/*NF*/,

            "status_charge": "Charging",
            "status_discharge": "Discharging",
            "status_normal": "Normal",
            "status_block": "Blocked",
            "status_force": "Forced",

            "dynamic_tariff_schedule": "Dynamic tariff schedule",
            "schedule_cheap_hours": "Cheap hours",
            "schedule_cheap_hours_help": "Select the cheapest x hours within a 24 hour window between 20:00 and 20:00 of the following day. If the battery storage should be charged from the grid, it makes sense to enter the length of the charge time here. Quarters of an hour can be selected by entering multiples of 0.25. 1.25 h is one hour and 15 minutes.",
            "schedule_expensive_hours": "Expensive hours",
            "schedule_expensive_hours_help": "Select the most expensive x hours within a 24 hour window between 20:00 and 20:00 of the following day. Quarters of an hour can be selected by entering multiples of 0.25. 1.25 h is one hour and 15 minutes.",
            "schedule_hours_muted": "in 0.25 h steps",
            "schedule_graph": "Schedule",
            "schedule_graph_muted": "intervals from 20:00 to 20:00",

            "enable_battery_control": "Battery control enabled",

            "table_battery_display_name": "Display name",
            "table_battery_class": "Class",

            "add_battery_title": "Add battery storage",
            "add_battery_message": /*SFN*/(have: number, max: number) => `${have} of ${max} battery storages configured`/*NF*/,
            "add_battery_slot": "Number",
            "add_battery_class": "Class",

            "edit_battery_title": "Edit battery storage",
            "edit_battery_slot": "Number",
            "edit_battery_class": "Class",

            "battery_import_error_json_malformed": "JSON is malformed.",
            "battery_import_error_class_unknown": "Class is unknown.",
            "battery_import_error_import_unsupported": "Import is not supported by this class.",
            "battery_import_error_config_malformed": "Config is malformed.",
            "battery_export_basename": "battery-storage-config",

            "rules_charge": "Rules for charging",
            "rules_discharge": "Rules for discharging",

            "table_rule_enabled": "Enabled",
            "table_rule_desc": "Description",
            "table_rule_time": "Time",
            "table_rule_soc": "State of charge",
            "table_rule_price": "Energy price",
            "table_rule_forecast": "PV-forecast",
            "table_rule_schedule": "Tariff schedule",
            "table_rule_fast_chg": "Eco/Fast charge",
            "table_rule_action": "Action",

            "table_rule_fast_chg_not_active": "Not active",
            "table_rule_fast_chg_active": "Active",

            "add_rule_title_charge": "Add charge rule",
            "add_rule_title_discharge": "Add discharge rule",
            "add_rule_message": /*SFN*/(have: number, max: number) => `${have} of ${max} rules configured`/*NF*/,
            "add_rule_desc": "Description",
            "add_rule_time": "Time period",
            "add_rule_time_start": "from",
            "add_rule_time_end": "until",
            "add_rule_soc": "State of charge",
            "add_rule_price": "Dynamic energy price",
            "add_rule_price_muted" : "market price",
            "add_rule_forecast": "Solar forecast",
            "add_rule_schedule": "Dynamic tariff schedule",
            "add_rule_fast_chg": "Vehicle eco/fast charge",
            "add_rule_action": "Action",

            "edit_rule_title_charge": "Edit charge rule",
            "edit_rule_title_discharge": "Edit discharge rule",
            "edit_rule_desc": "Description",
            "edit_rule_time": "Time period",
            "edit_rule_time_start": "from",
            "edit_rule_time_end": "until",
            "edit_rule_soc": "State of charge",
            "edit_rule_price": "Dynamic energy price",
            "edit_rule_price_muted" : "market price",
            "edit_rule_forecast": "Solar forecast",
            "edit_rule_schedule": "Dynamic tariff schedule",
            "edit_rule_fast_chg": "Vehicle eco/fast charge",
            "edit_rule_action": "Action",

            "condition_ignore": "Is ignored",
            "condition_below": "Smaller than",
            "condition_above": "Greater than",

            "condition_time_not": "Not",
            "condition_time_only": "Only",

            "condition_schedule_cheap": "Cheap hours",
            "condition_schedule_cheap_compact": "Cheap",
            "condition_schedule_not_cheap": "Not cheap hours",
            "condition_schedule_not_cheap_compact": "Not cheap",
            "condition_schedule_expensive": "Expensive hours",
            "condition_schedule_expensive_compact": "Expensive",
            "condition_schedule_not_expensive": "Not expensive hours",
            "condition_schedule_not_expensive_compact": "Not expensive",
            "condition_schedule_moderate": "Moderately-priced hours (neither cheap nor expensive)",
            "condition_schedule_moderate_compact": "Moderately-priced",

            "condition_fast_chg_inactive": "No eco/fast charge active",
            "condition_fast_chg_active": "Eco/Fast charge active",

            "rule_action_block": "Block",
            "rule_action_force": "Force",

            "rule_action_block_charge": "Block charge",
            "rule_action_force_charge": "Force charge (blocks discharge)",
            "rule_action_block_discharge": "Block discharge",
            "rule_action_force_discharge": "Force discharge (blocks charge)",

            "invalid_feedback_all_ignored": "Cannot ignore all conditions at the same time."
        },
        "script": {
            "save_failed": "Failed to save the battery control settings.",
            "reboot_content_changed": "battery control settings",
            "battery": /*SFN*/(slot: number|"?") => "Battery #" + slot/*NF*/
        }
    }
}
