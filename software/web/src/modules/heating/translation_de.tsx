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
            "extended_logging_activate": "Weiterführende Informationen zu den Steuerentscheidungen werden im Ereignis-Log angezeigt.",
            "extended_logging_description": "",
            "minimum_holding_time": "Mindesthaltezeit",
            "minimum_holding_time_description": "Wie lange soll ein Anlaufbefehl mindestens gelten",
            "minutes": "Minuten",
            "month": "Monat",
            "day": "Tag",
            "winter_mode": "Wintermodus",
            "winter_start": "Winter Start",
            "winter_end": "Winter Ende",
            "dpc_low": "Einschalten bei niedrigem Strompreis",
            "dpc_high": "Blockieren bei hohem Strompreis",
            "dpc_extended_activate": "Einschaltempfehlung für die Heizung bei unterschreiten der Preisschwelle",
            "dpc_extended_help": <>
                <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                <p>Wenn die dynamische Preissteuerung für niedrige Preise aktiviert ist, wird der Heizung eine Einschaltempfehlung gegeben sobald der Strompreis x% oder weniger vom Tagesdurchschnitt entspricht.</p>
                <p>Beispiel: Angenommen der Durchschnittsstrompreis für einen Tag liegt bei 10ct und die Schwelle ist auf 80% konfiguriert. In diesem Fall würde der Heizung über den Tag verteilt eine Einschaltempfehlung gegeben, in den Zeiträumen wo der Preis 8ct oder niedriger ist.</p>
            </>,
            "dpc_blocking_activate": "Blockiert die Heizung bei überschreiten der Preisschwelle",
            "dpc_blocking_help": <>
                <p>Diese Einstellung steuert den blockierenden Betrieb (SG-Ready-Ausgang 1).</p>
                <p>Wenn die dynamische Preissteuerung für hohe Preise aktiviert ist, wird der Heizung in den blockierenden Betrieb gebracht sobald der Strompreis x% oder mehr vom Tagesdurchschnitt entspricht.</p>
                <p>Beispiel: Angenommen der Durchschnittsstrompreis für einen Tag liegt bei 10ct und die Schwelle ist auf 120% konfiguriert. In diesem Fall würde der Heizung über den Tag verteilt blockiert werden, in den Zeiträumen wo der Preis 12ct oder höher ist.</p>
                <p>Hinweis: Falls gleichzeitig die PV-Überschuss-Steuerung aktiviert ist und ein ausreichender PV-Überschuss vorhanden ist, wird diese Einstellung ingoriert.</p>
            </>,
            "average_price_threshold": "Preisschwelle",
            "average_price_threshold_extended_description": "Liegt der Tagesdurchschnitt unter der Schwelle, wird SG Ready zur Sollwerterhöhung geschaltet",
            "average_price_threshold_blocking_description": "Liegt der Tagesdurchschnitt über der Schwelle, wird SG Ready zur Blockierung geschaltet",
            "pv_excess_control": "Einschalten bei PV-Überschuss",
            "pv_excess_control_activate": "Aktiviert die Optimierung der Heizungssteuerung anhand des PV-Überschuss",
            "pv_excess_control_help": <>
                <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                <p>Wenn die PV-Überschuss-Steuerung aktiviert ist, wird der Heizung eine Einschaltempfehlung gegeben sobald der Überschuss am Netzanschluss den eingestellt Wert überschreitet.</p>
                <p>Beispiel: Verbraucht die Leistung im laufenden Betrieb ca. 1500W, dann dann sollte hier auch 1500W eingetragen werden. In dem Fall würde die Einschaltempfehlung automatisch gegeben, wenn ein entsprechender PV-Überschuss vorhanden ist.</p>
            </>,
            "pv_excess_threshold": "PV-Überschuss-Schwelle",
            "pv_excess_threshold_description": "Ab wieviel Watt PV-Überschuss soll der Heizung ein Anlaufbefehl gegeben werden",
            "summer_mode": "Sommermodus",
            "summer_start": "Sommeranfang",
            "summer_end": "Sommerende",
            "block_time": "Blockierzeit",
            "active_time_help": <>
                <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                <p>Wenn die aktuelle Uhrzeit außerhalb der Aktivzeit liegt, wird keine Einschaltempfehlung gegeben. Somit kann in der Sommerzeit verhindert werden dass morgens teurer Strom eingekauft wird, wenn Tagsüber die PV-Anlage Strom liefert.</p>
                <p>Hinweis: Die Aktivzeit kann von der PV-Ertragsprognose überschrieben werden (siehe unten).</p>
            </>,
            "enable_daily_block_period": "Täglicher Blockierzeitraum aktiviert",
            "from": "Von",
            "to": "Bis",
            "morning": "Blockierzeit morgens",
            "evening": "Blockierzeit abends",
            "pv_yield_forecast": "PV-Ertragsprognose",
            "pv_yield_forecast_activate": "Blockiere nur wenn erwarteter Ertrag über konfigurierter Blockier-Schwelle",
            "pv_yield_forecast_help": <>
                <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                <p>Wenn die PV-Ertragsprognose aktiviert ist, kann der Heizung eine Einschaltempfehlung auch außerhalb der Aktivzeit gegeben werden, wenn die PV-Ertragsprognose (Summe des Ertrags von einem Tag in kWh) die eingetellte Schwelle unterschreitet.</p>
                <p>Beispiel: Es ist 05:00 morgens und die Aktivzeit ist von 06:00 bis 18:00. Die PV-Ertragsprognose ist aktiviert und die Schwelle ist auf 50kWh eingestellt, die Prognose sagt allerdings nur einen Ertrag von 10kWh voraus. In diesem Fall würde trotz der Uhrzeit außerhalb der Aktivzeit eine Einschaltempfehlung an die Heizung gesendet, wenn der Strom günstig ist.</p>
                <p>Die Idee ist es, Nachts den günstigen Strom zu nutzen wenn bekannt ist das am Tag die Sonne nicht scheinen wird.</p>
            </>,
            "blocking_threshold": "Blockier-Schwelle",
            "blocking_threshold_description": "Ab wieviel kWh Ertragsprognose soll der Anlaufbefehl anhand Uhrzeit blockiert werden",
            "p14_enwg_control_activate": "Aktiviert die Kontrolle der Heizung nach §14 EnWG",
            "input": "Eingang",
            "throttled_if_input": "Drosselung",
            "closed": "Aktiv wenn geschlossen",
            "opened": "Aktiv wenn geöffnet",
            "meter_slot_grid_power": "Stromzähler",
            "meter_slot_grid_power_muted": "typischerweise am Netzanschluss",
            "meter_slot_grid_power_select": "Auswählen...",
            "meter_slot_grid_power_none": "Kein Stromzähler konfiguriert"
        },
        "script": {
            "save_failed": "Speichern der Heizungseinstellungen fehlgeschlagen",
            "reboot_content_changed": "Heizungseinstellungen"
        }
    }
}
