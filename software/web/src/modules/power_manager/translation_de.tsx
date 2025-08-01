/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
import { toLocaleFixed } from "../../ts/util";
import { __, removeUnicodeHacks } from "../../ts/translation";
let x = {
    "power_manager": {
        "status": {
            "phase_switching": "Phasenumschaltung",
            "single_phase": "Einphasig",
            "three_phase":"Dreiphasig",
            "config_error_phase_switching": "Phasenumschaltung oder Schütz nicht konfiguriert",
            "config_error_no_max_current": "Maximaler Gesamtstrom der Wallboxen nicht konfiguriert",
            "config_error_no_chargers": "Keine Wallboxen konfiguriert",
            "config_error_excess_no_meter": "Überschussladen aktiviert aber kein Stromzähler eingerichtet oder Stromzähler liefert keine Gesamtleistung",
            "config_error_dlm_no_meter": "Dynamisches Lastmanagement aktiviert aber kein Stromzähler eingerichtet oder Stromzähler liefert keine gerichteten Phasenströme"
        },
        "navbar": {
            "pv_excess_settings": "PV-Überschussladen"
        },
        "content": {
            "em_proxy_warning": "Wenn der Energy Manager von einem anderen Lastmanager gesteuert wird, muss PV-Überschussladen auf dem Lastmanager aktiviert werden und nicht hier.",
            "cm_requirements_warning": <>Um das PV-Überschussladen zu nutzen, muss die <a href="#charge_manager_chargers">{__("charge_manager.content.enable_charge_manager")}</a> auf „{__("charge_manager.content.mode_manager")}“ konfiguriert sein.</>,

            "phase_switching_mode": "Phasenumschaltung",
            "automatic": "Automatisch",
            "external_control": "Externe Steuerung (EVCC)",
            "external_control_unavailable": "Externe Steuerung (EVCC) – keine kontrollierte Wallbox für Phasenumschaltung",
            "external_control_notification": "Externe Steuerung für die Phasenumschaltung ist aktiv. Dadurch werden alle Einstellungen im Zusammenhang mit PV-Überschussladen deaktiviert.",

            "header_excess_charging": "Photovoltaik-Überschussladen",
            "enable_excess_charging": "Überschussladen aktiviert",
            "enable_excess_charging_help": /*FFN*/() => options.WARP_DOC_BASE_URL.length > 0 ? <p>Das <a href={removeUnicodeHacks(`${options.WARP_DOC_BASE_URL}/docs/tutorials/pv_excess_charging`)}>Tutorial PV-Überschussladen</a> hilft bei der Einrichtung.</p> : undefined/*NF*/,
            "enable_excess_charging_desc": "Regelt die kontrollierten Wallboxen abhängig vom gemessenen Überschuss einer Photovoltaikanlage und vom Lademodus.",
            "default_mode": "Standardlademodus",
            "default_mode_muted": "wird nach Neustart verwendet",
            "meter_slot_grid_power": "Stromzähler",
            "meter_slot_grid_power_muted": "typischerweise am Netzanschluss",
            "meter_slot_grid_power_none": "Kein Stromzähler konfiguriert",
            "meter_slot_grid_power_missing_value": "Summe der gerichteten Wirkleistung fehlt",
            "meter_slot_grid_power_in_use_by_battery": "als Batteriespeicher-Stromzähler ausgewählt",
            "guaranteed_power": "Min + PV: Mindestladeleistung",
            "guaranteed_power_muted": "Leistung, die im Min + PV-Modus zum Laden von Fahrzeugen aus dem Netz bezogen werden darf.",
            "control_behavior": "Regelverhalten",
            "control_behavior_help": <><p>Legt fest, auf welchen Netzbezug geregelt werden soll, damit ein Batteriespeicher höher oder niedriger priorisiert wird als ein Fahrzeugladevorgang.</p></>,
            "target_power_n200": "Sehr konservativ (− 200 W)",
            "target_power_n100": "Konservativ (− 100 W)",
            "target_power_n50_with_battery_meter": "Leicht konservativ (− 50 W)",
            "target_power_n50_without_battery_meter": "Leicht konservativ (− 50 W) – lädt Batteriespeicher zuerst",
            "target_power_0_with_battery_meter": "Ausgeglichen (± 0 W)",
            "target_power_0_without_battery_meter": "Ausgeglichen (± 0 W) – empfohlen ohne Batteriespeicher",
            "target_power_p50_with_battery_meter": "Leicht aggressiv (+ 50 W)",
            "target_power_p50_without_battery_meter": "Leicht aggressiv (+ 50 W) – verwendet Batteriespeicher für Fahrzeugladevorgänge",
            "target_power_p100": "Aggressiv (+ 100 W)",
            "target_power_p200": "Sehr aggressiv (+ 200 W)",
            "cloud_filter": "Wolkenfilter",
            "cloud_filter_muted": "reduziert den Einfluss von wechselnd bewölktem Wetter",
            "cloud_filter_off": "Aus",
            "cloud_filter_weak": "Schwach",
            "cloud_filter_medium": "Mittel",
            "cloud_filter_strong": "Stark",

            "header_battery_storage": "Batteriespeicher",
            "meter_slot_battery_power": "Stromzähler",
            "meter_slot_battery_power_muted": "des Batteriespeichers",
            "meter_slot_battery_power_none": "Kein Speicher",
            "meter_slot_battery_power_missing_value": "Summe der gerichteten Wirkleistung oder gerichtete DC-Leistung fehlen",
            "meter_slot_battery_power_in_use_by_grid": "als Netzbezugs-Stromzähler ausgewählt",
            "battery_mode": "Speicherpriorität",
            "battery_mode_prefer_chargers": "Wallboxen bevorzugen, überschüssige Leistung für Speicher",
            "battery_mode_prefer_battery": "Speicher bevorzugen, überschüssige Leistung für Wallboxen",
            "battery_mode_target_soc": "Speicher bevorzugen, solange der Ladestand unter dem Zielwert liegt",
            "battery_target_soc": "Speicher-Ziel-Ladestand",
            "battery_target_soc_muted": "gewünschter Mindest-Ladestand des Speichers",
            "battery_target_soc_help": "Der Speicher wird bevorzugt geladen, solange dessen Ladestand unter dem Ziel-Ladestand liegt. Sobald der Ziel-Ladestand erreicht wurde, werden Wallboxen bevorzugt.",
            "battery_deadzone": "Bezugs- und Einspeise-Toleranz",
            "battery_deadzone_muted": "am Netzanschluss, während der Speicher aktiv ist",
            "battery_deadzone_help": "Die meisten Batteriespeicher erlauben einen gewissen Bezug und Einspeisung beim Entladen bzw. Laden. Die Toleranz sollte auf das 1,5-fache des erwarteten Bezugs und Einspeisung gestellt werden. Lässt der Speicher einen Bezug von 50 W und eine Einspeisung von -50 W zu, sollte die Toleranz auf 75 W gestellt werden."
        },
        "automation": {
            "slot": "Blockierungsslot",
            "block_charge": "Ladevorgänge blockieren",
            "unblock_charge": "Ladevorgänge freigeben",
            "block_mode": "Modus",
            "automation_block_charge_action_text": /*FFN*/(slot: number, block: boolean) => {
                if (block) {
                    return <><b>blockiere</b> das Laden durch Slot <b>{toLocaleFixed(slot, 0)}</b>.</>
                }
                return <><b>gebe</b> das Laden durch Slot <b>{toLocaleFixed(slot, 0)} frei</b>.</>
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

            "set_phases": "Schalte Phasen um",
            "phases_wanted": "Umschaltung auf",
            "single_phase": "auf einphasig",
            "three_phase": "auf dreiphasig",
            "automation_action_text": /*FFN*/(phases: number) => <>schalte auf <b>{phases == 1 ? "einphasig" : "dreiphasig"}</b> um.</>/*NF*/
        },
        "script": {
            "save_failed": "Speichern der Power Manager-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Power Manager-Einstellungen",
            "phase_change_failed": "Phasenumschaltung fehlgeschlagen.",

            "meter_slots_internal": "nur Wallbox",
            "meter_slots_no_values": "noch keine Werte"
        }
    }
}
