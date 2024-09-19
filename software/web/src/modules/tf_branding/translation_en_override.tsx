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
            "factory_reset_modal_text": <>A factory reset will reset all configuration to their default values. <b>This action cannot be undone!</b> The Brick will re-open the WiFi access point with the default SSID and passphrase from the label on the backside of the Brick. Are you sure?</>,
            "reboot_desc": "",
            "config_reset_modal_text": "",
            "factory_reset_desc": "removes all configuration",
            "config_reset_desc": ""
        }
    },
    "firmware_update": {
        "script": {
            "install_state_19": "Firmware file corrupted (info page missing)",
            "install_state_21": "Firmware file does not match the type of this Brick"
        }
    },
    "network": {
        "content": {
            "enable_mdns_desc": "Allows other devices in this network to find the Brick."
        }
    },
    "mqtt": {
        "content": {
            "enable_mqtt_desc": <>This allows controlling the Brick over the configured MQTT broker. <a href="{{{apidoc_url}}}">MQTT API documentation</a></>
        }
    },
    "util": {
        "event_connection_lost_title": "Brick connection lost!"
    },
    "wifi": {
        "content": {
            "ap_hide_ssid_desc_pre": "The Brick is reachable under the BSSID ",
            "confirm_content": <>Are you sure you want to disable the WiFi Access Point? If no connection to a configured network can be established in the future, <b>it is not possible to access the brick anymore</b>.The brick then has to be reset to factory defaults. <a href="{{{manual_url}}}">See manual for details.</a></>
        }
    },
    "remote_access": {
        "content": {
            "registration": "Registrating Brick"
        }
    }
}
