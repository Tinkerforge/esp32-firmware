/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "evse": {
        "status": {
            "evse": "Fahrzeugstatus",
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
            "evse_settings": "Ladeeinstellungen"
        },
        "content": {
            "status": "Ladestatus",
            "settings": "Ladeeinstellungen",
            "iec_state": "IEC-61851-Zustand",
            "iec_state_a": "A (getrennt)",
            "iec_state_b": "B (verbunden)",
            "iec_state_c": "C (lädt)",
            "iec_state_d": "D (nicht unterstützt)",
            "iec_state_ef": "E/F (Fehler)",
            "contactor_state": "Schützprüfung",
            "contactor_names": "vor Schütz, nach Schütz, Zustand",
            "contactor_not_live": "Stromlos",
            "contactor_live": "Stromführend",
            "contactor_ok": "OK",
            "contactor_error": "Fehler",
            "allowed_charging_current": "Erlaubter Ladestrom",
            "error_state": "Fehlerzustand",
            "error_state_desc": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "error_ok": "OK",
            "error_switch": "Schalter",
            "error_contactor": "Schütz",
            "error_communication": "Kommunikation",
            "lock_state": "Kabelverriegelung",
            "lock_init": "Start",
            "lock_open": "Offen",
            "lock_closing": "Schließend",
            "lock_close": "Geschlossen",
            "lock_opening": "Öffnend",
            "lock_error": "Fehler",
            "time_since_state_change": "Zeit seit Zustandswechsel",
            "state_change": "Zustandswechsel",
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
            "led_state_blinking": "Bestätigendes Blinken",
            "led_state_flickering": "Ablehnendes Blinken",
            "led_state_breathing": "Auffordendes Blinken",
            "led_state_error": /*SFN*/(count: number) => {
                return "Blinken (" + count + "x)";
            }/*NF*/,
            "led_state_api": "API",
            "led_duration": "Dauer",
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
            "active_high": "Wenn geöffnet",
            "active_low": "Wenn geschlossen",
            "gpio_state": "Status",
            "gpio_out_high": "Hochohmig",
            "gpio_out_low": "Verbunden mit Masse",
            "gpio_out": "Konfigurierbarer Ausgang",
            "gpio_in": "Konfigurierbarer Eingang",
            "gpio_shutdown": "Abschalteingang",
            "button_pressed": "Drücken des Tasters",
            "button_released": "Loslassen des Tasters",
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

            "enable_led_api": "Status-LED Steuerung",
            "enable_led_api_desc": "Erlaubt die externe Steuerung der Status-LED.",
            "api_must_be_enabled": "Die API muss aktiviert sein, um die Status-LED steuern zu können.",
            "cron_state_change_trigger": /*FFN*/(state: string) => {
                return (
                  <>
                    Wenn der Ladecontroller in den Zustand "<b>{state}</b>" wechselt,{" "}
                  </>
                );
              }/*NF*/,
              "cron_action_text": /*FFN*/(current: number) => {
                return (
                  <>
                    setze den erlaubten Ladestrom auf <b>{current} A</b>.
                  </>
                );
              }/*NF*/,
              "cron_led_action_text": /*FFN*/(state: string, duration: number) => {
                if (state == "An" || state == "Aus") {
                  return (
                    <>
                      schalte die Status-LED für <b>{duration / 1000} Sekunden</b> <b>{state}</b>.
                    </>
                  );
                }
                return <>
                  zeige <b>{state}</b> für <b>{duration / 1000} Sekunden</b> auf der Status-LED.
                </>
              }/*NF*/,

            "slot_0": "Zuleitung",
            "slot_1": "Typ-2-Ladekabel",
            "slot_2": "Abschalteingang",
            "slot_3": "Konfigurierbarer Eingang",
            "slot_4": "Manuelle Ladefreigabe",
            "slot_5": "Konfiguration",
            "slot_6": "Benutzer",
            "slot_7": "Lastmanagement",
            "slot_8": "Externe Steuerung",
            "slot_9": "Modbus TCP-Strom",
            "slot_10": "Modbus TCP-Freigabe",
            "slot_11": "OCPP",
            "slot_12": "Energie-/Zeitlimit",
            "slot_13": "Zählerüberwachung",
            "slot_14": "Cron"
        },
        "script": {
            "error_code": "Fehlercode",
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

            "acc_blocked": "Blockiert",
            "by": "durch",

            "save_failed": "Speichern der Ladeeinstellungen fehlgeschlagen.",

            "reset_slot_failed": "Freigeben der Stromgrenze fehlgeschlagen",

            "slot_disabled": "Nicht aktiv",
            "slot_blocks": "Blockiert",
            "slot_no_limit": "Freigegeben",

            "slot_0": "Zuleitung",
            "slot_1": "Ladekabel",
            "slot_2": "Abschalteingang",
            "slot_3": "konfigurierbaren Eingang",
            "slot_5": "Konfiguration",
            "slot_6": "Benutzer",
            "slot_7": "Lastmanagement",
            "slot_8": "externe Steuerung",
            "slot_9": "Modbus TCP",
            "slot_10": "Modbus TCP-Freigabe",
            "slot_11": "OCPP",
            "slot_12": "Energie-/Zeitlimit",
            "slot_13": "Zählerüberwachung",

            "reboot_content_changed": "Ladeeinstellungen"
        }
    }
}
