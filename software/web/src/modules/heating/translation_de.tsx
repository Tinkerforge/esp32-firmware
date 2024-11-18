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
            "extended_logging": "Erweitertes Logging",
            "extended_logging_activate": "Weiterführende Informationen zu den Steuerentscheidungen werden im Ereignis-Log angezeigt.",
            "extended_logging_description": "",
            "minimum_holding_time": "Mindesthaltezeit",
            "minimum_holding_time_muted": "für SG-Ready-Ausgang 1 und SG-Ready-Ausgang 2",
            "remaining_holding_time": "Resthaltezeit",
            "remaining_holding_time_muted": "",
            "minutes": "Minuten",
            "minute": "Minute",
            "dpc_low": "für günstigste",
            "dpc_high": "Blockieren für die teuersten",
            "dpc_extended_help": <>
                <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                <p>Wenn die dynamische Preissteuerung für niedrige Preise aktiviert ist, wird der Heizung eine Einschaltempfehlung gegeben sobald der Strompreis x% oder weniger vom Tagesdurchschnitt entspricht.</p>
                <p>Beispiel: Angenommen der Durchschnittsstrompreis für einen Tag liegt bei 10ct und die Schwelle ist auf 80% konfiguriert. In diesem Fall würde der Heizung über den Tag verteilt eine Einschaltempfehlung gegeben, in den Zeiträumen wo der Preis 8ct oder niedriger ist.</p>
            </>,
            "dpc_blocking_help": <>
                <p>Diese Einstellung steuert den blockierenden Betrieb (SG-Ready-Ausgang 1).</p>
                <p>Wenn die dynamische Preissteuerung für hohe Preise aktiviert ist, wird der Heizung in den blockierenden Betrieb gebracht sobald der Strompreis x% oder mehr vom Tagesdurchschnitt entspricht.</p>
                <p>Beispiel: Angenommen der Durchschnittsstrompreis für einen Tag liegt bei 10ct und die Schwelle ist auf 120% konfiguriert. In diesem Fall würde der Heizung über den Tag verteilt blockiert werden, in den Zeiträumen wo der Preis 12ct oder höher ist.</p>
                <p>Hinweis: Falls gleichzeitig die PV-Überschuss-Steuerung aktiviert ist und ein ausreichender PV-Überschuss vorhanden ist, wird diese Einstellung ingoriert.</p>
            </>,
            "pv_excess_control": "bei PV-Überschuss ab",
            "pv_excess_control_help": <>
                <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                <p>Wenn die PV-Überschuss-Steuerung aktiviert ist, wird der Heizung eine Einschaltempfehlung gegeben sobald der Überschuss am Netzanschluss den eingestellt Wert überschreitet.</p>
                <p>Beispiel: Verbraucht die Heizung im laufenden Betrieb ca. 1500W, dann dann sollte hier auch 1500W eingetragen werden. In dem Fall würde die Einschaltempfehlung automatisch gegeben, wenn ein entsprechender PV-Überschuss vorhanden ist.</p>
            </>,
            "pv_yield_forecast": "PV-Ertragsprognose unter",
            "pv_yield_forecast_help": <>
                <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                <p>Wenn die PV-Ertragsprognose aktiviert ist, kann der Heizung eine Einschaltempfehlung auch außerhalb der Aktivzeit gegeben werden, wenn die PV-Ertragsprognose (Summe des Ertrags von einem Tag in kWh) die eingetellte Schwelle unterschreitet.</p>
                <p>Beispiel: Es ist 05:00 morgens und die Aktivzeit ist von 06:00 bis 18:00. Die PV-Ertragsprognose ist aktiviert und die Schwelle ist auf 50kWh eingestellt, die Prognose sagt allerdings nur einen Ertrag von 10kWh voraus. In diesem Fall würde trotz der Uhrzeit außerhalb der Aktivzeit eine Einschaltempfehlung an die Heizung gesendet, wenn der Strom günstig ist.</p>
                <p>Die Idee ist es, Nachts den günstigen Strom zu nutzen wenn bekannt ist das am Tag die Sonne nicht scheinen wird.</p>
            </>,
            "p14_enwg_control_activate": "Aktiviert die Kontrolle der Heizung nach §14 EnWG",
            "input": "Eingang",
            "output": "Ausgang",
            "throttled_if_input": "Drosselung",
            "closed": "Aktiv wenn geschlossen",
            "opened": "Aktiv wenn geöffnet",
            "meter_slot_grid_power": "Stromzähler",
            "meter_slot_grid_power_muted": "typischerweise am Netzanschluss",
            "meter_slot_grid_power_select": "Auswählen...",
            "meter_slot_grid_power_none": "Kein Stromzähler konfiguriert",
            "sg_ready_output": "SG-Ready-Ausgang",
            "sg_ready_output1_muted": "Ausgang 1 wird für den blockierenden Betrieb verwendet (SG Ready Zustand 1).",
            "sg_ready_output1_help": "Dieser Betriebszustand ist abwärtskompatibel zur häufig zu festen Uhrzeiten geschalteten EVU-Sperre und umfasst maximal 2 Stunden harte Sperrzeit.",
            "sg_ready_output2_muted": "Ausgang 2 wird für den erweiterten Betrieb verwendet (SG Ready Zustand 3).",
            "sg_ready_output2_help": "In diesem Betriebszustand läuft die Wärmepumpe im erweiterten Betrieb für Raumheizung und Warmwasserbereitung. Es handelt sich dabei nicht um einen definitiven Anlaufbefehl, sondern um eine Einschaltempfehlung.",
            "control": "Steuerung",
            "active": "Aktiv",
            "inactive": "Inaktiv",
            "watt": "Watt",
            "price_based_heating_plan": "Preisbasierter Heizplan",
            "price_based_heating_plan_muted": "Heizplan anhand dynamischer Preise: Rot = blockierender Betrieb, Grün = erweiterter Betrieb",
            "status": "Status",
            "january": "Januar",
            "february": "Februar",
            "march": "März",
            "april": "April",
            "may": "Mai",
            "june": "Juni",
            "july": "Juli",
            "august": "August",
            "september": "September",
            "october": "Oktober",
            "november": "November",
            "december": "Dezember",
            "blocked": "Blockiert",
            "extended": "Erweitert",
            "sg_ready_muted": "Ausgang 1 und Ausgang 2",
            "average_price": "Durchschnittspreis",
            "solar_forecast": "PV-Ertragsprognose",
            "solar_forecast_needs_activation": <>
                <p style="color:red;">Für diese Konfiguration muss die <a href="#solar_forecast" onClick={() => {window.scrollTo(0, 0); return true;}}>PV-Ertragsprognose</a> aktiviert werden.</p>
            </>,
            "day_ahead_prices_needs_activation": <>
                <p style="color:red;">Für diese Konfiguration müssen die <a href="#day_ahead_prices" onClick={() => {window.scrollTo(0, 0); return true;}}>dynamischen Strompreise</a> aktiviert werden.</p>
            </>,
            "meter_needs_activation": <>
                <p style="color:red;">Für diese Konfiguration mus ein Stromzähler ausgewählt werden.</p>
            </>,
            "hours": "Stunden",
            "update_now": "Jetzt aktualisieren",
            "extended_operation": "Erweiterter Betrieb",
            "extended_operation_help": <>
                <p>Diese Einstellungen steuern den erweiterten Betrieb (SG-Ready-Ausgang 2).</p>
                <p>Wenn die PV-Überschuss-Steuerung aktiviert ist, wird die Heizung in den erweiterten Betrieb gebracht sobald der Überschuss am Netzanschluss den eingestellten Wert überschreitet.</p>
                <p>Wenn die Dynamische-Strompreis-Steuerung aktiviert ist, wird die Heizung zusätzlich für die günstigsten x Stunden am Tag in den erweiterten Betrieb gebracht.</p>
                <p>Wenn die PV-Ertragsprognosen-Steuerung aktiviert ist und die Prognose größer als die eingestellte Enerige ausfällt, entfällt die Dynamische-Strompreis-Steuerung und es wird ausschließlich PV-Überschuss genutzt.</p>
                <p>Beispiel-Konfiguration: Erweiterter Betrieb bei PV-Überschuss <b>ab 1200W</b> oder für <b>günstigste 4 Stunden pro Tag</b> aber nur wenn PV-Ertragsprognose <b>unter 10 kWh/Tag</b></p>.
                <p>In diesem Beispiel würde bei einem PV-Überschuss von 1200W, ungeachtet der folgenden Einstellungen, der erweiterte Betrieb aktiviert. Zusätzlich würde für die 4 günstigsten Stunden am Tag der erweiterte Betrieb aktiviert, aber nur falls die PV-Ertragsprognose weniger als 10 kWh für den Tag vorher gesagt hat. Falls ein höherer Ertrag erwartet wird, wird kein erweiterter Betrieb aktiviert.</p>
                <p>Ziel dieser Einstellungen: Wenn ein hoher PV-Ertrag erwartet wird keinen Strom aus dem Netz für den erweiterten Betrieb bezogen, trotz dass der Strompreis günstig ist.</p>
            </>,
            "blocking_operation": "Blockierender Betrieb",
            "blocking_operation_help": <>
                <p>Diese Einstellungen steuern den blockierenden Betrieb (SG-Ready-Ausgang 1).</p>
                <p>Bei Aktivierung wird die Heizung für die teuersten x Stunden am Tag in den blockierenden Betrieb gebracht.</p>
                <p>Beispiel-Konfiguration: Blockierender Betrieb bei den teuersten 5 Stunden. Die Heizung wird für die 5 teuersten Stunden am Tag blockiert. Ausnahme: Wenn die PV-Überschuss-Steuerung aktiviert ist und der eingestellte Schwellwert überschritten wird, wird der blockierende Betrieb aufgehoben und der erweiterte Betrieb für die Heizung aktiviert.</p>
            </>,
            "or": "oder",
            "but_only_if": "aber nur wenn",
            "kwh_per_day": "kWh/Tag",
            "h_per_day": "h/Tag"
        },
        "script": {
            "save_failed": "Speichern der Heizungseinstellungen fehlgeschlagen",
            "reboot_content_changed": "Heizungseinstellungen"
        }
    }
}
