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

            "discharge_blocked": "Battery discharge",
            "discharge_blocked_no": "Not forbidden",
            "discharge_blocked_yes": "Forbidden",

            "block_discharge_during_fast_charge": "Forbid discharge during fast charge",
            "block_discharge_during_fast_charge_desc": "Will forbid the battery to discharge while a vehicle is fast charging or is allowed to do so by its Eco charge plan.",

            "managed_batteries": "Managed battery storages",

            "table_display_name": "Display name",
            "table_class": "Class",

            "add_battery_title": "Add battery storage",
            "add_battery_prefix": "",
            "add_battery_infix": " of ",
            "add_battery_suffix": " battery storages configured",
            "add_battery_slot": "Number",
            "add_battery_class": "Class",

            "edit_battery_title": "Edit battery storage",
            "edit_battery_slot": "Number",
            "edit_battery_class": "Class"
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
