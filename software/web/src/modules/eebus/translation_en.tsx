/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eebus": {
        "content": {
            "enable_eebus": "Enabled EEBUS",
            "enable_eebus_desc": "Enables communication with EEBUS devices",
            "enable_eebus_help": <>
                <p>EEBUS is a communication standard that will be used in the future by energy network operators in Germany to reduce the
                load on the power grid. EEBUS will be used to control consumption devices (e.g., wall boxes) that
                fall under §14a EnWG. EEBUS is standardized, but many details regarding the exact implementation are still open.
                We are currently waiting for more detailed information and testing opportunities from the network operators.</p>
                <p>Have you already gotten a "Steuerbox" with EEBUS support installed? If so,
                please contact us at <a href="mailto:info@tinkerforge.com">info@tinkerforge.com</a>.</p>
                <p>EEBUS defines various use cases. For control by the network operator, the LPC (Limitation of Power Consumption)
                use case is required. We will publish this use case as soon as the first control boxes with EEBUS support are
                rolled out.</p>
                <p>For control via a HEMS, other use cases are defined by EEBUS.
                For the area of E-Mobility EEBUS defines the following use cases:
                <ul>
                    <li>Coordinated EV Charging</li>
                    <li>Overload Protection by EV Charging Current Curtailment</li>
                    <li>Optimization of Self-Consumption during EV Charging</li>
                    <li>EV Charging Electricity Measurement</li>
                    <li>EV Charging Summary</li>
                    <li>EV Commissioning and Configuration</li>
                    <li>EVSE Commissioning and Configuration</li>
                </ul>
                In a second step, we will implement relevant use cases from the E-Mobility sector.</p>
                <p>Currently, it is already possible to search for other EEBUS-enabled devices and establish a connection
                between this device and other EEBUS-enabled devices. In addition, we log the communication in the debug log,
                such that the communication can be evaluated by us.</p>
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
                "state_connected": "Connected",
                "state_eebus_connected": "EEBUS Active",
                "state_disconnected": "Disconnected",
                "state_discovered": "Discovered",
                "state_loaded_from_config": "Loaded from Config",
                "autoregister": "Autoregistration enabled",
                "overwrite_notice": "These settings will be overwritten when a EEBUS device with that SKI is detected.",
                "persistent": "Stored in config",
                "persistent_remove_error": "Can only remove peers stored in config. Discovered peers cannot be removed. They will be automatically discarded on reboot."
            },
            "show_usecase_details": "Show EEBUS Details",
            "yes": "Yes",
            "no": "No"
        },
        "script": {
            "save_failed": "Failed to save the EEBUS settings.",
            "reboot_content_changed": "EEBUS-Settings"
        }
    }
}
