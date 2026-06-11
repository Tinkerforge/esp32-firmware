/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "ev": {
        "navbar": {
            "ev": "Vehicles"
        },
        "status": {
            "ev": "Vehicle",
            "no_ev": "No vehicle connected",
            "soc": "State of charge",
            "capacity": "Battery capacity",
            "efficiency": "Charging efficiency"
        },
        "content": {
            "ev": "Vehicles",
            "ev_profiles": "Vehicle profiles",
            "ev_profiles_desc": "Configure known vehicles by their MAC address for Autocharge. Their battery capacity and charging efficiency are used to estimate the state of charge.",
            "table_name": "Name",
            "table_mac": "MAC address",
            "table_capacity": "Capacity",
            "table_efficiency": "Efficiency",
            "add_ev_title": "Add vehicle profile",
            "add_ev_message": /*SFN*/(current: number, max: number) => `${current} of ${max} vehicle profiles configured.`/*NF*/,
            "add_ev_name": "Name",
            "add_ev_name_desc": "A name to identify this vehicle.",
            "add_ev_mac": "MAC address",
            "add_ev_mac_desc": "MAC address of the vehicle.",
            "add_ev_mac_invalid": "Invalid MAC address format.",
            "add_ev_rotating_mac": "Changing address",
            "add_ev_rotating_mac_desc": "Enable this if the vehicle changes part of its address between charging sessions. This is common for some vehicles (e.g. VW ID models). Only the fixed part of the address will be used for identification.",
            "add_ev_capacity": "Battery capacity",
            "add_ev_capacity_desc": "Total battery capacity in kWh.",
            "add_ev_efficiency": "Charging efficiency",
            "add_ev_efficiency_desc": "Ratio of energy stored vs. energy consumed.",
            "edit_ev_title": "Edit vehicle profile",
            "active_ev": "Active vehicle",
            "currently_connected": "Currently connected",
            "active_ev_name": "Name",
            "active_ev_mac": "MAC address",
            "active_ev_soc": "State of charge",
            "active_ev_capacity": "Battery capacity",
            "active_ev_efficiency": "Charging efficiency",
            "no_active_ev": "No vehicle currently connected.",
            "seen_macs": "Recently seen vehicle MACs",
            "seen_macs_desc": "MAC addresses of recently connected EVs.",
            "no_seen_macs": "No MAC addresses seen yet.",
            "seen_mac_last_seen": /*SFN*/(time: string) => `Last seen ${time} ago`/*NF*/,
            "seen_mac_charger": /*SFN*/(charger: string) => `Seen at charger: ${charger}`/*NF*/,
            "mac_duplicate": "This MAC address is already used by another profile.",
            "mac_already_configured": /*SFN*/(name: string) => `Already configured: ${name}`/*NF*/,
            "add_ev_seen_macs": "Recently seen MACs"
        },
        "script": {
            "save_failed": "Failed to save the vehicle settings.",
            "reboot_content_changed": "vehicle settings"
        }
    }
}
