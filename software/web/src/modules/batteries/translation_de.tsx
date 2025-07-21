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

            "dynamic_tariff_schedule": "Zeitplan für dynamischen Strompreis",
            "schedule_cheap_hours": "Günstige Stunden",
            "schedule_cheap_hours_help": "Wähle die x günstigsten Stunden in einem 24-Stunden-Zeitfenster zwischen 20:00 und 20:00 des Folgetages aus. Wenn der Batteriespeicher aus dem Netz geladen werden soll, ist es sinnvoll, hier die Ladezeit des Speichers einzutragen. Viertelstunden können eingegeben werden, indem Vielfache von 0,25 eingegeben werden. 1,25 h ist eine Stunde und 15 Minuten.",
            "schedule_expensive_hours": "Teure Stunden",
            "schedule_expensive_hours_help": "Wähle die x teuersten Stunden in einem 24-Stunden-Zeitfenster zwischen 20:00 und 20:00 des Folgetages aus. Viertelstunden können eingegeben werden, indem Vielfache von 0,25 eingegeben werden. 1,25 h ist eine Stunde und 15 Minuten.",
            "schedule_hours_muted": "in 0,25 h-Schritten",

            "forbid_discharge_during_fast_charge": "Batterieentladung beim Schnellladen verbieten",
            "forbid_discharge_during_fast_charge_desc": "Verbietet dem Speicher, sich zu entladen, während ein Fahrzeug schnell lädt oder sein Eco-Ladeplan es erlaubt.",

            "managed_batteries": "Kontrollierte Batteriespeicher",

            "table_battery_display_name": "Anzeigename",
            "table_battery_class": "Klasse",

            "add_battery_title": "Batteriespeicher hinzufügen",
            "add_battery_message": /*SFN*/(have: number, max: number) => `${have} von ${max} Batteriespeichern konfiguriert`/*NF*/,
            "add_battery_slot": "Nummer",
            "add_battery_class": "Klasse",

            "edit_battery_title": "Batteriespeicher bearbeiten",
            "edit_battery_slot": "Nummer",
            "edit_battery_class": "Klasse",

            "battery_import_error_json_malformed": "Das JSON-Format ist beschädigt.",
            "battery_import_error_class_unknown": "Klasse ist unbekannt.",
            "battery_import_error_import_unsupported": "Importieren wird für diese Klasse nicht unterstüzt.",
            "battery_import_error_config_malformed": "Das Konfigurations-Format ist beschädigt.",
            "battery_export_basename": "Batteriespeicher-Konfiguration",

            "rules_permit_grid_charge": "Regeln für Ladung-vom-Netz-Erlaubnis",
            "rules_forbid_discharge": "Regeln für Batterieentladungs-Verbot",
            "rules_forbid_charge": "Regeln für Batterieladungs-Verbot",

            "table_rule_desc": "Beschreibung",
            "table_rule_soc": "Batterieladestand",
            "table_rule_price": "Dynamischer Strompreis",
            "table_rule_forecast": "PV-Ertragsprognose",
            "table_rule_schedule": "Dynamischer Strompreisplan",

            "add_rule_title": "Regel hinzufügen",
            "add_rule_message": /*SFN*/(have: number, max: number) => `${have} von ${max} Regeln konfiguriert`/*NF*/,
            "add_rule_desc": "Beschreibung",
            "add_rule_soc": "Batterieladestand",
            "add_rule_price": "Dynamischer Strompreis",
            "add_rule_forecast": "PV-Ertragsprognose",
            "add_rule_schedule": "Dynamischer Strompreisplan",

            "edit_rule_title": "Regel bearbeiten",
            "edit_rule_desc": "Beschreibung",
            "edit_rule_soc": "Batterieladestand",
            "edit_rule_price": "Dynamischer Strompreis",
            "edit_rule_forecast": "PV-Ertragsprognose",
            "edit_rule_schedule": "Dynamischer Strompreisplan",

            "condition_ignore": "Wird ignoriert",
            "condition_below": "Kleiner als",
            "condition_above": "Größer als",

            "condition_schedule_cheap": "Günstige Stunden",
            "condition_schedule_not_cheap": "Nicht günstige Stunden",
            "condition_schedule_expensive": "Teure Stunden",
            "condition_schedule_not_expensive": "Nicht teure Stunden",
            "condition_schedule_moderate": "Preislich moderate Stunden (weder günstig noch teuer)",

            "invalid_feedback_all_ignored": "Es können nicht alle Bedingungen gleichzeitig ignoriert werden."
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
