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
            "pv_excess_control": "bei PV-Überschuss ab",
            "pv_yield_forecast": "PV-Ertragsprognose unter",
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
            "sg_ready_output1_help": "Im blockierenden Betrieb läuft die Heizung nicht. Dieser Betriebszustand ist abwärtskompatibel zur häufig zu festen Uhrzeiten geschalteten EVU-Sperre.",
            "sg_ready_output2_muted": "Ausgang 2 wird für den erweiterten Betrieb verwendet (SG Ready Zustand 3).",
            "sg_ready_output2_help": "In diesem Betriebszustand läuft die Wärmepumpe im erweiterten Betrieb für Raumheizung und Warmwasserbereitung. Es handelt sich dabei nicht um einen definitiven Anlaufbefehl, sondern um eine Einschaltempfehlung.",
            "control": "Steuerung",
            "active": "Aktiv",
            "inactive": "Inaktiv",
            "watt": "Watt",
            "price_based_heating_plan": "Preisbasierter Heizplan",
            "price_based_heating_plan_muted": "Heizplan anhand dynamischer Preise: Rot = blockierender Betrieb, Grün = erweiterter Betrieb",
            "status": "Status",
            "status_help": <>Der preibasierte Heizplan zeigt den Heizplan anhand der konfigurierten günstigsten und teuersten Stunden an. Der Heizplan muss nicht der echten späteren Steuerung der Heizung entsprechen, da z.B. ein unerwartet hoher PV-Überschuss den blockierenden Betrieb verhindern kann.</>,
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
                <p>Wenn die PV-Überschuss-Steuerung aktiviert ist, wird die Heizung in den erweiterten Betrieb gebracht, sobald der Überschuss am Netzanschluss den eingestellten Wert überschreitet.</p>
                <p>Wenn die Dynamische-Strompreis-Steuerung aktiviert ist, wird die Heizung zusätzlich für die günstigsten x Stunden am Tag in den erweiterten Betrieb gebracht.</p>
                <p>Wenn die PV-Ertragsprognosen-Steuerung aktiviert ist und die Prognose größer als die eingestellte Energie ausfällt, entfällt die Dynamische-Strompreis-Steuerung und es wird ausschließlich der PV-Überschuss genutzt.</p>
                <p>Beispiel-Konfiguration: Erweiterter Betrieb bei PV-Überschuss <b>ab 1200W</b> oder für <b>günstigste 4 Stunden pro Tag</b> aber nur wenn PV-Ertragsprognose <b>unter 10 kWh/Tag</b>.</p>
                <p>In diesem Beispiel würde bei einem PV-Überschuss von 1200W, ungeachtet der folgenden Einstellungen, der erweiterte Betrieb aktiviert. Zusätzlich würde für die 4 günstigsten Stunden am Tag der erweiterte Betrieb aktiviert, aber nur falls die PV-Ertragsprognose weniger als 10 kWh für den Tag vorher gesagt hat. Falls ein höherer Ertrag erwartet wird, wird kein erweiterter Betrieb aktiviert.</p>
                <p>Ziel dieser Einstellungen: Wenn ein hoher PV-Ertrag erwartet wird, wird kein Strom aus dem Netz für den erweiterten Betrieb bezogen, trotz dass der Strompreis günstig ist.</p>
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
            "h_per_x": /*SFN*/(x: number) => x == 24 ? "h/Tag" : "h/" + x + "h"/*NF*/,
            "h_per_day": "h/Tag",
            "for_the_most_expensive": "bei den teuersten",
            "p14_enwg_help": <>
                <p>Ab dem 01.01.2024 fordert der §14a des EnWG die Steuerbarkeit von Wärmepumpen. Konkret müssen neu verbaute Wärmepumpen über eine Kommunikationsschnittstelle zum Netzbetreiber verfügen, damit dieser in Ausnahmefällen die Leistung des Geräts anpassen kann.</p>
                <p>Die Steuerung nach §14a EnWG kann hier aktiviert werden. Dazu wird ein Eingang vom Netzbetreiber gesteuert und im Gegenzug wird über den SG-Ready-Ausgang 1 die Wärmepumpe entsprechend des Eingangs gesteuert.</p>
                <p>Hinweis: Die Steuerung nach §14a EnWG hat Priorität gegenüber der oben konfigurierten Heizungssteuerung.</p>
            </>,
            "control_period": "Regelzeitraum",
            "control_period_help": <>
                <p>Der Regelzeitraum bestimmt, für welchen Zeitraum die eingestellten Steuerregeln aktiv sind.</p>
                <p>Beispielkonfiguration:</p>
                <ul>
                    <li>Regelzeitraum: 24 Stunden</li>
                    <li>Erweiterter Betrieb: Günstigste 8 Stunden</li>
                    <li>Blockierender Betrieb: Teuerste 8 Stunden</li>
                </ul>
                <p>In diesem Fall Kann es passieren das bis zu 8 Stunden am Stück der erweiterte Betrieb und bis zu 8 Stunden am Stück der blockierende Betrieb geschaltet wird. Angenommen die Heizung muss mindestens 2x pro Tag laufen damit eine gewünschte Temperatur eingehalten wird, würde sich eventuell folgende Konfiguration anbieten:</p>
                <ul>
                    <li>Regelzeitraum: 12 Stunden</li>
                    <li>Erweiterter Betrieb: Günstigste 4 Stunden</li>
                    <li>Blockierender Betrieb: Teuerste 4 Stunden</li>
                </ul>
                <p>In dieser Konfiguration würde die Heizung auch pro Tag 8 Stunden in den erweiterten Betrieb sowieso 8 Stunden in den blockiernden Betrieb gebracht.</p>
                <p>Allerdings wird der Tag in zwei 12 Stunden Regelzeiträume geteilt in denen je die günstigsten/teuersten Steunden gesucht werden und entsprechend kann die Heizung pro 12 Stunden auch höchsten für 4 Stunden am Stück in den erweiterten oder blockierenden Betrieb gebracht werden.</p>
            </>,
            "control_period_muted": "Zeitraum in dem die günstigsten Stunden für den erweiterten und die teuersten Stunden für den blockierenden Betrieb bestimmt werden."
        },
        "script": {
            "save_failed": "Speichern der Heizungseinstellungen fehlgeschlagen",
            "reboot_content_changed": "Heizungseinstellungen"
        }
    }
}
