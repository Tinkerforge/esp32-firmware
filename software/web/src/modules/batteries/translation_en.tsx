/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "batteries": {
        "status": {
        },
        "navbar": {
            "batteries": "Battery Control"
        },
        "content": {
            "batteries": "Battery Control",

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

            "managed_batteries": "Managed battery storages",

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
            "table_rule_soc": "Battery SoC",
            "table_rule_price": "Energy price",
            "table_rule_forecast": "PV-forecast",
            "table_rule_schedule": "Tariff schedule",
            "table_rule_fast_chg": "Fast charge",
            "table_rule_action": "Action",

            "table_rule_fast_chg_not_active": "Not active",
            "table_rule_fast_chg_active": "Active",

            "add_rule_title": "Add rule",
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
            "add_rule_fast_chg": "Vehicle fast charge",
            "add_rule_action": "Action",

            "edit_rule_title": "Edit rule",
            "edit_rule_desc": "Description",
            "edit_rule_time": "Time period",
            "edit_rule_time_start": "from",
            "edit_rule_time_end": "until",
            "edit_rule_soc": "State of charge",
            "edit_rule_price": "Dynamic energy price",
            "edit_rule_price_muted" : "market price",
            "edit_rule_forecast": "Solar forecast",
            "edit_rule_schedule": "Dynamic tariff schedule",
            "edit_rule_fast_chg": "Vehicle fast charge",
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

            "condition_fast_chg_inactive": "No fast charge active",
            "condition_fast_chg_active": "Fast charge active",

            "rule_action_block": "Block",
            "rule_action_force": "Force",

            "invalid_feedback_all_ignored": "Cannot ignore all conditions at the same time."
        },
        "script": {
            "save_failed": "Failed to save the battery control settings.",
            "reboot_content_changed": "battery control settings",
            "battery": /*SFN*/(slot: number|"?") => "Battery #" + slot/*NF*/
        }
    }
}
