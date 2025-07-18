/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": "Charge Manager"
        },
        "script": {
            "mode_explainer_1": <>{__("This_device")} is part of a charge management group with other ELTAKO Wallboxes. Another device controls this group to make sure the configured current is never exceeded.</>,
            "mode_explainer_2": <>{__("This_device")} either uses the PV excess charging or controls a charge management group with other ELTAKO Wallboxes to make sure the configured current is never exceeded.</>
        }
    },
    "system": {
        "content": {
            "factory_reset_modal_text": <>A factory reset will reset all configuration to their default values. All tracked charges are lost. <b>This action cannot be undone!</b> The ELTAKO Wallbox will re-open the WiFi access point with the default SSID and passphrase from the manual. Continue?</>,
            "config_reset_modal_text": <>Resets only the configuration to default values. Tracked charges will be preserved. <b>This action cannot be undone!</b> The ELTAKO Wallbox will re-open the WiFi access point with the default SSID and passphrase from the manual. Continue?</>,
            "reboot_desc": "charging will not be interrupted",
            "factory_reset_desc": "removes the configuration and all tracked charges",
            "config_reset_desc": "tracked charges will be preserved"
        }
    },
    "firmware_update": {
        "script": {
            "install_state_19": "Firmware file corrupted (info page missing)"
        }
    },
    "meters": {
        "status": {
            "power_history": "Charge history"
        }
    },
    "meters_modbus_tcp": {
        "content": {
            "table_tinkerforge_warp_charger": "ELTAKO Wallbox"
        }
    },
    "modbus_tcp": {
        "content": {
            "warp": "ELTAKO Wallbox"
        }
    },
    "modbus_tcp_debug": {
        "content": {
            "server_missing_github": <></>
        }
    },
    "branding": {
        "device": "ELTAKO Wallbox"
    }
}
