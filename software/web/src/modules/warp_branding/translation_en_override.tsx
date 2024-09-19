/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": "Charge Manager"
        }
    },
    "system": {
        "content": {
            "factory_reset_modal_text": <>A factory reset will reset all configuration to their default values. All tracked charges are lost. <b>This action cannot be undone!</b> The charger will re-open the WiFi access point with the default SSID and passphrase from the manual. Continue?</>,
            "config_reset_modal_text": <>Resets only the configuration to default values. Tracked charges will be preserved. <b>This action cannot be undone!</b> The charger will re-open the WiFi access point with the default SSID and passphrase from the manual. Continue?</>,
            "reboot_desc": "charging will not be interrupted",
            "factory_reset_desc": "removes the configuration and all tracked charges",
            "config_reset_desc": "tracked charges will be preserved"
        }
    },
    "firmware_update": {
        "script": {
            "install_state_19": "Firmware file corrupted or for WARP1 Charger (info page missing)",
            "install_state_21": "Firmware file does not match the type of this charger"
        }
    },
    "mqtt": {
        "content": {
            "enable_mqtt_desc": <>This allows controlling the charger over the configured MQTT broker. <a href="{{{apidoc_url}}}">MQTT API documentation</a></>
        }
    },
    "network": {
        "content": {
            "enable_mdns_desc": "Allows other devices in this network to find the charger."
        }
    },
    "util": {
        "event_connection_lost_title": "Charger connection lost!"
    },
    "wifi": {
        "content": {
            "ap_hide_ssid_desc_pre": "The charger is reachable under the BSSID ",
            "confirm_content": <>Are you sure you want to disable the WiFi Access Point? If no connection to a configured network can be established in the future, <b>it is not possible to access the charger anymore</b>.The charger then has to be reset to factory defaults. <a href="{{{manual_url}}}">See manual for details.</a></>
        }
    },
    "charge_tracker": {
        "content": {
            "charge_tracker_remove_modal_text": <>All tracked charges and the username history will be removed. <b>This action cannot be undone!</b> The charger then will restart.</>
        }
    },
    "meters": {
        "status": {
            "power_history": "Charge history"
        }
    },
    "power_manager": {
        "content": {
            "cm_requirements_warning": "To use the PV excess charging, charge management must be enabled and set to “Charge manager” and the charger should only manage itself."
        }
    },
    "remote_access": {
        "content": {
            "registration": "Registrating charger"
        }
    }
}
