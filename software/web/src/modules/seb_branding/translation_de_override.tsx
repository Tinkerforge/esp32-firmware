/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": "Lastmanagement"
        }
    },
    "ethernet": {
        "content": {
            "enable_desc": "Der SEB verbindet sich beim Start automatisch zum konfigurierten Netzwerk"
        }
    },
    "system": {
        "content": {
            "factory_reset_modal_text": <>Hiermit wird die gesamte Konfiguration auf den Werkszustand zurückgesetzt. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Der SEB öffnet dann wieder einen eigenen WLAN-Access-Point mit dem Netzwerknamen (SSID) und der Passphrase aus der beiliegenden Betriebsanleitung. Fortfahren?</>,
            "reboot_desc": "",
            "config_reset_modal_text": "",
            "factory_reset_desc": "löscht die gesamte Konfiguration",
            "config_reset_desc": ""
        }
    },
    "firmware_update": {
        "script": {
            "install_state_19": "Firmware-Datei ist beschädigt oder für ein anderes Produkt (Firmware-Info-Seite fehlt)",
            "install_state_21": "Firmware-Datei passt nicht zum SEB"
        }
    },
    "network": {
        "content": {
            "enable_mdns_desc": "Erlaubt es anderen Geräten in diesem Netzwerk, der SEB zu finden."
        }
    },
    "mqtt": {
        "content": {
            "enable_mqtt_desc": <>Hierdurch kann der SEB über den konfigurierten MQTT-Broker kontrolliert werden. <a href="{{{apidoc_url}}}">MQTT-API-Dokumentation</a></>
        }
    },
    "util": {
        "event_connection_lost_title": "Verbindung zum SEB verloren!"
    },
    "wifi": {
        "content": {
            "sta_enable_sta_desc": "Der SEB verbindet sich beim Start automatisch zum konfigurierten Netzwerk",
            "ap_hide_ssid_desc_pre": "Der SEB ist unter der BSSID ",
            "confirm_content": <>Soll der WLAN-Access-Point wirklich komplett deaktiviert werden? Falls zukünftig keine Verbindung zu einem konfigurierten Netzwerk möglich ist, <b>kann nicht mehr auf den SEB zugegriffen werden</b>. Der SEB muss dann auf den Werkszustand zurückgesetzt werden. <a href="{{{manual_url}}}">Siehe Betriebsanleitung für Details.</a></>
        }
    },
    "meters": {
        "status": {
            "power_history": "Leistungsverlauf"
        }
    },
    "power_manager": {
        "content": {
            "cm_requirements_warning": "Keine kontrollierten Wallboxen eingetragen. Um das PV-Überschussladen zu nutzen, müssen alle angeschlossenen Wallboxen im Lastmanagement als kontrollierte Wallboxen eingetragen sein."
        }
    },
    "remote_access": {
        "content": {
            "registration": "Registriere SEB"
        }
    }
}
