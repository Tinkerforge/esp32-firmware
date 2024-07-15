/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "power_manager": {
        "status": {
            "mode": "Lademodus",
            "mode_fast": "Schnell",
            "mode_off": "Aus",
            "mode_pv": "PV",
            "mode_min_pv": "Min + PV",
            "phase_switching": "Phasenumschaltung",
            "single_phase": "Einphasig",
            "three_phase":"Dreiphasig",
            "config_error_phase_switching": "Phasenumschaltung oder Schütz nicht konfiguriert",
            "config_error_no_max_current": "Maximaler Gesamtstrom der Wallboxen nicht konfiguriert",
            "config_error_no_chargers": "Keine Wallboxen konfiguriert",
            "config_error_excess_no_meter": "Überschussladen aktiviert aber kein Stromzähler eingerichtet"
        },
        "navbar": {
            "power_manager_settings": "Einstellungen",
            "pv_excess_settings": "PV-Überschussladen"
        },
        "content": {
            "page_header": "Energy Manager",

            "enable_pm": "Power Manager aktiviert",
            "enable_pm_desc": "Passt die verfügbare Ladeleistung automatisch an",
            "cm_requirements_warning": null,
            "cm_multiple_chargers_warning": "Um das PV-Überschussladen mit dieser Wallbox zu nutzen, sollten im Lastmanagement keine weiteren Wallboxen als kontrollierte Wallboxen eingetragen sein.",

            "contactor_installed": "Schütz angeschlossen",
            "contactor_installed_desc": "Der Energiemanager verfügt über ein Schütz, mit dem die von ihm kontrollierten Wallboxen zwischen ein- und dreiphasigem Anschluss umgeschaltet werden können.",
            "phase_switching_mode": "Phasenumschaltung",
            "fixed_single_phase": "Fest einphasig angeschlossen",
            "fixed_three_phases": "Fest dreiphasig angeschlossen",
            "automatic": "Automatisch",
            "always_single_phase": "Immer einphasig",
            "always_three_phases": "Immer dreiphasig",
            "pv1p_fast3p": "PV-Modus einphasig, Schnell-Modus dreiphasig",
            "external_control": "Externe Steuerung (EVCC)",
            "external_control_notification": "Externe Steuerung für die Phasenumschaltung ist aktiv. Dadurch werden alle Einstellungen im Zusammenhang mit PV-Überschussladen deaktiviert.",

            "header_excess_charging": "Photovoltaik-Überschussladen",
            "enable_excess_charging": "Überschussladen aktiviert",
            "enable_excess_charging_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "enable_excess_charging_desc": "Regelt die kontrollierten Wallboxen abhängig vom gemessenen Überschuss einer Photovoltaikanlage und vom Lademodus.",
            "default_mode": "Standardlademodus",
            "default_mode_muted": "wird nach Neustart verwendet",
            "meter_slot_grid_power": "Stromzähler",
            "meter_slot_grid_power_muted": "typischerweise am Netzanschluss",
            "meter_slot_grid_power_select": "Auswählen...",
            "meter_slot_grid_power_none": "Kein Stromzähler konfiguriert",
            "meter_slot_grid_power_missing_value": "Summe der gerichteten Wirkleistung fehlt",
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
            "cloud_filter_off": "Aus",
            "cloud_filter_weak": "Schwach",
            "cloud_filter_medium": "Mittel",
            "cloud_filter_strong": "Stark",

            "header_expert_settings": "Experteneinstellungen",
            "hysteresis_time": "Hysterese-Zeit",
            "hysteresis_time_muted": "Minimale Wartezeit vor Phasenumschaltungen bzw. Anfang und Ende eines Ladevorganges, um die Ladeelektronik des Fahrzeuges nicht durch häufige Schaltvorgänge zu belasten."
        },
        "automation": {
            "slot": "Blockierungsslot",
            "block_charge": "Ladevorgänge blockieren",
            "unblock_charge": "Ladevorgänge freigeben",
            "block_mode": "Modus",
            "automation_block_charge_action_text": /*FFN*/(slot: number, block: boolean) => {
                if (block) {
                    return <><b>blockiere</b> das Laden durch Slot <b>{slot}</b>.</>
                }
                return <><b>gebe</b> das Laden durch Slot <b>{slot} frei</b>.</>
            }/*NF*/,

            "limit_max_current": "Begrenze maximalen Gesamtstrom",
            "limit_mode": "Modus",
            "reset_limit_max_current": "Maximalen Gesamtstrom zurücksetzen",
            "max_current": "Maximaler Gesamtstrom",
            "automation_limit_max_current_action_text": /*FFN*/(current: number, default_current: number) => {
                if (current === -1) {
                    return <>setze den maximalen Gesamtstrom zurück auf den Standardwert (<b>{default_current / 1000} A</b>).</>
                }
                return <>begrenze den maximalen Gesamtstrom auf <b>{current / 1000} A</b>.</>
            }/*NF*/,

            "grid_power_draw": "Netzbezug",
            "drawing": "Bezieht Energie aus dem Netz",
            "feeding": "Speist Energie ins Netz ein",
            "automation_grid_power_draw_text": /*FFN*/(drawing_power: boolean) => {
                if (drawing_power) {
                    return <>Wenn <b>Energie</b> aus dem Stromnetz <b>bezogen</b> wird, </>;
                } else {
                    return <>Wenn <b>Energie</b> ins Stromnetz <b>eingespeist</b> wird, </>;
                }
            }/*NF*/,

            "power_available": "Leistung verfügbar",
            "not_available": "Nicht verfügbar",
            "available": "Verfügbar",
            "power": "Leistung",
            "automation_power_available_text": /*FFN*/(power: boolean) => {
                let not = <></>
                if (!power) {
                    not = <><b>nicht </b></>
                }
                return <>Wenn {not}<b>genug</b> Leistung zum Laden verfügbar ist, </>
            }/*NF*/,

            "charge_mode_default": "Standardmodus",
            "charge_mode_switch": "Wechsle Lademodus",
            "charge_mode": "Lademodus",
            "fast": "Schnell",
            "disabled": "Deaktiviert",
            "pv_excess": "PV-Überschuss",
            "guaranteed_power": "Min + PV: Mindestladeleistung",
            "charge_mode_switch_action_text": /*FFN*/(mode: number, default_mode: number) => {
                const modes = ["Schnell", "Deaktiviert", "PV-Überschuss", "Min + PV"];

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
                        ret = <><b>Standardmodus ({modes[default_mode]})</b></>
                        break;
                }
                return <>wechsle Lademodus auf {ret}.</>
            }/*NF*/,

            "set_phases": "Schalte Phasen um",
            "phases_wanted": "Umschaltung auf",
            "single_phase": "auf einphasig",
            "three_phase": "auf dreiphasig",
            "automation_action_text": /*FFN*/(phases: number) => <>schalte auf <b>{phases == 1 ? "einphasig" : "dreiphasig"}</b> um.</>/*NF*/
        },
        "script": {
            "save_failed": "Speichern der Power Manager-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Power Manager-Einstellungen",
            "mode_change_failed": "Wechsel des Lademodus fehlgeschlagen.",

            "meter_slots_internal": "nur Wallbox",
            "meter_slots_no_values": "noch keine Werte"
        }
    }
}
