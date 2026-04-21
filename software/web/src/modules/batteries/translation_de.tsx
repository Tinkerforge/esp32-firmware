/** @jsxImportSource preact */
import { h } from "preact";
import { BatteryMode } from "modules/batteries/generated/battery_mode.enum";
let x = {
    "batteries": {
        "status": {
            "batteries": "Batteriesteuerung"
        },
        "navbar": {
            "batteries": "Batteriesteuerung"
        },
        "content": {
            "batteries": "Batteriesteuerung",

            "experimental": "Die Batteriesteuerung ist experimentell; Änderungen vorbehalten.",

            "battery": /*SFN*/(slot: number|"?") => "Speicher #" + slot/*NF*/,

            "test_warnings": "Tests",
            "test_warning": /*SFN*/(display_name: string, mode_name: string) => `Für Batteriespeicher ${display_name} läuft ein Test: ${mode_name}.`/*NF*/,

            "battery_mode_block": "Blockieren",
            "battery_mode_normal": "Normal",
            "battery_mode_block_discharge": "Entladen blockieren",
            "battery_mode_force_charge": "Laden erzwingen",
            "battery_mode_block_charge": "Laden blockieren",
            "battery_mode_force_discharge": "Entladen erzwingen",

            "battery_mode_long_block": "Laden blockieren, Entladen blockieren",
            "battery_mode_long_normal": "Normal Laden, normal Entladen",
            "battery_mode_long_block_discharge": "Normal Laden, Entladen blockieren",
            "battery_mode_long_force_charge": "Laden erzwingen, Entladen blockieren",
            "battery_mode_long_block_charge": "Laden blockieren, normal Entladen",
            "battery_mode_long_force_discharge": "Laden blockieren, Entladen erzwingen",

            "battery_status_blocked": "Blockiert",
            "battery_status_normal": "Normal",
            "battery_status_discharge_blocked": "Entladen blockiert",
            "battery_status_charge_forced": "Laden erzwungen",
            "battery_status_charge_blocked": "Laden blockiert",
            "battery_status_discharge_forced": "Entladen erzwungen",
            "battery_status_unknown": "Unbekannt",
            "battery_status_by_index": /*SFN*/(mode: BatteryMode) => {
                return [
                    __("batteries.content.battery_status_unknown"),
                    __("batteries.content.battery_status_blocked"),
                    __("batteries.content.battery_status_normal"),
                    __("batteries.content.battery_status_discharge_blocked"),
                    __("batteries.content.battery_status_charge_forced"),
                    __("batteries.content.battery_status_charge_blocked"),
                    __("batteries.content.battery_status_discharge_forced"),
                ][mode + 1];
            }/*NF*/,

            "status_charge": "Laden",
            "status_discharge": "Entladen",
            "status_normal": "Normal",
            "status_block": "Blockiert",
            "status_force": "Erzwungen",

            "dynamic_tariff_schedule": "Zeitplan für dynamischen Strompreis",
            "schedule_cheap_hours": "Günstige Stunden",
            "schedule_cheap_hours_help": "Wähle die x günstigsten Stunden in einem 24-Stunden-Zeitfenster zwischen 20:00 und 20:00 des Folgetages aus. Wenn der Batteriespeicher aus dem Netz geladen werden soll, ist es sinnvoll, hier die Ladezeit des Speichers einzutragen. Viertelstunden können eingegeben werden, indem Vielfache von 0,25 eingegeben werden. 1,25 h ist eine Stunde und 15 Minuten.",
            "schedule_expensive_hours": "Teure Stunden",
            "schedule_expensive_hours_help": "Wähle die x teuersten Stunden in einem 24-Stunden-Zeitfenster zwischen 20:00 und 20:00 des Folgetages aus. Viertelstunden können eingegeben werden, indem Vielfache von 0,25 eingegeben werden. 1,25 h ist eine Stunde und 15 Minuten.",
            "schedule_hours_muted": "in 0,25 h-Schritten",
            "schedule_graph": "Zeitplan",
            "schedule_graph_muted": "jeweils von 20:00 bis 20:00",

            "enable_battery_control": "Batteriesteuerung aktiviert",

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

            "rules_charge": "Laderegeln",
            "rules_discharge": "Entladeregeln",

            "table_rule_enabled": "Aktiviert",
            "table_rule_desc": "Beschreibung",
            "table_rule_time": "Zeit",
            "table_rule_soc": "Ladestand",
            "table_rule_price": "Strompreis",
            "table_rule_forecast": "PV-Ertrag",
            "table_rule_schedule": "Strompreisplan",
            "table_rule_fast_chg": "Eco-/Schnellladung",
            "table_rule_action": "Aktion",

            "table_rule_fast_chg_not_active": "Nicht aktiv",
            "table_rule_fast_chg_active": "Aktiv",

            "add_rule_title_charge": "Laderegel hinzufügen",
            "add_rule_title_discharge": "Entladeregel hinzufügen",
            "add_rule_message": /*SFN*/(have: number, max: number) => `${have} von ${max} Regeln konfiguriert`/*NF*/,
            "add_rule_desc": "Beschreibung",
            "add_rule_time": "Zeitraum",
            "add_rule_time_start": "von",
            "add_rule_time_end": "bis",
            "add_rule_soc": "Batterieladestand",
            "add_rule_price": "Dynamischer Strompreis",
            "add_rule_price_muted": "Börsenpreis",
            "add_rule_forecast": "PV-Ertragsprognose",
            "add_rule_schedule": "Dynamischer Strompreisplan",
            "add_rule_fast_chg": "Fahrzeug-Eco-/Schnellladung",
            "add_rule_action": "Aktion",

            "edit_rule_title_charge": "Laderegel bearbeiten",
            "edit_rule_title_discharge": "Entladeregel bearbeiten",
            "edit_rule_desc": "Beschreibung",
            "edit_rule_time": "Zeitraum",
            "edit_rule_time_start": "von",
            "edit_rule_time_end": "bis",
            "edit_rule_soc": "Batterieladestand",
            "edit_rule_price": "Dynamischer Strompreis",
            "edit_rule_price_muted": "Börsenpreis",
            "edit_rule_forecast": "PV-Ertragsprognose",
            "edit_rule_schedule": "Dynamischer Strompreisplan",
            "edit_rule_fast_chg": "Fahrzeug-Eco-/Schnellladung",
            "edit_rule_action": "Aktion",

            "condition_ignore": "Wird ignoriert",
            "condition_below": "Kleiner als",
            "condition_above": "Größer als",

            "condition_time_not": "Nicht",
            "condition_time_only": "Nur",

            "condition_schedule_cheap": "Günstige Stunden",
            "condition_schedule_cheap_compact": "Günstig",
            "condition_schedule_not_cheap": "Nicht günstige Stunden",
            "condition_schedule_not_cheap_compact": "Nicht günstig",
            "condition_schedule_expensive": "Teure Stunden",
            "condition_schedule_expensive_compact": "Teuer",
            "condition_schedule_not_expensive": "Nicht teure Stunden",
            "condition_schedule_not_expensive_compact": "Nicht teuer",
            "condition_schedule_moderate": "Preislich moderate Stunden (weder günstig noch teuer)",
            "condition_schedule_moderate_compact": "Moderat",

            "condition_fast_chg_inactive": "Keine Eco-/Schnellladung aktiv",
            "condition_fast_chg_active": "Eco-/Schnellladung aktiv",

            "rule_action_block": "Blockieren",
            "rule_action_force": "Erzwingen",

            "rule_action_block_charge": "Laden blockieren",
            "rule_action_force_charge": "Laden erzwingen (blockiert Entladen)",
            "rule_action_block_discharge": "Entladen blockieren",
            "rule_action_force_discharge": "Entladen erzwingen (blockiert Laden)",

            "invalid_feedback_all_ignored": "Es können nicht alle Bedingungen gleichzeitig ignoriert werden."
        },
        "script": {
            "save_failed": "Speichern der Batteriesteuerungs-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Batteriesteuerungs-Einstellungen",
            "battery": /*SFN*/(slot: number|"?") => "Speicher #" + slot/*NF*/
        }
    }
}
