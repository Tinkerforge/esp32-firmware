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

            "permitted_no": "Nicht erlaubt",
            "permitted_yes": "Erlaubt",
            "forbidden_no": "Nicht verboten",
            "forbidden_yes": "Verboten",

            "grid_charge_permitted": "Ladung vom Netz",
            "discharge_forbidden": "Batterieentladung",
            "charge_forbidden": "Batterieladung",

            "forbid_discharge_during_fast_charge": "Batterieentladung beim Schnellladen verbieten",
            "forbid_discharge_during_fast_charge_desc": "Verbietet dem Speicher, sich zu entladen, während ein Fahrzeug schnell lädt oder sein Eco-Ladeplan es erlaubt.",

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
            "edit_battery_class": "Klasse",

            "battery_import_error_json_malformed": "Das JSON-Format ist beschädigt.",
            "battery_import_error_class_unknown": "Klasse ist unbekannt.",
            "battery_import_error_import_unsupported": "Importieren wird für diese Klasse nicht unterstüzt.",
            "battery_import_error_config_malformed": "Das Konfigurations-Format ist beschädigt.",
            "battery_export_basename": "Batteriespeicher-Konfiguration"
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
