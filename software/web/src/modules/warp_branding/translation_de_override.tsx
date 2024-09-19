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
            "enable_desc": "Die Wallbox verbindet sich automatisch beim Start oder wenn ein Kabel eingesteckt wird"
        }
    },
    "system": {
        "content": {
            "factory_reset_modal_text": <>Hiermit wird die gesamte Konfiguration auf den Werkszustand zurückgesetzt. Alle aufgezeichneten Ladevorgänge gehen verloren. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Die Wallbox öffnet dann wieder einen eigenen WLAN-Access-Point mit dem Netzwerknamen (SSID) und der Passphrase aus der beiliegenden Betriebsanleitung. Fortfahren</>,
            "config_reset_modal_text": <>Hiermit wird nur die Konfiguration auf den Werkszustand zurückgesetzt. Aufgezeichnete Ladevorgänge bleiben erhalten. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Die Wallbox öffnet dann wieder einen eigenen WLAN-Access-Point mit dem Netzwerknamen (SSID) und der Passphrase aus der beiliegenden Betriebsanleitung. Fortfahren</>,
            "reboot_desc": "ein laufender Ladevorgang wird nicht unterbrochen",
            "factory_reset_desc": "löscht die Konfiguration und alle aufgezeichneten Ladevorgänge",
            "config_reset_desc": "aufgezeichnete Ladevorgänge bleiben erhalten"
        }
    },
    "firmware_update": {
        "script": {
            "install_state_19": "Firmware-Datei ist beschädigt oder für WARP1 Charger (Firmware-Info-Seite fehlt)",
            "install_state_21": "Firmware-Datei passt nicht zu diesem Wallbox-Typ"
        }
    },
    "network": {
        "content": {
            "enable_mdns_desc": "Erlaubt es anderen Geräten in diesem Netzwerk die Wallbox zu finden"
        }
    },
    "mqtt": {
        "content": {
            "enable_mqtt_desc": <>Hierdurch kann die Wallbox über den konfigurierten MQTT-Broker kontrolliert werden. <a href="{{{apidoc_url}}}">MQTT-API-Dokumentation</a></>
        }
    },
    "util": {
        "event_connection_lost_title": "Verbindung zur Wallbox verloren!"
    },
    "wifi": {
        "content": {
            "sta_enable_sta_desc": "Die Wallbox verbindet sich beim Start automatisch zum konfigurierten Netzwerk",
            "ap_hide_ssid_desc_pre": "Die Wallbox ist unter der BSSID ",
            "confirm_content": <>Soll der WLAN-Access-Point wirklich komplett deaktiviert werden? Falls zukünftig keine Verbindung zu einem konfigurierten Netzwerk möglich ist, <b>kann nicht mehr auf die Wallbox zugegriffen werden</b>. Die Wallbox muss dann auf den Werkszustand zurückgesetzt werden. <a href="{{{manual_url}}}">Siehe Betriebsanleitung für Details.</a></>
        }
    },
    "charge_tracker": {
        "content": {
            "charge_tracker_remove_modal_text": <>Alle aufgezeichneten Ladevorgänge und der Benutzernamen-Verlauf werden gelöscht. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Danach wird die Wallbox neugestartet.</>
        }
    },
    "meters": {
        "status": {
            "power_history": "Ladeverlauf"
        }
    },
    "power_manager": {
        "content": {
            "cm_requirements_warning": "Um das PV-Überschussladen zu nutzen, muss das Lastmanagement auf „Lastmanager“ eingestellt und nur diese Wallbox als kontrollierte Wallbox eingetragen sein."
        }
    },
    "remote_access": {
        "content": {
            "registration": "Registriere Wallbox"
        }
    }
}
