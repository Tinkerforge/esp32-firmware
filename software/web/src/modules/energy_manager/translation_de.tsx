/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "energy_manager": {
        "status": {
            "mode": "Lademodus",
            "mode_fast": "Schnell",
            "mode_off": "Aus",
            "mode_pv": "PV",
            "mode_min_pv": "Min + PV",
            "phase_switching": "Phasenumschaltung",
            "single_phase": "Einphasig",
            "three_phase":"Dreiphasig",
            "external_control_state": "Externe Steuerung",
            "external_control_state_available": "Verfügbar",
            "external_control_state_disabled": "Deaktiviert",
            "external_control_state_unavailable": "Nicht verfügbar",
            "external_control_state_switching": "Schaltet gerade",
            "external_control_request": "Externe Phasenanforderung",
            "external_control_request_none": "Keine",
            "status": "Energiemanager",
            "error_ok": "OK",
            "error_network": "Netzwerkfehler",
            "error_contactor": "Schützfehler",
            "error_config": "Unvollständige Konfiguration",
            "error_internal": "Interner Fehler",
            "config_error_phase_switching": "Phasenumschaltung oder Schütz nicht konfiguriert",
            "config_error_no_max_current": "Maximaler Gesamtstrom der Wallboxen nicht konfiguriert",
            "config_error_no_chargers": "Keine Wallboxen konfiguriert",
            "config_error_excess_no_meter": "Überschussladen aktiviert aber kein Stromzähler eingerichtet",

            "no_bricklet": "Interner Fehler, Bricklet nicht gefunden"
        },
        "navbar": {
            "energy_manager": "Einstellungen"
        },
        "content": {
            "page_header": "Energiemanager",

            "header_phase_switching": "Phasenumschaltung",
            "contactor_installed": "Schütz angeschlossen",
            "contactor_installed_desc": "Der Energiemanager verfügt über ein Schütz, mit dem die von ihm verwalteten Wallboxen zwischen ein- und dreiphasigem Anschluss umgeschaltet werden können.",
            "phase_switching_mode": "Umschaltungsmodus",
            "fixed_single_phase": "Fest einphasig angeschlossen",
            "fixed_three_phases": "Fest dreiphasig angeschlossen",
            "automatic": "Automatisch",
            "always_single_phase": "Immer einphasig",
            "always_three_phases": "Immer dreiphasig",
            "pv1p_fast3p": "Einphasiger PV-Modus, dreiphasiger Schnell-Modus",
            "external_control": "Externe Steuerung (EVCC)",
            "external_control_notification": "Externe Steuerung für die Phasenumschaltung ist aktiv. Dadurch werden alle Einstellungen im Zusammenhang mit PV-Überschussladen deaktiviert.",

            "header_load_management": "Dynamisches Lastmanagement",
            "load_management_explainer": "Der Energiemanager regelt die verwalteten Wallboxen so, dass der maximal zulässige Strom des Gebäudeanschlusses auf keiner Phase überschritten wird. (Bald verfügbar: Diese Funktion befindet sich aktuell noch in der Entwicklung.)",

            "header_excess_charging": "Photovoltaik-Überschussladen",
            "enable_excess_charging": "Überschussladen aktiviert",
            "enable_excess_charging_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "enable_excess_charging_desc": "Der Energiemanager regelt die verwalteten Wallboxen abhängig vom gemessenen Überschuss einer Photovoltaikanlage und vom Lademodus.",
            "default_mode": "Standardlademodus",
            "default_mode_muted": "wird nach Neustart des Energiemanagers verwendet",
            "auto_reset_charging_mode": "Tägliches Zurücksetzen",
            "auto_reset_charging_mode_desc": "Setzt den Lademodus täglich auf den Standardlademodus zurück.",
            "auto_reset_time": "Zurücksetzen um",
            "meter_slot_grid_power": "Stromzähler",
            "meter_slot_grid_power_muted": "für Überschussladen",
            "meter_slot_grid_power_select": "Auswählen...",
            "meter_slot_grid_power_none": "Keine Stromzähler konfiguriert",
            "guaranteed_power": "Min + PV: Mindestladeleistung",
            "guaranteed_power_muted": "Leistung, die im Min + PV-Modus zum Laden von Fahrzeugen aus dem Netz bezogen werden darf.",
            "target_power_from_grid": "Soll-Netzbezug",
            "target_power_from_grid_muted": "Sollwert für Überschussregelung",
            "control_behavior": "Regelverhalten",
            "control_behavior_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "target_power_n200": "Sehr konservativ (− 200 W)",
            "target_power_n100": "Konservativ (− 100 W)",
            "target_power_n50": "Leicht konservativ (− 50 W) – lädt Batteriespeicher zuerst",
            "target_power_0": "Ausgeglichen (± 0 W) – empfohlen ohne Batteriespeicher",
            "target_power_p50": "Leicht aggressiv (+ 50 W) – verwendet Batteriespeicher für Fahrzeugladevorgänge",
            "target_power_p100": "Aggressiv (+ 100 W)",
            "target_power_p200": "Sehr aggressiv (+ 200 W)",
            "cloud_filter": "Wolkenfilter",
            "cloud_filter_muted": "reduziert den Einfluss von wechselnd bewölktem Wetter",
            "cloud_filter_off": "Ohne",
            "cloud_filter_weak": "Schwach",
            "cloud_filter_medium": "Mittel",
            "cloud_filter_strong": "Stark",

            "relay": "Relais",
            "single_phase": "Einphasig",
            "three_phase": "Dreiphasig",
            "input3_rule_then": "Modus",
            "input4_rule_then": "Modus",
            "relay_config": "Modus",
            "relay_manual": "Manuell gesteuert oder nicht verwendet",
            "relay_rules": "Regelbasiert",
            "relay_rule_when": "Wenn",
            "relay_rule_is": "Ist",
            "relay_config_then": "Dann",
            "relay_config_close": "Relais schließen",
            "low": "Offen",
            "high": "Geschlossen",
            "contactor_fail": "Ausgelöst, Schütz defekt",
            "contactor_ok": "Nicht ausgelöst, Schütz OK",
            "power_available": "Leistung verfügbar",
            "power_sufficient": "Ausreichend für einen Ladevorgang",
            "power_insufficient": "Nicht ausreichend für einen Ladevorgang",
            "grid_draw": "Netzbezug",
            "grid_gt0": "Bezieht Energie aus dem Netz",
            "grid_le0": "Speist Energie ins Netz ein",

            "input3": "Eingang 3",
            "input4": "Eingang 4",
            "input_unused": "Nicht verwendet",
            "contactor_check": "Schützüberwachung",
            "block_charging": "Laden blockieren",
            "limit_max_current": "Ladestrom begrenzen",
            "limit_to_current": "Begrenzen auf",
            "input_when": "Wenn",
            "input_low": "Eingang offen",
            "input_high": "Eingang geschlossen",
            "phase_switching": "Phasenumschaltung",
            "input_switch_mode": "Lademodus wechseln",
            "input_when_closing": "Beim Schließen des Eingangs",
            "input_when_opening": "Beim Öffnen des Eingangs",
            "input_switch_to_prefix": "Zu ",
            "input_switch_to_suffix": "",
            "input_mode_nothing": "Nicht wechseln",

            "header_expert_settings": "Experteneinstellungen",
            "hysteresis_time": "Hysterese-Zeit",
            "hysteresis_time_muted": "Minimale Wartezeit vor Phasenumschaltungen bzw. Anfang und Ende eines Ladevorganges, um die Ladeelektronik des Fahrzeuges nicht durch häufige Schaltvorgänge zu belasten."
        },
        "cron": {
            "switch_contactor": "Relay schalten",
            "contactor_state": "Schalten auf",
            "contactor_state_open": "Offen",
            "contactor_state_closed": "Geschlossen",
            "contactor_action_text": /*FFN*/(state: boolean) => {
                let ret = state ? <><b>schließen</b></> : <><b>öffnen</b></>
                return <>Relay {ret}.</>
            }/*NF*/,
            "charge_mode_default": "Standardmodus",
            "charge_mode_switch": "Lademodus wechseln",
            "charge_mode": "Lademodus",
            "fast": "Schnell",
            "disabled": "Deaktiviert",
            "pv_excess": "PV-Überschuss",
            "guaranteed_power": "Min + PV: Mindestladeleistung",
            "charge_mode_switch_action_text": /*FFN*/(mode: number) => {
                let ret = <></>;
                switch (mode) {
                    case 0:
                        ret = <><b>Schnell</b></>
                        break;

                    case 1:
                        ret = <><b>Deaktiviert</b></>
                        break;

                    case 2:
                        ret = <><b>PV-Überschuss</b></>
                        break;

                    case 3:
                        ret = <><b>Min + PV</b></>
                        break;

                    default:
                        ret = <><b>Standardmodus</b></>
                        break;
                }
                return <>wechsel Lademodus auf {ret}.</>
            }/*NF*/,
            "set_phases": "Phasenumschaltung",
            "phases_wanted": "Umschaltung auf",
            "single_phase": "Einphasig",
            "three_phase": "Dreiphasig",
            "cron_action_text": /*SFN*/(phases: number) => "Umschaltung auf " + (phases == 1 ? "Einphasig." : "Dreiphasig.")/*NF*/
        },
        "script": {
            "save_failed": "Speichern der Energiemanager-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Energiemanager-Einstellungen",
            "mode_change_failed": "Wechsel des Lademodus fehlgeschlagen.",
            "meter": /*SFN*/(slot: number|"?") => "Zähler #" + slot/*NF*/
        }
    }
}
