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

            "permitted_no": "Not permitted",
            "permitted_yes": "Permitted",
            "forbidden_no": "Not forbidden",
            "forbidden_yes": "Forbidden",

            "grid_charge_permitted": "Charge from grid",
            "discharge_forbidden": "Battery discharge",
            "charge_forbidden": "Battery charge",

            "dynamic_tariff_schedule": "Dynamic tariff schedule",
            "schedule_cheap_hours": "Cheap hours",
            "schedule_cheap_hours_help": "Select the cheapest x hours within a 24 hour window between 20:00 and 20:00 of the following day. If the battery storage should be charged from the grid, it makes sense to enter the length of the charge time here. Quarters of an hour can be selected by entering multiples of 0.25. 1.25 h is one hour and 15 minutes.",
            "schedule_expensive_hours": "Expensive hours",
            "schedule_expensive_hours_help": "Select the most expensive x hours within a 24 hour window between 20:00 and 20:00 of the following day. Quarters of an hour can be selected by entering multiples of 0.25. 1.25 h is one hour and 15 minutes.",
            "schedule_hours_muted": "in 0.25 h steps",

            "forbid_discharge_during_fast_charge": "Forbid discharge during fast charge",
            "forbid_discharge_during_fast_charge_desc": "Will forbid the battery to discharge while a vehicle is fast charging or is allowed to do so by its Eco charge plan.",

            "managed_batteries": "Managed battery storages",

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

            "rules_permit_grid_charge": "Rules for charge from grid permission",
            "rules_forbid_discharge": "Rules for battery discharge forbiddance",
            "rules_forbid_charge": "Rules for battery charge forbiddance",

            "table_rule_desc": "Description",
            "table_rule_soc": "State of charge",
            "table_rule_price": "Dynamic energy price",
            "table_rule_forecast": "Solar forecast",
            "table_rule_schedule": "Dynamic tariff schedule",

            "add_rule_title": "Add rule",
            "add_rule_message": /*SFN*/(have: number, max: number) => `${have} of ${max} rules configured`/*NF*/,
            "add_rule_desc": "Description",
            "add_rule_soc": "State of charge",
            "add_rule_price": "Dynamic energy price",
            "add_rule_forecast": "Solar forecast",
            "add_rule_schedule": "Dynamic tariff schedule",

            "edit_rule_title": "Edit rule",
            "edit_rule_desc": "Description",
            "edit_rule_soc": "State of charge",
            "edit_rule_price": "Dynamic energy price",
            "edit_rule_forecast": "Solar forecast",
            "edit_rule_schedule": "Dynamic tariff schedule",

            "condition_ignore": "Is ignored",
            "condition_below": "Smaller than",
            "condition_above": "Greater than",

            "condition_schedule_cheap": "Cheap hours",
            "condition_schedule_not_cheap": "Not cheap hours",
            "condition_schedule_expensive": "Expensive hours",
            "condition_schedule_not_expensive": "Not expensive hours",
            "condition_schedule_moderate": "Moderately-priced hours (neither cheap nor expensive)",

            "invalid_feedback_all_ignored": "Cannot ignore all conditions at the same time."
        },
        "script": {
            "save_failed": "Failed to save the battery control settings.",
            "reboot_content_changed": "battery control settings",
            "battery": /*SFN*/(slot: number|"?") => "Battery #" + slot/*NF*/
        },
        "automation": {
            "permit_grid_charge": "Permit battery grid charge",
            "revoke_grid_charge_override": "Revoke battery grid charge permission",
            "forbid_discharge": "Forbid battery discharge",
            "revoke_discharge_override": "Revoke battery discharge forbiddance"
        }
    }
}
