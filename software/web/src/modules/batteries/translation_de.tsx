/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "batteries": {
        "status": {
        },
        "navbar": {
            "batteries": "Speichersteuerung"
        },
        "content": {
            "batteries": "Speichersteuerung",

            "discharge_blocked": "Batterieentladung",
            "discharge_blocked_no": "Nicht verboten",
            "discharge_blocked_yes": "Verboten",

            "block_discharge_during_fast_charge": "Batterieentladung beim Schnellladen verbieten",
            "block_discharge_during_fast_charge_desc": "Verbietet dem Speicher, sich zu entladen, während ein Fahrzeug schnell lädt oder sein Eco-Ladeplan es erlaubt.",

            "managed_batteries": "Kontrollierte Batteriespeicher",

            "table_display_name": "Anzeigename",
            "table_class": "Klasse",

            "add_battery_title": "Batteriespeicher hinzufügen",
            "add_battery_prefix": "",
            "add_battery_infix": " von ",
            "add_battery_suffix": " Batteriespeichern konfiguriert",
            "add_battery_slot": "Nummer",
            "add_battery_class": "Klasse",

            "edit_battery_title": "Batteriespeicher bearbeiten",
            "edit_battery_slot": "Nummer",
            "edit_battery_class": "Klasse"
        },
        "script": {
            "save_failed": "Speichern der Speichersteuerungs-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Speichersteuerungs-Einstellungen",
            "battery": /*SFN*/(slot: number|"?") => "Speicher #" + slot/*NF*/
        },
        "automation": {
            "permit_grid_charge": "Laden des Speichers vom Netz erlauben",
            "revoke_grid_charge_override": "Erlaubnis zum Laden des Speichers vom Netz zurücknehmen",
            "forbid_discharge": "Entladen des Speichers verbieten",
            "revoke_discharge_override": "Verbot zum Entladen des Speichers zurücknehmen"
        }
    }
}
