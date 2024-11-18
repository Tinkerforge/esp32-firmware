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
            "extended_logging": "Extended Logging",
            "extended_logging_activate": "Activates extended logging",
            "extended_logging_description": "More information about the control decisions in the event log",
            "minimum_holding_time": "Minimum Holding Time",
            "minimum_holding_time_muted": "for SG Ready output 1 and SG Ready output 2",
            "remaining_holding_time": "Remaining Holding Time",
            "remaining_holding_time_muted": "",
            "minutes": "Minutes",
            "minute": "Minute",
            "dpc_low": "for cheapest",
            "dpc_high": "Blocking for the most expensive",
            "dpc_extended_help": <>
                <p>This setting controls the activation recommendation (SG Ready output 2).</p>
                <p>If the dynamic price control for low prices is enabled, SG Ready output 2 will be activated as soom as the electricity price is x% or less of the daily average.</p>
                <p>Example: Assume that the average electricity price for a day is 10ct and the threshold is configured to 80%. In that case the Heating would be activated through SG Ready output 2 in the periods of time where the price is 8ct or less.</p>
            </>,
            "dpc_blocking_help": <>
                <p>This setting controls the blocking operation (SG Ready output 1).</p>
                <p>This setting controls SG Ready output 1.</p>
                <p>If the dynamic price control for high prices is enabled, SG Ready output 1 will be activated as soom as the electricity price is x% or more of the daily average.</p>
                <p>Example: Assume that the average electricity price for a day is 10ct and the threshold is configured to 120%. In that case the Heating would be blocked through SG Ready output 1 in the periods of time where the price is 12ct or above.</p>
            </>,
            "pv_excess_control": "if PV excess is above",
            "pv_excess_control_help": <>
                <p>This setting controls the blocking operation (SG Ready output 1).</p>
                <p>If pv excess control is enabled, SG Ready output 1 will be activated as soon as the pv excess at the grid connection exceeds the configured value.</p>
                <p>Example: If the heating uses about 1500W when running, then you should configure 1500W here, such that SG Ready output 1 is automatically activated when a corresponding pv excess is available.</p>
            </>,
            "pv_yield_forecast": "PV yield is below",
            "pv_yield_forecast_help": <>
                <p>This setting controls the activation recommendation (SG Ready output 2).</p>
                <p>If the PV yield forecast is enabled, an activation recommendation can be given to the heating even outside the active time if the PV yield forecast (sum of the yield from one day in kWh) falls below the configured threshold.</p>
                <p>Example: It is 05:00 in the morning and the active time is from 06:00 to 18:00. The PV yield forecast is enabled and the threshold is set to 50kWh, but the forecast only predicts a yield of 10kWh. In this case, an activation recommendation would be sent to the heating even outside the active time if the electricity is cheap.</p>
                <p>The idea is to use the cheap electricity at night when it is known that the sun will not shine during the day.</p>
            </>,
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
            "sg_ready_output1_help": "This operating state is backwards compatible with the EVU lock switched at fixed times and includes a maximum of 2 hours of hard blocking time.",
            "sg_ready_output2_muted": "Output 2 is used for the activation recommendation (SG Ready state 3).",
            "sg_ready_output2_help": "In this operating state, the heat pump runs within the controller in the reinforced mode for room heating and hot water preparation. This is not a definitive start command, but a start recommendation according to today's increase.",
            "control": "Control",
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
            "solar_forecast": "Solar forecast",
            "solar_forecast_needs_activation": <>
                <p style="color:red;">The <a href="#solar_forecast" onClick={() => {window.scrollTo(0, 0); return true;}}>solar forecast</a> must be activated for this configuration.</p>
            </>,
            "day_ahead_prices_needs_activation": <>
                <p style="color:red;">The <a href="#day_ahead_prices" onClick={() => {window.scrollTo(0, 0); return true;}}>dynamic prices</a> must be activated for this configuration.</p>
            </>,
            "meter_needs_activation": <>
                <p style="color:red;">A meter must be selected for this configuration.</p>
            </>,
            "hours": "Hours",
            "update_now": "Update now",
            "extended_operation": "Extended operation",
            "extended_operation_help": <>
                <p>These settings control the extended operation (SG Ready output 2).</p>
                <p>If the PV excess control is enabled, the heating is put into extended operation as soon as the excess at the grid connection exceeds the set value.</p>
                <p>If the dynamic price control is enabled, the heating is put into extended operation for the cheapest x hours per day.</p>
                <p>If the PV yield forecast control is enabled, the heating is put into extended operation if the forecast is below the set value.</p>
                <p>Example configuration: Extended operation with PV excess <b>above 1200W</b> or for <b>cheapest 4 hours per day</b> but only if PV yield forecast <b>below 10 kWh/day</b>.</p>
                <p>In this example, with a PV excess of 1200W, regardless of the following settings, the extended operation would be activated. In addition, the extended operation would be activated for the cheapest 4 hours per day, but only if the PV yield forecast was less than 10 kWh for the previous day. If a higher yield is expected, no extended operation is activated.</p>
                <p>Objective of these settings: If a high PV yield is expected, no electricity is drawn from the grid for extended operation, even if the electricity price is cheap.</p>
            </>,
            "blocking_operation": "Blocking operation",
            "blocking_operation_help": <>
                <p>These settings control the blocking operation (SG Ready output 1).</p>
                <p>If the dynamic price control is enabled, the heating is additionally put into blocking operation for the most expensive x hours per day.</p>
                <p>Example configuration: Blocking operation for the most expensive 5 hours. The heating is blocked for the 5 most expensive hours per day. Exception: If the PV excess control is enabled and the set threshold is exceeded, the blocking operation is lifted and the extended operation for the heating is activated.</p>
            </>,
            "or": "or",
            "but_only_if": "but only if",
            "kwh_per_day": "kWh/day",
            "h_per_day": "h/day"
        },
        "script": {
            "save_failed": "Failed to save the heating settings",
            "reboot_content_changed": "heating settings"
        }
    }
}
