/** @jsxImportSource preact */
import {h} from "preact";

let x = {
    "eebus": {
        "content": {
            "enable_eebus": "Enabled EEBUS",
            "enable_eebus_desc": "Enables communication with EEBUS devices",
            "eebus_detail_info": "The values that are provided or which can be written by the use cases via EEBUS are displayed here. The values displayed reflect the latest EEBUS communication data. Values that can be written by EEBUS control devices are marked as such. All other values are read-only for EEBUS devices.",
            "enable_eebus_help_intro": <>
                <p>EEBUS is a communication standard for smart energy management. It enables communication
                    between energy devices such as wallboxes, heat pumps, PV systems and energy management systems
                    (HEMS).</p>
            </>,
            "enable_eebus_help_usecases_intro": <>
                <p>The standard defines use cases which can be used to represent various functions of devices and
                    systems. </p>
                <p>The following use cases are supported:</p>
            </>,
            "usecase_lpc_desc": "Power consumption limitation. Used by control boxes for grid control according to §14a EnWG. Allows the grid operator to limit power consumption of major power-consumers (e.g. Wallboxes, HVAC systems).",
            "usecase_lpp_desc": "Power production limitation. Allows limiting the power feed-in to the grid by the grid operator.",
            "usecase_mpc_desc": "Power consumption monitoring. Monitors the power consumption or output of a device.",
            "usecase_mgcp_desc": "Grid connection point monitoring. Monitors the power flow at the grid connection point including feed-in and consumption.",
            "usecase_cevc_desc": "Coordinated charging. Allows an energy manager to control charging based on charging plans and incentives.",
            "usecase_evcc_desc": "EV Vehicle configuration. Reports information about the connected vehicle such as communication standard and power limits.",
            "usecase_evcem_desc": "EV Charging measurement. Provides measurements such as current, power and charged energy.",
            "usecase_evsecc_desc": "Wallbox status. Reports the operating state of the charging station and any error conditions.",
            "usecase_evcs_desc": "Charging summary. Provides summaries of completed charging sessions with energy amount and costs.",
            "usecase_opev_desc": "EV Overload protection. Allows limiting the charging current of an EV. If the Limit conflicts with a limit given by the LPC usecase, the LPC limit takes precedence.",
            "usecase_value_writable": "writable",
            "cert": "certificate",
            "key": "key",
            "ski": "SKI",
            "ski_muted": "Subject Key Identifier of this device",
            "ski_help": <>
                <p>The Subject Key Identifier (SKI) is a unique cryptographic identifier for this device.</p>
                <p>If this device needs to communicate with, for example, a "Steuerbox", the SKI may be
                    required during the setup of the Steuerbox.</p>
            </>,
            "no_cert": "Not used",
            "unknown": "Unknown",
            "search_peers": "Search",
            "searching_peers": "Searching....",
            "search_completed": "New Search",
            "search_failed": "Search failed",
            "add_peers": "Add Device",
            "add_peer_title": "Add EEBUS Device",
            "add_peer_message": "Add a new EEBUS peer device to connect to.",
            "discovered_peers": "Discovered Devices",
            "no_peers_found": "No devices found. Ensure devices are on the same network.",
            "add_peer_manual_desc": "Or enter device details manually:",
            "already_added": "Added",
            "ski_required": "SKI is required",
            "ip_required": "IP address is required",
            "ip_too_short": "IP address must be at least 7 characters (e.g. 1.1.1.1)",
            "add_peer_failed": "Failed to add peer",
            "peer_info": {
                "peers": "EEBUS-Peer",
                "dns_name": "DNS Address",
                "device_ip": "Addresses",
                "device_port": "Port",
                "device_trusted": "Allow Connection",
                "model_brand": "Manufacturer",
                "model_model": "Device Name",
                "model_type": "Device Type",
                "trusted_no": "No",
                "trusted_yes": "Yes",
                "edit_peer_title": "Edit",
                "wss_path": "WSS Path",
                "state": "Status",
                "state_connected": "SHIP Active",
                "state_eebus_connected": "EEBUS Active",
                "state_disconnected": "Disconnected",
                "state_discovered": "Discovered",
                "state_loaded_from_config": "Loaded from Config",
                "autoregister": "Autoregistration enabled",
                "overwrite_notice": "These settings will be overwritten when a EEBUS device with that SKI is detected.",
                "persistent": "Stored in config",
                "persistent_remove_error": "Can only remove peers stored in config. Discovered peers cannot be removed. They will be automatically discarded on reboot."
            },
            "usecase_details": "Details",
            "yes": "Yes",
            "no": "No",
            "devices": "Peers",
            "connected": "Connected",
            "discovered": "Discovered",
            "commands": "Commands",
            "received": "Received",
            "sent": "Sent",
            "usecases": "Use Cases"
        },
        "navbar": {
            "eebus": "EEBUS"
        },
        "status": {
            "peer_degraded": "Peer Degraded",
            "lpc_failsafe": "LPC Failsafe",
            "evse_failure": "EVSE Failure",
            "heartbeat_timeout": "Heartbeat Timeout",
            "discovery_failed": "Discovery Failed"
        },
        "script": {
            "save_failed": "Failed to save the EEBUS settings.",
            "reboot_content_changed": "EEBUS-Settings"
        }
    }
}
