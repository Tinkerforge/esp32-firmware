/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": "Charge management"
        },
        "navbar": {
            "charge_manager": "Chargers"
        },
        "content": {
            "charge_manager": "Chargers"
        }
    },
    "event_log": {
        "content": {
            "debug_report_desc_muted": "complete report of all status information of this energy manager except passwords"
        }
    },
    "firmware_update": {
        "content": {
            "factory_reset_modal_text": <>A factory reset will reset all configuration to their default values. <b>This action cannot be undone!</b> The energy manager will re-open the WiFi access point with the default SSID and passphrase from the manual. Are you sure?</>,
            "reboot_desc": "",
            "config_reset_modal_text": "",
            "factory_reset_desc": "removes all configuration",
            "config_reset_desc": ""
        },
        "script": {
            "no_info_page": "Firmware file corrupted or for WARP Charger (info page missing)",
            "wrong_firmware_type": "Firmware file does not match the type of this energy manager"
        }
    },
    "mqtt": {
        "content": {
            "enable_mqtt_desc": <>This allows controlling the energy manager over the configured MQTT broker. <a href="{{{apidoc_url}}}">MQTT API documentation</a></>
        }
    },
    "network": {
        "content": {
            "enable_mdns_desc": "Allows other devices in this network to find the energy manager."
        }
    },
    "util": {
        "event_connection_lost_title": "Energy manager connection lost!"
    },
    "wifi": {
        "content": {
            "ap_hide_ssid_desc_pre": "The energy manager is reachable under the BSSID ",
            "confirm_content": <>Are you sure you want to disable the WiFi Access Point? If no connection to a configured network can be established in the future, <b>it is not possible to access the energy manager anymore</b>.The energy manager then has to be reset to factory defaults. <a href="{{{manual_url}}}">See manual for details.</a></>
        }
    },
    "charge_tracker": {
        "content": {
            "charge_tracker_remove_modal_text": <>All tracked charges and the username history will be removed. <b>This action cannot be undone!</b> The energy manager then will restart.</>
        }
    },
    "meter": {
        "status": {
            "charge_history": "Consumption history"
        }
    }
}
