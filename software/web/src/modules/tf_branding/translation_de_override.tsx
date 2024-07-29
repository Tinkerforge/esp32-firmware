/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": "Lastmanager"
        }
    },
    "ethernet": {
        "content": {
            "enable_desc": "Der Brick verbindet sich automatisch beim Start oder wenn ein Kabel eingesteckt wird"
        }
    },
    "system": {
        "content": {
            "factory_reset_modal_text": <>Hiermit wird die gesamte Konfiguration auf den Werkszustand zurückgesetzt. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Der Brick öffnet dann wieder einen eigenen WLAN-Access-Point mit dem Netzwerknamen (SSID) und der Passphrase aus dem Etikett auf der Rückseite des Bricks. Fortfahren?</>,
            "reboot_desc": "",
            "config_reset_modal_text": "",
            "factory_reset_desc": "löscht die gesamte Konfiguration",
            "config_reset_desc": ""
        }
    },
    "firmware_update": {
        "script": {
            "install_state_19": "Firmware-Datei ist beschädigt (Firmware-Info-Seite fehlt)",
            "install_state_21": "Firmware-Datei passt nicht zu diesem Brick-Typ"
        }
    },
    "mqtt": {
        "content": {
            "enable_mqtt_desc": <>Hierdurch kann der Brick über den konfigurierten MQTT-Broker kontrolliert werden. <a href="{{{apidoc_url}}}">MQTT-API-Dokumentation</a></>
        }
    },
    "network": {
        "content": {
            "enable_mdns_desc": "Erlaubt es anderen Geräten in diesem Netzwerk den Brick zu finden."
        }
    },
    "util": {
        "event_connection_lost_title": "Verbindung zum Brick verloren!"
    },
    "wifi": {
        "content": {
            "sta_enable_sta_desc": "Der Brick verbindet sich beim Start automatisch zum konfigurierten Netzwerk",
            "ap_hide_ssid_desc_pre": "Der Brick ist unter der BSSID ",
            "confirm_content": <>Soll der WLAN-Access-Point wirklich komplett deaktiviert werden? Falls zukünftig keine Verbindung zu einem konfigurierten Netzwerk möglich ist, <b>kann nicht mehr auf den Brick zugegriffen werden</b>. Der Brick muss dann auf den Werkszustand zurückgesetzt werden. <a href="{{{manual_url}}}">Siehe Betriebsanleitung für Details.</a></>
        }
    }
}
