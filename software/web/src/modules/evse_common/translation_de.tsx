/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "evse": {
        "status": {
            "evse": "Ladestatus",
            "not_connected": "Getrennt",
            "waiting_for_charge_release": "Warte auf Freigabe",
            "ready_to_charge": "Ladebereit",
            "charging": "Lädt",
            "error": "Fehler",
            "configured_charging_current": "Konfigurierter Ladestrom",
            "allowed_charging_current": "Erlaubter Ladestrom",
            "charge_control": "Ladesteuerung",
            "start_charging": "Start",
            "stop_charging": "Stop"
        },
        "navbar": {
            "evse": "Ladestatus",
            "evse_settings": "Einstellungen"
        },
        "content": {
            "status": "Ladestatus",
            "evse_settings": "Wallbox",
            "iec_state": "IEC-61851-Zustand",
            "iec_state_a": "A (getrennt)",
            "iec_state_b": "B (verbunden)",
            "iec_state_c": "C (lädt)",
            "iec_state_d": "D (nicht unterstützt)",
            "iec_state_ef": "E/F (Fehler)",
            "contactor_state": "Schützprüfung",
            "contactor_names": /*SFN*/(is_evse_v3: boolean) => is_evse_v3 ? "Schütz L1+N, Schütz L2+L3, Zustand" : "vor Schütz, nach Schütz, Zustand"/*NF*/,
            "contactor_not_live": /*SFN*/(is_evse_v3: boolean) => is_evse_v3 ? "Offen" : "Stromlos"/*NF*/,
            "contactor_live": /*SFN*/(is_evse_v3: boolean) => is_evse_v3 ? "Geschlossen" : "Stromführend"/*NF*/,
            "contactor_ok": "OK",
            "contactor_error": /*SFN*/(contactor_error: number) => {
                if (contactor_error == 0)
                    return "Fehler";
                return "Fehlercode " + contactor_error.toString();
                }/*NF*/,
            "allowed_charging_current": "Erlaubter Ladestrom",
            "error_state": "Fehlerzustand",
            "error_state_desc": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "error_ok": "OK",
            "error_switch": "Schalter",
            "error_contactor": /*SFN*/(pe_error: boolean, contactor_error: boolean) => (contactor_error == pe_error ? "Schütz/PE" : (pe_error ? "PE" : "Schütz"))/*NF*/,
            "error_communication": "Kommunikation",
            "lock_state": "Kabelverriegelung",
            "lock_init": "Start",
            "lock_open": "Offen",
            "lock_closing": "Schließend",
            "lock_close": "Geschlossen",
            "lock_opening": "Öffnend",
            "lock_error": "Fehler",
            "time_since_state_change": "Zeit seit Zustandswechsel",
            "uptime": "Laufzeit",
            "configuration": "Hardware-Konfiguration",
            "has_lock_switch": "Kabelverriegelung vorhanden",
            "lock_no": "Nein",
            "lock_yes": "Ja",
            "jumper_config_max_current": "Maximalstrom der Zuleitung",
            "jumper_config": "durch Schalter konfiguriert",
            "jumper_config_software": "Software",
            "jumper_config_unconfigured": "Unkonfiguriert",
            "evse_version": "Hardware-Version des Ladecontrollers",
            "evse_fw_version": "Firmware-Version des Ladecontrollers",
            "charging_current": "Ladestromgrenzen",
            "charging_current_muted": "Minimum der Ladestromgrenzen",
            "reset_slot": "Zurücksetzen",
            "low_level_state": "Low-Level-Zustand",
            "led_state": "LED-Zustand",
            "led_state_off": "Aus",
            "led_state_on": "An",
            "led_state_blinking": "Blinkend",
            "led_state_flickering": "Flackernd",
            "led_state_breathing": "Atmend",
            "led_state_api": "API",
            "cp_pwm_dc": "CP-PWM-Tastverhältnis",
            "adc_values": "ADC-Werte",
            "voltages": "Spannungen",
            "resistances": "Widerstände",
            "resistance_names": "CP/PE, PP/PE",
            "gpios": "GPIOs",
            "gpio_low": "Low",
            "gpio_high": "High",
            "debug": "Ladeprotokoll",
            "debug_start": "Start",
            "debug_stop": "Stop+Download",
            "debug_description": "Ladeprotokoll erstellen",
            "debug_description_muted": "zur Diagnose bei Ladeproblemen",
            "gpio_out_high": "Hochohmig",
            "gpio_out_low": "Verbunden mit Masse",
            "gpio_out": "Konfigurierbarer Ausgang",
            "gpio_in": "Konfigurierbarer Eingang",
            "gpio_shutdown": "Abschalteingang",
            "button_configuration": "Tastereinstellung",

            "auto_start_description": "Manuelle Ladefreigabe",
            "auto_start_description_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "auto_start_enable": "Erzwingt dass Ladevorgänge immer über das Webinterface, die API oder (je nach Tastereinstellung) den Taster manuell gestartet werden müssen.",

            "external_description": "Externe Steuerung",
            "external_description_muted": <><a href="https://www.warp-charger.com/evcc.html?v=2">siehe Einrichtungs-Tutorial für Details</a></>,
            "external_enable": <>Erlaubt einer externen Steuerung (z.B. <a href="https://evcc.io/">EVCC</a>) diese Wallbox zu steuern</>,

            "boost_mode_desc": "Boost-Modus",
            "boost_mode_desc_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "boost_mode": "Gibt der Ladeelektronik des Fahrzeugs einen leicht höheren Ladestrom vor (+ 0,24 A) um Messfehler der Ladeelektronik zu kompensieren. Nur Verwenden, falls ein Fahrzeug mit einem kleineren als dem erlaubten Ladestrom lädt!",

            "reset_description": "Aktionen",
            "reset_description_muted": "",
            "reset_evse": "Neu starten",
            "reflash_evse": "Neu flashen",
            "charging_time": "Lädt seit",

            "meter_monitoring": "Zählerüberwachung",
            "meter_monitoring_desc": "Überwacht den Stromzähler und blockiert Ladevorgänge im Falle eines Defekts.",

            "enable_led_api": "Status-LED-Steuerung",
            "enable_led_api_desc": "Erlaubt die externe Steuerung der Status-LED.",

            "slot": /*SFN*/(i: number) => { return {
                0: "Zuleitung",
                1: "Typ-2-Ladekabel",
                2: "Abschalteingang",
                3: "Konfigurierbarer Eingang",
                4: "Manuelle Ladefreigabe",
                5: "Konfiguration",
                6: "Benutzer/NFC",
                7: "Lastmanagement",
                8: "Externe Steuerung",
                9: "Modbus TCP-Strom",
                10: "Modbus TCP-Freigabe",
                11: "OCPP",
                12: "Energie-/Zeitlimit",
                13: "Zählerüberwachung",
                14: "Automatisierung"
            }[i];}/*NF*/,

            // EVSE V1 only
            "gpio_names": "Eingang, Ausgang, Motoreingangsschalter, Relais, Motorfehler",
            "user_calibration": "Kalibrierungsstatus",
            "user_calibration_state_disabled": "Werkseinstellungen",
            "user_calibration_state_enabled": "Modifiziert",
            "voltage_calibration": "Spannungskalibrierung",
            "voltage_calibration_names": "Diff, Mul, Div",
            "resistance_2700": "Widerstandskalibrierung 2700Ω",
            "resistance_880": "Widerstandskalibrierung 880Ω",
            "user_calibration_description": "Kalibrierung",
            "user_calibration_description_muted": "",
            "user_calibration_download": "Herunterladen",
            "user_calibration_upload": "Hochladen",
            "user_calibration_browse": "Durchsuchen",
            "user_calibration_select_file": "Kalibrierungsdatei auswählen...",
            "user_calibration_reset": "Zurücksetzen",

            // EVSE V2 only
            "evse_v2_gpio_names_0": "Stromkonfiguration 0, Motorfehler, DC-Fehler, Stromkonfiguration 1",
            "evse_v2_gpio_names_1": "DC-Fehlerstromschutz-Test, Abschalteingang, Taster, CP-PWM",
            "evse_v2_gpio_names_2": "Motoreingangsschalter, Schützsteuerung, Konfigurierbarer Ausgang, CP-Trennung",
            "evse_v2_gpio_names_3": "Motor aktiv, Motor-Phase, Schützprüfung vorher, Schützprüfung nachher",
            "evse_v2_gpio_names_4": "Konfigurierbarer Eingang, DC X6, DC X30, LED",
            "evse_v2_gpio_names_5": "nicht belegt",

            // EVSE V3 only
            "evse_v3_gpio_names_0": "DC X30, DC X6, DC-Fehler, DC-Fehlerstromschutz-Test",
            "evse_v3_gpio_names_1": "EVSE-Status-LED, Taster, LED rot, LED blau",
            "evse_v3_gpio_names_2": "LED grün, CP-PWM, Schütz 1, Schütz 0",
            "evse_v3_gpio_names_3": "Schütz 1 Feedback, Schütz 0 Feedback, PE-Prüfung, Stromkonfiguration 1",
            "evse_v3_gpio_names_4": "CP-Trennung, Stromkonfiguration 0, Abschalteingang, Versionsdetektion",
            "evse_v3_gpio_names_5": "nicht belegt",

            // EVSE V2 and V3
            "energy_meter_type": "Stromzählertyp",
            "gpio_shutdown_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "gpio_shutdown_not_configured": "Nicht konfiguriert",
            "gpio_shutdown_on_open": "Abschalten wenn geöffnet",
            "gpio_shutdown_on_close": "Abschalten wenn geschlossen",
            "gpio_4200w_on_open": "Begrenzen auf 4200 W wenn geöffnet (§14 EnWG)",
            "gpio_4200w_on_close": "Begrenzen auf 4200 W wenn geschlossen (§14 EnWG)",
            "not_configured": "Nicht konfiguriert",
            "active_low_blocked": "Blockiert wenn geschlossen",
            "active_low_prefix": "Limitiert Ladestrom auf ",
            "active_low_suffix": " wenn geschlossen",
            "active_high_blocked": "Blockiert wenn geöffnet",
            "active_high_prefix": "Limitiert Ladestrom auf ",
            "active_high_suffix": " wenn geöffnet",

            "todo": "Ideen bzw. Wünsche? Schreib eine Mail an info@tinkerforge.com",
            "gpio_in_muted": "kann als GPIO 16 gelesen werden",
            "gpio_out_muted": <><a href="https://de.wikipedia.org/wiki/Open-Collector-Ausgang">Open-Collector-Ausgang</a></>,
            "button_configuration_muted": "Aktion, die bei Druck des Tasters ausgeführt wird.",
            "button_configuration_deactivated": "Keine Aktion",
            "button_configuration_start_charging": "Ladestart",
            "button_configuration_stop_charging": "Ladestop",
            "button_configuration_start_and_stop_charging": "Ladestart bzw. Ladestop",

            "ev_wakeup_desc": "Fahrzeug-Weckruf",
            "ev_wakeup_desc_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "ev_wakeup": "Versucht die Ladeelektronik des Fahrzeugs aus einem Energiesparmodus zu wecken, indem ein Abziehen und Anstecken des Ladekabels vorgetäuscht wird.",

            "phase_auto_switch_desc": "Automatischer Phasenwechsel",
            "phase_auto_switch_desc_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "phase_auto_switch": "Schaltet automatisch auf einphasiges Laden um, wenn das angeschlossene Fahrzeug nur auf L1 Strom bezieht.",

            "dc_fault_current_state": "DC-Fehlerstromzustand",
            "dc_fault_current_state_desc": "",
            "dc_fault_current_ok": "OK",
            "dc_fault_current_6_ma": "DC-Fehler",
            "dc_fault_current_system": "Systemfehler",
            "dc_fault_current_unknown": "Unbekannter Fehler",
            "dc_fault_current_calibration": /*SFN*/ (dc_fault_state: number, dc_fault_pins: number) => "Kalibrierungsfehler" + (dc_fault_state == 4 ? (": " + dc_fault_pins.toString()) : "")/*NF*/,
            "dc_fault_current_reset": "Zurücksetzen",
            "dc_fault_current_20_ma": "AC-Fehler",
            "dc_fault_current_6_ma_20_ma": "AC- und DC-Fehler",

            "reset_dc_fault_title": "Zurücksetzen des DC-Fehlerstromschutzmoduls",
            "reset_dc_fault_content": <>Durch das Zurücksetzen des Moduls kann wieder geladen werden. <b>Es muss sichergestellt sein, dass der Grund für das Auslösen des Moduls behoben wurde!</b> <a href="{{{manual_url}}}">Siehe Betriebsanleitung für Details.</a> Soll das DC-Fehlerstromschutzmodul wirklich zurückgesetzt werden?</>,
            "abort": "Abbrechen",
            "reset": "Zurücksetzen",
            "trigger_dc_fault_test": "DC-Fehlerschutz testen",
            "time_since_dc_fault_check": "Zeit seit dem letzten DC-Fehlerschutztest",

            // EVSE version specific value for common placeholder
            "error_2": /*SFN*/(is_evse_v2: boolean) => is_evse_v2 ? "DC-Fehlerstromschutz" : "Kalibrierung"  /*NF*/,
            "adc_names": /*FFN*/(is_evse_v2: boolean) => is_evse_v2 ? <>CP/PE vor Widerstand (PWM High), CP/PE nach Widerstand (PWM High)<br/>CP/PE vor Widerstand (PWM Low), CP/PE nach Widerstand (PWM Low)<br/>PP/PE, +12V Rail<br/>-12V Rail</> : <>CP/PE, PP/PE</> /*NF*/,
            "voltage_names": /*FFN*/(is_evse_v2: boolean) => is_evse_v2 ? <>CP/PE vor Widerstand (PWM High), CP/PE nach Widerstand (PWM High)<br/>CP/PE vor Widerstand (PWM Low), CP/PE nach Widerstand (PWM Low)<br/>PP/PE, +12V Rail<br/>-12V Rail</> : <>CP/PE, PP/PE,<br/> CP/PE (high)</> /*NF*/,
            "dc_fault_sensor_type": "Version des DC-Fehlerschutzsensors",
            "dc_fault_pins": "Pins des DC-Fehlerschutzsensors",
            "temperature": "Temperatur",
            "phases_current": "Phasen geschaltet",
            "phases_requested": "Phasen angefordert",
            "phases_state": "Zustand der Phasenumschaltung",

            "phases_connected": "Zuleitung",
            "phases_connected_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "phases_connected_1": "Einphasig",
            "phases_connected_3": "Dreiphasig"

        },
        "automation" : {
            "external_current_wd": "Watchdog der externen Steuerung ausgelöst",
            "external_current_wd_trigger": <>Wenn der <b>Watchdog</b> der <b>externen</b> Steuerung auslöst, </>,
            "state_change": "Ladestatus gewechselt",
            "led_duration": "Dauer",
            "indication": "Blinkmuster",
            "color": "Farbe",
            "led_indication": "Zeige auf Status-LED an",
            "led_indication_off": "Aus",
            "led_indication_on": "An",
            "led_indication_blinking": "Bestätigendes Blinken",
            "led_indication_flickering": "Ablehnendes Blinken",
            "led_indication_breathing": "Auffordendes Blinken",
            "led_indication_error": /*SFN*/(count: number) => {
                return "Blinken (" + count + "x)";
            }/*NF*/,
            "from": "Von",
            "to": "Zu",
            "any": "Beliebiger Status",
            "automation_action_block": "Laden blockieren",
            "automation_action_allow": "Laden freigeben",
            "automation_action_limit_current": "Ladestrom begrenzen",
            "allowed_charging_current": "Erlaubter Ladestrom",
            "action_allowed_charging_current": "Steuere Ladevorgang",
            "automation_state_change_trigger": /*FFN*/(old_state: string, new_state: string) => <>Wenn der Ladestatus von "<b>{old_state}</b>" auf "<b>{new_state}</b>" wechselt,{" "}</>/*NF*/,
            "automation_action_text": /*FFN*/(current: string) => {
                if (current === "0") {
                    return <><b>blockiere</b> das Laden.</>
                } else if (current === "32") {
                    return <><b>erlaube</b> das Laden.</>
                }
                return <>limitiere den erlaubten Ladestrom auf <b>{current} A</b>.</>
            }/*NF*/,
            "automation_led_action_text": /*FFN*/(indication_number: number, indication_text: string, duration: number, color: string) => {
                let c = color == "" ? "" : <>in <span class="px-2 mr-1" style={"background-color: " + color + "; border: 1px solid black;"}></span></>;
                if (indication_number == 0)
                    return <>schalte die Status-LED für <b>{duration / 1000} Sekunden</b> <b>{indication_text}</b>.</>;
                if (indication_number == 255)
                    return <>schalte die Status-LED für <b>{duration / 1000} Sekunden</b> {c} <b>{indication_text}</b>.</>;
                return <>zeige <b>{indication_text}</b> {c} für <b>{duration / 1000} Sekunden</b> auf der Status-LED.</>;
            }/*NF*/
        },
        "script": {
            "set_charging_current_failed": "Konnte Ladestrom nicht setzen",
            "start_charging_failed": "Ladestart auslösen fehlgeschlagen",
            "stop_charging_failed": "Ladestop auslösen fehlgeschlagen",
            "tab_close_warning": "Die Aufzeichnung des Ladecontroller-Logs wird abgebrochen, wenn der Tab geschlossen wird.",
            "loading_debug_report": "Lade Debug-Report",
            "loading_debug_report_failed": "Laden des Debug-Reports fehlgeschlagen",
            "loading_event_log": "Lade Event-Log",
            "loading_event_log_failed": "Laden des Event-Logs fehlgeschlagen",
            "starting_debug": "Aktiviere Aufzeichnung des Ladecontroller-Logs.",
            "starting_debug_failed": "Aktivierung der Aufzeichnung des Ladecontroller-Logs fehlgeschlagen.",
            "debug_running": "Aufzeichnung läuft. Tab nicht schließen!",
            "debug_stop_failed": "Stoppen der Aufzeichnung des Ladecontroller-Logs fehlgeschlagen.",
            "debug_stopped": "Aufzeichnung des Ladecontroller-Logs gestoppt.",
            "debug_done": "Abgeschlossen.",
            "debug_file": "EVSE-Ladeprotokoll",

            "acc_blocked": "Blockiert",
            "by": "durch",

            "save_failed": "Speichern der Ladeeinstellungen fehlgeschlagen.",

            "reset_slot_failed": "Freigeben der Stromgrenze fehlgeschlagen",

            "slot_disabled": "Nicht aktiv",
            "slot_blocks": "Blockiert",
            "slot_no_limit": "Freigegeben",

            "slot": /*SFN*/(i: number, is_evse_v2: boolean) => {return {
                0: "Zuleitung",
                1: "Ladekabel",
                2: "Abschalteingang",
                3: "konfigurierbaren Eingang",
                4: is_evse_v2 ? "Manuelle Ladefreigabe oder Taster" : "Manuelle Ladefreigabe, Taster oder Schlüsselschalter",
                5: "Konfiguration",
                6: "Benutzer/NFC",
                7: "Lastmanagement",
                8: "externe Steuerung",
                9: "Modbus TCP",
                10: "Modbus TCP-Freigabe",
                11: "OCPP",
                12: "Energie-/Zeitlimit",
                13: "Zählerüberwachung",
                14: "Automation"
            }[i];}/*NF*/,

            "reboot_content_changed": "Ladeeinstellungen",

            // EVSE V1 only
            "user_calibration_upload_failed": "Aktualisieren der Kalibrierung fehlgeschlagen",

            // EVSE V2 only
            "reset_dc_fault_current_failed": "Zurücksetzen des DC-Fehlerstromschutzmoduls fehlgeschlagen",
            "gpio_configuration_failed": "Speichern der GPIO-Konfiguration fehlgeschlagen",

            "meter_type_0": "Kein Stromzähler angeschlossen",
            "meter_type_1": "Eastron SDM72",
            "meter_type_2": "Eastron SDM630",
            "meter_type_3": "Eastron SDM72V2",
            "meter_type_6": "Eltako DSZ15DZMOD",
            "meter_type_7": "YTL DEM4A",
            "meter_type_254": "intern"
        }
    }
}
