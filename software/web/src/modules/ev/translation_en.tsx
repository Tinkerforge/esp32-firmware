/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "ev": {
        "navbar": {
            "ev": "Electric Vehicles"
        },
        "status": {
            "ev": "Electric Vehicle",
            "no_ev": "No EV connected",
            "soc": "State of Charge",
            "capacity": "Battery Capacity",
            "efficiency": "Charging Efficiency"
        },
        "content": {
            "ev": "Electric Vehicles",
            "ev_profiles": "EV Profiles",
            "ev_profiles_desc": "Configure known electric vehicles by their MAC address for Autocharge. Their battery capacity and charging efficiency are used to estimate the state of charge.",
            "table_name": "Name",
            "table_mac": "MAC Address",
            "table_capacity": "Capacity",
            "table_efficiency": "Efficiency",
            "add_ev_title": "Add EV Profile",
            "add_ev_message": /*SFN*/(current: number, max: number) => `${current} of ${max} EV profiles configured.`/*NF*/,
            "add_ev_name": "Name",
            "add_ev_name_desc": "A name to identify this EV.",
            "add_ev_mac": "MAC Address",
            "add_ev_mac_desc": "MAC address of the EV.",
            "add_ev_mac_invalid": "Invalid MAC address format.",
            "add_ev_rotating_mac": "Changing address",
            "add_ev_rotating_mac_desc": "Enable this if the EV changes part of its address between charging sessions. This is common for some vehicles (e.g. VW ID models). Only the fixed part of the address will be used for identification.",
            "add_ev_capacity": "Battery Capacity",
            "add_ev_capacity_desc": "Total battery capacity in kWh.",
            "add_ev_efficiency": "Charging Efficiency",
            "add_ev_efficiency_desc": "Ratio of energy stored vs. energy consumed.",
            "edit_ev_title": "Edit EV Profile",
            "active_ev": "Active EV",
            "currently_connected": "Currently Connected",
            "active_ev_name": "Name",
            "active_ev_mac": "MAC Address",
            "active_ev_soc": "State of Charge",
            "active_ev_capacity": "Battery Capacity",
            "active_ev_efficiency": "Charging Efficiency",
            "no_active_ev": "No EV currently connected.",
            "seen_macs": "Recently Seen EV MACs",
            "seen_macs_desc": "MAC addresses of recently connected EVs.",
            "no_seen_macs": "No MAC addresses seen yet.",
            "seen_mac_last_seen": /*SFN*/(time: string) => `Last seen ${time} ago`/*NF*/,
            "seen_mac_charger": /*SFN*/(charger: string) => `Seen at charger "${charger}"`/*NF*/,
            "mac_duplicate": "This MAC address is already used by another profile.",
            "mac_already_configured": /*SFN*/(name: string) => `Already configured: ${name}`/*NF*/,
            "add_ev_seen_macs": "Recently Seen MACs"
        },
        "script": {
            "save_failed": "Failed to save the EV settings.",
            "reboot_content_changed": "EV settings"
        }
    }
}
