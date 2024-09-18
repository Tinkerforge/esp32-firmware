/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "heating": {
        "status": {},
        "navbar": {
            "heating": "Heizung"
        },
        "content": {
            "heating": "Heizung",
            "control_signal": "Steuersignal",
            "control_signal_description": <>
                <div>
                    Als Steuersignal für die Heizung wird "SG Ready" verwendet.
                    Der SEB hat dafür zwei potentialfreie Schaltausgänge:
                </div>
                <div>
                    <ul>
                        <li>Ausgang 1 wird für den blockierenden Betrieb verwendet (SG Ready Zustand 1).</li>
                        <li>Ausgang 2 wird für die Einschaltempfehlung verwendet (SG Ready Zustand 3).</li>
                        <li>Wenn beide Ausgänge nicht geschaltet sind findet der Normalbetrieb statt (SG Ready Zustand 2).</li>
                    </ul>
                </div>
                <div>
                    Die intelligente Winter-/Sommermodussteuerung nutzt den Ausgang 2 und
                    die Steuerung nach §14 EnWG den Ausgang 1. Es ist möglich nur einen der beiden Ausgänge anzuschließen
                    wenn nur eine der beiden Steuerungen verwendet werden soll.
                </div>
            </>,
            "extended_logging": "Erweitertes Logging",
            "extended_logging_activate": "Aktiviert das erweiterte Logging",
            "extended_logging_description": "Mehr Informationen zu den Steuerentscheidungen im Ereignis-Log",
            "minimum_holding_time": "Mindesthaltezeit",
            "minimum_holding_time_description": "Wie lange soll ein Anlaufbefehl mindestens gelten",
            "minutes": "Minuten",
            "month": "Monat",
            "day": "Tag",
            "winter_mode": "Wintermodus",
            "winter_start": "Winter Start",
            "winter_end": "Winter Ende",
            "dynamic_price_control": "Dynamische Preissteuerung",
            "dynamic_price_control_activate": "Aktiviert die Optimierung der Heizungssteuerung anhand des dynamischen Strompreises",
            "average_price_threshold": "Durchschnitspreis-Schwelle",
            "average_price_threshold_description": "Liegt der Tagesdurchschnittspreis unter der Schwelle, dann wird der Heizung ein Anlaufbefehl gegeben",
            "pv_excess_control": "PV-Überschuss-Steuerung",
            "pv_excess_control_activate": "Aktiviert die Optimierung der Heizungssteuerung anhand des PV-Überschuss",
            "pv_excess_threshold": "PV-Überschuss-Schwelle",
            "pv_excess_threshold_description": "Ab wieviel Watt PV-Überschuss soll der Heizung ein Anlaufbefehl gegeben werden",
            "summer_mode": "Sommermodus",
            "summer_start": "Sommer Start",
            "summer_end": "Sommer Ende",
            "block_time": "Blockierzeit",
            "enable_daily_block_period": "Aktiviert den täglichen Blockierzeitraum",
            "from": "Von",
            "to": "Bis",
            "morning": "Morgens",
            "evening": "Abends",
            "pv_yield_forecast": "PV-Ertragsprognose",
            "pv_yield_forecast_activate": "Blockiere nur wenn erwarteter Ertrag über konfigurierter Blockier-Schwelle",
            "blocking_threshold": "Blockier-Schwelle",
            "blocking_threshold_description": "Ab wieviel kWh Ertragsprognose soll der Anlaufbefehl anhand Uhrzeit blockiert werden",
            "p14_enwg_control_activate": "Aktiviert die Kontrolle der Heizung anhand §14 EnWG",
            "input": "Eingang",
            "throttled_if_input": "Gedrosselt wenn Eingang",
            "closed": "Geschlossen",
            "opened": "Geöffnet",
            "meter_slot_grid_power": "Stromzähler",
            "meter_slot_grid_power_muted": "Typischerweise am Netzanschluss",
            "meter_slot_grid_power_select": "Auswählen...",
            "meter_slot_grid_power_none": "Kein Stromzähler konfiguriert"
        },
        "script": {
            "save_failed": "Speichern der Heizungseinstellungen fehlgeschlagen",
            "reboot_content_changed": "Heizungseinstellungen"
        }
    }
}
