/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": "Charge management"
        }
    },
    "system": {
        "content": {
            "factory_reset_modal_text": <>A factory reset will reset all configuration to their default values. <b>This action cannot be undone!</b> The SEB will re-open the WiFi access point with the default SSID and passphrase from the manual. Are you sure?</>,
            "reboot_desc": "",
            "config_reset_modal_text": "",
            "factory_reset_desc": "removes all configuration",
            "config_reset_desc": ""
        }
    },
    "firmware_update": {
        "script": {
            "install_state_19": "Firmware file corrupted or for a different product (info page missing)",
            "install_state_21": "Firmware file does not match the type of this SEB"
        }
    },
    "mqtt": {
        "content": {
            "enable_mqtt_desc": <>This allows controlling the SEB over the configured MQTT broker. <a href="{{{apidoc_url}}}">MQTT API documentation</a></>
        }
    },
    "network": {
        "content": {
            "enable_mdns_desc": "Allows other devices in this network to find the SEB."
        }
    },
    "util": {
        "event_connection_lost_title": "SEB connection lost!"
    },
    "wifi": {
        "content": {
            "ap_hide_ssid_desc_pre": "The SEB is reachable under the BSSID ",
            "confirm_content": <>Are you sure you want to disable the WiFi Access Point? If no connection to a configured network can be established in the future, <b>it is not possible to access the SEB anymore</b>.The SEB then has to be reset to factory defaults. <a href="{{{manual_url}}}">See manual for details.</a></>
        }
    },
    "meters": {
        "status": {
            "power_history": "Power history"
        }
    },
    "power_manager": {
        "content": {
            "cm_requirements_warning": "No chargers configured. To use the PV excess charging, all connected chargers must be configured as managed chargers in the charge manager."
        }
    },
    "remote_access": {
        "content": {
            "registration": "Registrating SEB"
        }
    }
}
