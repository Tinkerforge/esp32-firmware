/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eebus": {
        "content": {
            "enable_eebus": "EEBUS enabled",
            "enable_eebus_desc": "Enables communication with EEBUS devices",
            "enable_eebus_help_intro": <>
                <p>EEBUS is a communication standard for smart energy management. It enables communication
                    between energy devices such as EV chargers, heat pumps, PV systems and energy management systems
                    (HEMS).</p>
            </>,
            "enable_eebus_help_usecases_intro": <>
                <p>The standard defines use cases which can be used to represent various functions of devices and
                    systems. </p>
                <p>The following use cases are supported:</p>
            </>,
            "usecase_lpc_desc": <>Power consumption limitation. Used by control boxes for <strong>grid control according to §14a EnWG</strong>. Allows the grid operator to limit power consumption of major power-consumers (e.g. EV chargers, HVAC systems).</>,
            "usecase_lpp_desc": "Power production limitation. Allows limiting the power feed-in to the grid by the grid operator.",
            "usecase_mpc_desc": "Power consumption monitoring. Monitors the power consumption or output of a device.",
            "usecase_mgcp_desc": "Grid connection point monitoring. Monitors the power flow at the grid connection point including feed-in and consumption.",
            "usecase_cevc_desc": "Coordinated charging. Allows an energy manager to control charging based on charging plans and incentives.",
            "usecase_evcc_desc": "EV Vehicle configuration. Reports information about the connected vehicle such as communication standard and power limits.",
            "usecase_evcem_desc": "EV Charging measurement. Provides measurements such as current, power and charged energy.",
            "usecase_evsecc_desc": "Charger status. Reports the operating state of the charging station and any error conditions.",
            "usecase_evcs_desc": "Charging summary. Provides summaries of completed charging sessions with energy amount and costs.",
            "usecase_opev_desc": "EV Overload protection. Allows limiting the charging current of an EV. If the Limit conflicts with a limit given by the LPC use case, the LPC limit takes precedence.",
            "usecase_value_writable": "writable",
            "ski": "SKI",
            "ski_muted": "Subject Key Identifier of this device",
            "ski_help": <>
                <p>The Subject Key Identifier (SKI) is a unique cryptographic identifier for this device.</p>
                <p>If this device needs to communicate with, for example, a "Steuerbox", the SKI may be
                    required during the setup of the Steuerbox.</p>
            </>,
            "unknown": "Unknown",
            "searching_peers": "Searching....",
            "add_peer_title": "Add EEBUS device",
            "add_peer_message": /*SFN*/(have: number, max: number) => `${have} of ${max} EEBUS devices configured`/*NF*/,
            "discovered_peers": "Discovered devices",
            "no_peers_found": "No devices found. Ensure devices are on the same network.",
            "add_peer_manual_desc": "Or enter device details manually:",
            "is_optional": "optional",
            "peer_info": {
                "peers_desc": "In order to establish an EEBUS connection, an EEBUS device must first be added. Connections from other devices will be rejected.",
                "peers": "EEBUS devices",
                "dns_name": "DNS address",
                "device_ip": "Addresses",
                "device_port": "Port",
                "device_trusted": "Allow connection",
                "model_brand": "Manufacturer",
                "model_model": "Device name",
                "model_type": "Device Type",
                "trusted_no": "No",
                "trusted_yes": "Yes",
                "edit_peer_title": "Edit",
                "wss_path": "WSS path",
                "state": "Status",
                "state_connected": "SHIP Active",
                "state_eebus_connected": "EEBUS active",
                "state_disconnected": "Disconnected",
                "state_discovered": "Discovered",
                "state_loaded_from_config": "Loaded from config",
                "state_awaiting_approval": "Awaiting approval",
                "state_connecting": "Connecting",
                "state_degraded": "Degraded",
                "unknown_device": "Unknown device",
                "autoregister": "Autoregistration enabled"
            },
            "usecase_details": "Details",
            "yes": "Yes",
            "no": "No",
            "mdns_disabled_warning": "mDNS is disabled in the network settings. EEBUS requires mDNS for device discovery. Please enable mDNS under Network settings.",
            "usecase_inactive": "This use case is inactive because not all required meter data has been received yet.",
            "usecase_inactive_no_vehicle": "This use case is inactive because no vehicle is connected."
        },
        "navbar": {
            "eebus": "EEBUS"
        },
        "status": {
            "peer_degraded": "Peer Degraded",
            "lpc_failsafe": "LPC Failsafe",
            "evse_failure": "EVSE Failure",
            "heartbeat_timeout": "Heartbeat Timeout",
            "n_devices_connected": "connected"
        },
        "script": {
            "save_failed": "Failed to save the EEBUS settings.",
            "ski_required": "SKI is required",
            "ip_invalid": "IP address is invalid",
            "add_peer_failed": "Failed to add peer"
        }
    }
}
