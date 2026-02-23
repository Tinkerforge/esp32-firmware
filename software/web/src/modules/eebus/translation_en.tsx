/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eebus": {
        "content": {
            "enable_eebus": "Enabled EEBUS",
            "enable_eebus_desc": "Enables communication with EEBUS devices",
            "enable_eebus_help": <>
                <p>EEBUS is a communication standard for smart energy management. It enables communication
                between energy devices such as wallboxes, heat pumps, PV systems and energy management systems (HEMS).</p>
                <p>The following use cases are supported:</p>
                <ul>
                    <li><b>LPC (Limitation of Power Consumption)</b>: Power consumption limitation. Used by control boxes
                    (Steuerbox) for grid control according to §14a EnWG in Germany. Allows the grid operator to limit charging power.</li>
                    <li><b>MPC (Monitoring of Power Consumption)</b>: Power consumption monitoring. Provides power and
                    energy measurements to external systems.</li>
                    <li><b>CEVC (Coordinated EV Charging)</b>: Coordinated charging. Allows an energy manager to
                    control charging based on charging plans and incentives.</li>
                    <li><b>EVCC (EV Commissioning and Configuration)</b>: Vehicle configuration. Reports information about
                    the connected electric vehicle such as communication standard and power limits.</li>
                    <li><b>EVCEM (EV Charging Electricity Measurement)</b>: Charging measurement. Provides measurements
                    such as current, power and charged energy per phase.</li>
                    <li><b>EVSECC (EVSE Commissioning and Configuration)</b>: Wallbox status. Reports the operating state
                    of the charging station and any error conditions.</li>
                    <li><b>EVCS (EV Charging Summary)</b>: Charging summary. Provides summaries of completed
                    charging sessions with energy amount and costs.</li>
                </ul>
            </>,
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
