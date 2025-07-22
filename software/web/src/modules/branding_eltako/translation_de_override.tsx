/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": "Lastmanager"
        },
        "script": {
            "mode_explainer_0": <>{__("This_device")} ist nicht Teil eines Lastmanagement-Verbunds mit anderen ELTAKO Wallboxen. PV-Überschussladen ist deaktiviert.</>,
            "mode_explainer_1": <>{__("This_device")} ist Teil eines Lastmanagement-Verbunds mit anderen ELTAKO Wallboxen. Ein anderes Gerät steuert diesen Lastmanagement-Verbund, um sicherzustellen, dass nie mehr als der verfügbare Strom bezogen wird.</>,
            "mode_explainer_2": <>{__("This_device")} verwendet entweder das PV-Überschussladen oder steuert einen Lastmanagement-Verbund mit anderen ELTAKO Wallboxen um sicherzustellen, dass nie mehr als der verfügbare Strom bezogen wird.</>
        }
    },
    "system": {
        "content": {
            "factory_reset_modal_text": <>Hiermit wird die gesamte Konfiguration auf den Werkszustand zurückgesetzt. Alle aufgezeichneten Ladevorgänge gehen verloren. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Die ELTAKO Wallbox öffnet dann wieder einen eigenen WLAN-Access-Point mit dem Netzwerknamen (SSID) und der Passphrase aus der beiliegenden Betriebsanleitung. Fortfahren?</>,
            "config_reset_modal_text": <>Hiermit wird nur die Konfiguration auf den Werkszustand zurückgesetzt. Aufgezeichnete Ladevorgänge bleiben erhalten. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Die ELTAKO Wallbox öffnet dann wieder einen eigenen WLAN-Access-Point mit dem Netzwerknamen (SSID) und der Passphrase aus der beiliegenden Betriebsanleitung. Fortfahren?</>,
            "reboot_desc": "ein laufender Ladevorgang wird nicht unterbrochen",
            "factory_reset_desc": "löscht die Konfiguration und alle aufgezeichneten Ladevorgänge",
            "config_reset_desc": "aufgezeichnete Ladevorgänge bleiben erhalten"
        }
    },
    "firmware_update": {
        "script": {
            "install_state_19": "Firmware-Datei ist beschädigt (Firmware-Info-Seite fehlt)"
        }
    },
    "meters": {
        "status": {
            "power_history": "Ladeverlauf"
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
        "device": "ELTAKO Wallbox",
        "the": "die",
        "The": "Die",
        "this": "diese",
        "This": "Diese",
        "to_the": "zur",
        "from_the": "von der"
    }
}
