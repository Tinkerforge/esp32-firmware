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
            "dynamic_price_control": "Dynamic Price Control",
            "dynamic_price_control_activate": "Enables optimization of heating control based on dynamic electricity price",
            "average_price_threshold": "Average Price Threshold",
            "average_price_threshold_description": "If the daily average price is below the threshold, a start command will be given to the heating",
            "pv_excess_control": "PV Excess Control",
            "pv_excess_control_activate": "Enables optimization of heating control based on PV excess",
            "pv_excess_threshold": "PV Excess Threshold",
            "pv_excess_threshold_description": "At what wattage of PV excess should a start command be given to the heating",
            "summer_mode": "Summer Mode",
            "summer_start": "Summer Start",
            "summer_end": "Summer End",
            "block_time": "Block Time",
            "enable_daily_block_period": "Enable Daily Block Period",
            "from": "From",
            "to": "To",
            "morning": "Morning",
            "evening": "Evening",
            "pv_yield_forecast": "PV Yield Forecast",
            "pv_yield_forecast_activate": "Block only if expected yield is above configured blocking threshold",
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
