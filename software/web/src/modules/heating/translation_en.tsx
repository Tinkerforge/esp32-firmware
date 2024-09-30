/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "heating": {
        "status": {},
        "navbar": {
            "heating": "Heating"
        },
        "content": {
            "heating": "Heating",
            "control_signal": "Control Signal",
            "control_signal_description": <>
                <div>
                    "SG Ready" is used as the control signal for the heating.
                    The SEB has two potential-free switching outputs for this purpose:
                </div>
                <div>
                    <ul>
                        <li>Output 1 is used for blocking operation (SG Ready state 1).</li>
                        <li>Output 2 is used for the activation recommendation (SG Ready state 3).</li>
                        <li>If both outputs are not switched, normal operation takes place (SG Ready state 2).</li>
                    </ul>
                </div>
                <div>
                    The intelligent winter/summer mode control uses output 2, and
                    the control according to §14 EnWG uses output 1. It is possible to connect only one of the two outputs
                    if only one of the two control types is needed.
                </div>
            </>,
            "extended_logging": "Extended Logging",
            "extended_logging_activate": "Activates extended logging",
            "extended_logging_description": "More information about the control decisions in the event log",
            "minimum_holding_time": "Minimum Holding Time",
            "minimum_holding_time_description": "How long should a start command be valid for",
            "minutes": "Minutes",
            "month": "Month",
            "day": "Day",
            "winter_mode": "Winter Mode",
            "winter_start": "Winter Start",
            "winter_end": "Winter End",
            "dpc_low": "Dynamic Price Control",
            "dpc_high": "Dynamic Price Control",
            "dpc_extended_activate": "Einschaltempfehlung für die Heizung bei unterschreiten der Preisschwelle",
            "dpc_extended_help": <>
                <div>
                    <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                    <p>Wenn die dynamische Preissteuerung für niedrige Preise aktiviert ist, wird der Heizung eine Einschaltempfehlung gegeben sobald der Strompreis x% oder weniger vom Tagesdurchschnitt entspricht.</p>
                    <p>Beispiel: Angenommen der Durchschnittsstrompreis für einen Tag liegt bei 10ct und die Schwelle ist auf 80% konfiguriert. In diesem Fall würde der Heizung über den Tag verteilt eine Einschaltempfehlung gegeben, in den Zeiträumen wo der Preis 8ct oder niedriger ist.</p>
                </div>
            </>,
            "dpc_blocking_activate": "Blockiert die Heizung bei überschreiten der Preisschwelle",
            "dpc_blocking_help": <>
                <div>
                    <p>Diese Einstellung steuert den blockierenden Betrieb (SG-Ready-Ausgang 1).</p>
                    <p>Wenn die dynamische Preissteuerung für hohe Preise aktiviert ist, wird der Heizung in den blockierenden Betrieb gebracht sobald der Strompreis x% oder mehr vom Tagesdurchschnitt entspricht.</p>
                    <p>Beispiel: Angenommen der Durchschnittsstrompreis für einen Tag liegt bei 10ct und die Schwelle ist auf 120% konfiguriert. In diesem Fall würde der Heizung über den Tag verteilt blockiert werden, in den Zeiträumen wo der Preis 12ct oder höher ist.</p>
                </div>
            </>,
            "average_price_threshold": "Price Threshold",
            "average_price_threshold_extended_description": "Liegt der Tagesdurchschnitt unter der Schwelle, wird SG Ready zur Sollwerterhöhung geschaltet",
            "average_price_threshold_blocking_description": "Liegt der Tagesdurchschnitt über der Schwelle, wird SG Ready zur Blockierung geschaltet",
            //"dpc_activate": "Enables optimization of heating control based on dynamic electricity price",
            //"average_price_threshold_description": "If the daily average price is below the threshold, a start command will be given to the heating",
            "pv_excess_control": "PV Excess Control",
            "pv_excess_control_activate": "Enables optimization of heating control based on PV excess",
            "pv_excess_control_help": <>
                <div>
                    <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                    <p>Wenn die PV-Überschuss-Steuerung aktiviert ist, wird der Heizung eine Einschaltempfehlung gegeben sobald der Überschuss am Netzanschluss den eingestellt Wert überschreitet.</p>
                    <p>Beispiel: Verbraucht die Leistung im laufenden Betrieb ca. 1500W, dann dann sollte hier auch 1500W eingetragen werden. In dem Fall würde die Einschaltempfehlung automatisch gegeben, wenn ein entsprechender PV-Überschuss vorhanden ist.</p>
                </div>
            </>,
            "pv_excess_threshold": "PV Excess Threshold",
            "pv_excess_threshold_description": "At what wattage of PV excess should a start command be given to the heating",
            "summer_mode": "Summer Mode",
            "summer_start": "Summer Start",
            "summer_end": "Summer End",
            "block_time": "Block Time",
            "active_time_help": <>
                <div>
                    <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                    <p>Wenn die Blockzierzeit aktiviert ist und die aktuelle Uhrzeit innerhalb der Blockierzeit liegt, wird keine Einschaltempfehlung gegeben. Somit kann in der Sommerzeit verhindert werden dass morgens teurer Strom eingekauft wird, wenn Tagsüber die PV-Anlage Strom liefert.</p>
                    <p>Hinweis: Die Blockierzeit kann über eine PV-Ertragsprognose optimiert werden (siehe unten).</p>
                </div>
            </>,
            "enable_daily_block_period": "Enable Daily Block Period",
            "from": "From",
            "to": "To",
            "morning": "Block Time Morning",
            "evening": "Block Time Evening",
            "pv_yield_forecast": "PV Yield Forecast",
            "pv_yield_forecast_activate": "Block only if expected yield is above configured blocking threshold",
            "pv_yield_forecast_help": <>
                <div>
                    <p>Diese Einstellung steuert die Einschaltempfehlung (SG-Ready-Ausgang 2).</p>
                    <p>Wenn die PV-Überschuss-Steuerung aktiviert ist, wird der Heizung eine Einschaltempfehlung gegeben sobald der Überschuss am Netzanschluss den eingestellt Wert überschreitet.</p>
                    <p>Beispiel: Verbraucht die Leistung im laufenden Betrieb ca. 1500W, dann dann sollte hier auch 1500W eingetragen werden. In dem Fall würde die Einschaltempfehlung automatisch gegeben, wenn ein entsprechender PV-Überschuss vorhanden ist.</p>
                </div>
            </>,
            "blocking_threshold": "Blocking Threshold",
            "blocking_threshold_description": "At what kWh yield forecast should the start command be blocked based on time",
            "p14_enwg_control_activate": "Activates control of heating based on §14 EnWG (only relevant for Germany)",
            "input": "Input",
            "throttled_if_input": "Throttled if Input",
            "closed": "Closed",
            "opened": "Opened",
            "meter_slot_grid_power": "Power meter",
            "meter_slot_grid_power_muted": "typically at the grid connection",
            "meter_slot_grid_power_select": "Select...",
            "meter_slot_grid_power_none": "No power meter configured"
        },
        "script": {
            "save_failed": "Failed to save the heating settings",
            "reboot_content_changed": "heating settings"
        }
    }
}
