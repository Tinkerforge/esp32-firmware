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
                    Two potential-free switching outputs are available for this purpose:
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
            "minimum_holding_time_muted": "for SG Ready output 1 and SG Ready output 2",
            "minutes": "Minutes",
            "month": "Month",
            "day": "Day",
            "winter_mode": "Winter Mode",
            "winter_start": "Winter Start",
            "winter_end": "Winter End",
            "dpc_low": "Dynamic Price Control",
            "dpc_low_muted": "under % of daily average",
            "dpc_high": "Dynamic Price Control",
            "dpc_high_muted": "over % of daily average",
            "dpc_extended_activate": "Activate SG Ready output 2 when price falls below threshold",
            "dpc_extended_help": <>
                <p>This setting controls the activation recommendation (SG Ready output 2).</p>
                <p>If the dynamic price control for low prices is enabled, SG Ready output 2 will be activated as soom as the electricity price is x% or less of the daily average.</p>
                <p>Example: Assume that the average electricity price for a day is 10ct and the threshold is configured to 80%. In that case the Heating would be activated through SG Ready output 2 in the periods of time where the price is 8ct or less.</p>
            </>,
            "dpc_blocking_activate": "Block the heating (SG Ready output 1) when price threshold is exceeded",
            "dpc_blocking_help": <>
                <p>This setting controls the blocking operation (SG Ready output 1).</p>
                <p>This setting controls SG Ready output 1.</p>
                <p>If the dynamic price control for high prices is enabled, SG Ready output 1 will be activated as soom as the electricity price is x% or more of the daily average.</p>
                <p>Example: Assume that the average electricity price for a day is 10ct and the threshold is configured to 120%. In that case the Heating would be blocked through SG Ready output 1 in the periods of time where the price is 12ct or above.</p>
            </>,
            "pv_excess_control": "PV Excess Control",
            "pv_excess_control_activate": "Enables optimization of heating control based on PV excess",
            "pv_excess_control_help": <>
                <p>This setting controls the blocking operation (SG Ready output 1).</p>
                <p>If pv excess control is enabled, SG Ready output 1 will be activated as soon as the pv excess at the grid connection exceeds the configured value.</p>
                <p>Example: If the heating uses about 1500W when running, then you should configure 1500W here, such that SG Ready output 1 is automatically activated when a corresponding pv excess is available.</p>
            </>,
            "pv_excess_threshold": "PV Excess Threshold",
            "pv_excess_threshold_description": "At what wattage of PV excess should a start command be given to the heating",
            "summer_mode": "Summer Mode",
            "summer_start": "Summer Start",
            "summer_end": "Summer End",
            "summer_settings": "Summer settings",
            "block_time": "Block Time",
            "active_time": "Daily active time",
            "active_time_desc": "Activates a time period in which the sg redy outputs are controlled so that the PV system has priority in summer mode, e.g. from 08:00h to 20:00h",
            "active_time_help": <>
                <p>This setting controls the activation recommendation (SG Ready output 2).</p>
                <p>If the block time is enabled and the current time is within the block time, no activation recommendation is given. This way, it can be prevented that expensive electricity is purchased in the morning when the PV system is supplying electricity during the day.</p>
                <p>Note: The block time can be optimized using a PV yield forecast (see below).</p>
            </>,
            "enable_daily_block_period": "Enable Daily Block Period",
            "from": "From",
            "to": "To",
            "morning": "Block Time Morning",
            "evening": "Block Time Evening",
            "pv_yield_forecast": "PV Yield Forecast",
            "pv_yield_forecast_activate": "Block only if expected yield is above configured blocking threshold",
            "pv_yield_forecast_muted": "if the expected PV yield is below the configured value, the active time is extended to 24h, so that the cheap electricity tariffs can also be used at night.",
            "pv_yield_forecast_help": <>
                <p>This setting controls the activation recommendation (SG Ready output 2).</p>
                <p>If the PV yield forecast is enabled, an activation recommendation can be given to the heating even outside the active time if the PV yield forecast (sum of the yield from one day in kWh) falls below the configured threshold.</p>
                <p>Example: It is 05:00 in the morning and the active time is from 06:00 to 18:00. The PV yield forecast is enabled and the threshold is set to 50kWh, but the forecast only predicts a yield of 10kWh. In this case, an activation recommendation would be sent to the heating even outside the active time if the electricity is cheap.</p>
                <p>The idea is to use the cheap electricity at night when it is known that the sun will not shine during the day.</p>
            </>,
            "blocking_threshold": "Blocking Threshold",
            "blocking_threshold_description": "At what kWh yield forecast should the start command be blocked based on time",
            "p14_enwg_control_activate": "Activates control of heating based on §14 EnWG (only relevant for Germany)",
            "input": "Input",
            "output": "Output",
            "throttled_if_input": "Throttled if Input",
            "closed": "Closed",
            "opened": "Opened",
            "meter_slot_grid_power": "Power meter",
            "meter_slot_grid_power_muted": "typically at the grid connection",
            "meter_slot_grid_power_select": "Select...",
            "meter_slot_grid_power_none": "No power meter configured",
            "sg_ready_output": "SG Ready Output",
            "sg_ready_output1_muted": "Output 1 is used for blocking operation (SG Ready state 1).",
            "sg_ready_output2_muted": "Output 2 is used for the activation recommendation (SG Ready state 3).",
            "general_settings": "General Settings",
            "active": "Active",
            "inactive": "Inactive",
            "watt": "Watt",
            "price_based_heating_plan": "Price-based Heating Plan",
            "price_based_heating_plan_muted": "Heating plan based on dynamic prices: Red = blocking operation, Green = activation recommendation",
            "status": "Status",
            "january": "January",
            "february": "February",
            "march": "March",
            "april": "April",
            "may": "May",
            "june": "June",
            "july": "July",
            "august": "August",
            "september": "September",
            "october": "October",
            "november": "November",
            "december": "December",
            "blocked": "Blocked",
            "extended": "Extended",
            "sg_ready_muted": "Output 1 and output 2",
            "average_price": "Average price",
            "solar_forecast": "Solar forecast"
        },
        "script": {
            "save_failed": "Failed to save the heating settings",
            "reboot_content_changed": "heating settings"
        }
    }
}
