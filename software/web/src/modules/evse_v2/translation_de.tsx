/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "evse": {
        "content": {
            "error_dc_fault_current": "DC-Fehlerstromschutz",
            "energy_meter_type": "Stromzählertyp",
            "adc_names": <>CP/PE vor Widerstand (PWM High), CP/PE nach Widerstand (PWM High)<br/>CP/PE vor Widerstand (PWM Low), CP/PE nach Widerstand (PWM Low)<br/>PP/PE, +12V Rail<br/>-12V Rail</>,
            "voltage_names": <>CP/PE vor Widerstand (PWM High), CP/PE nach Widerstand (PWM High)<br/>CP/PE vor Widerstand (PWM Low), CP/PE nach Widerstand (PWM Low)<br/>PP/PE, +12V Rail<br/>-12V Rail</>,
            "gpio_names_0": "Stromkonfiguration 0, Motorfehler, Gleichstromfehler, Stromkonfiguration 1",
            "gpio_names_1": "DC-Fehlerstromschutz-Test, Abschaltung, Taster, CP-PWM",
            "gpio_names_2": "Motoreingangsschalter, Schützsteuerung, Konfigurierbarer Ausgang, CP-Trennung",
            "gpio_names_3": "Motor aktiv, Motor-Phase, Schützprüfung vorher, Schützprüfung nachher",
            "gpio_names_4": "Konfigurierbarer Eingang, DC X6, DC X30, LED",
            "gpio_names_5": "nicht belegt",
            "gpio_shutdown": "Abschalteingang",
            "gpio_shutdown_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "gpio_shutdown_not_configured": "Nicht konfiguriert",
            "gpio_shutdown_on_open": "Abschalten wenn geöffnet",
            "gpio_shutdown_on_close": "Abschalten wenn geschlossen",
            "not_configured": "Nicht konfiguriert",
            "active_low_blocked": "Blockiert wenn geschlossen",
            "active_low_prefix": "Limitiert Ladestrom auf ",
            "active_low_suffix": " wenn geschlossen",
            "active_high_blocked": "Blockiert wenn geöffnet",
            "active_high_prefix": "Limitiert Ladestrom auf ",
            "active_high_suffix": " wenn geöffnet",

            "todo": "Ideen bzw. Wünsche? Schreib eine Mail an info@tinkerforge.com",
            "gpio_in": "Konfigurierbarer Eingang",
            "gpio_in_muted": "Kann als GPIO 16 gelesen werden",
            "gpio_out": "Konfigurierbarer Ausgang",
            "gpio_out_muted": <><a href="https://de.wikipedia.org/wiki/Open-Collector-Ausgang">Open-Collector-Ausgang</a></>,
            "gpio_out_high": "Hochohmig",
            "gpio_out_low": "Verbunden mit Masse",
            "button_configuration": "Tastereinstellung",
            "button_configuration_muted": "Aktion, die bei Druck des Tasters ausgeführt wird.",
            "button_configuration_deactivated": "Keine Aktion",
            "button_configuration_start_charging": "Ladestart",
            "button_configuration_stop_charging": "Ladestop",
            "button_configuration_start_and_stop_charging": "Ladestart bzw. Ladestop",
            "button_pressed": "Drücken des Tasters",
            "button_released": "Loslassen des Tasters",

            "ev_wakeup_desc": "Fahrzeug-Weckruf",
            "ev_wakeup_desc_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "ev_wakeup": "Versucht die Ladeelektronik des Fahrzeugs aus einem Energiesparmodus zu wecken, indem ein Abziehen und Anstecken des Ladekabels vorgetäuscht wird.",

            "dc_fault_current_state": "DC-Fehlerstromzustand",
            "dc_fault_current_state_desc": "",
            "dc_fault_current_ok": "OK",
            "dc_fault_current_6_ma": "Gleichstromfehler",
            "dc_fault_current_system": "Systemfehler",
            "dc_fault_current_unknown": "Unbekannter Fehler",
            "dc_fault_current_calibration": "Kalibrierungsfehler",
            "dc_fault_current_reset": "Zurücksetzen",

            "reset_dc_fault_title": "Zurücksetzen des DC-Fehlerstromschutzmoduls",
            "reset_dc_fault_content": <>Durch das Zurücksetzen des Moduls kann wieder geladen werden. <b>Es muss sichergestellt sein, dass der Grund für das Auslösen des Moduls behoben wurde!</b> <a href="{{{manual_url}}}">Siehe Betriebsanleitung für Details.</a> Soll das DC-Fehlerstromschutzmodul wirklich zurückgesetzt werden?</>,
            "abort": "Abbrechen",
            "reset": "Zurücksetzen",
            "trigger_dc_fault_test": "DC-Fehlerschutz testen",
            "time_since_dc_fault_check": "Zeit seit dem letzten DC-Fehlerschutztest"
        },
        "script": {
            "reset_dc_fault_current_failed": "Zurücksetzen des DC-Fehlerstromschutzmoduls fehlgeschlagen",
            "gpio_configuration_failed": "Speichern der GPIO-Konfiguration fehlgeschlagen",

            "meter_type_0": "Kein Stromzähler angeschlossen",
            "meter_type_1": "SDM72",
            "meter_type_2": "SDM630",
            "meter_type_3": "SDM72V2",
            "meter_type_254": "intern",

            "slot_4": "Manuelle Ladefreigabe oder Taster"
        }
    }
}
