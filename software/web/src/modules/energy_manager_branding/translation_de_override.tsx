/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": "Lastmanagement"
        },
        "navbar": {
            "charge_manager": "Wallboxen"
        },
        "content": {
            "charge_manager": "Wallboxen"
        }
    },
    "ethernet": {
        "content": {
            "enable_desc": "Der Energy Manager verbindet sich beim Start automatisch zum konfigurierten Netzwerk"
        }
    },
    "event_log": {
        "content": {
            "debug_report_desc_muted": "kompletter Report aller Statusinformationen des Energy Managers außer Passwörtern"
        }
    },
    "firmware_update": {
        "content": {
            "factory_reset_modal_text": "Hiermit wird die gesamte Konfiguration auf den Werkszustand zurückgesetzt. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Der Energy Manager öffnet dann wieder einen eigenen WLAN-Access-Point mit dem Netzwerknamen (SSID) und der Passphrase aus der beiliegenden Betriebsanleitung. Fortfahren?",
            "reboot_desc": "",
            "config_reset_modal_text": "",
            "factory_reset_desc": "löscht die gesamte Konfiguration",
            "config_reset_desc": ""
        },
        "script": {
            "no_info_page": "Firmware-Datei ist beschädigt oder für WARP Charger (Firmware-Info-Seite fehlt)",
            "wrong_firmware_type": "Firmware-Datei passt nicht zum Energy Manager"
        }
    },
    "network": {
        "content": {
            "enable_mdns_desc": "Erlaubt es anderen Geräten in diesem Netzwerk, der Energy Manager zu finden."
        }
    },
    "mqtt": {
        "content": {
            "enable_mqtt_desc": "Hierdurch kann der Energy Manager über den konfigurierten MQTT-Broker kontrolliert werden. <a href=\"{{{apidoc_url}}}\">MQTT-API-Dokumentation</a>"
        }
    },
    "util": {
        "event_connection_lost_title": "Verbindung zum Energy Manager verloren!"
    },
    "wifi": {
        "content": {
            "sta_enable_sta_desc": "Der Energy Manager verbindet sich beim Start automatisch zum konfigurierten Netzwerk",
            "ap_hide_ssid_desc_pre": "Der Energy Manager ist unter der BSSID ",
            "confirm_content": <>Soll der WLAN-Access-Point wirklich komplett deaktiviert werden? Falls zukünftig keine Verbindung zu einem konfigurierten Netzwerk möglich ist, <b>kann nicht mehr auf den Energy Manager zugegriffen werden</b>. Der Energy Manager muss dann auf den Werkszustand zurückgesetzt werden. <a href="{{{manual_url}}}">Siehe Betriebsanleitung für Details.</a></>
        }
    },
    "charge_tracker": {
        "content": {
            "charge_tracker_remove_modal_text": "Alle aufgezeichneten Ladevorgänge und der Benutzernamen-Verlauf werden gelöscht. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Danach wird der Energy Manager neugestartet."
        }
    },
    "meter": {
        "status": {
            "charge_history": "Verbrauchsverlauf"
        }
    }
}
