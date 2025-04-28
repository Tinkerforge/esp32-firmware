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
            "override": "Override",
            "override_blocking": "Override blocking operation",
            "discard_override": "Cancel",
            "override_blocked_by_p14enwg": "Override suspended by §14a EnWG",
            "close_now": "Close now",
            "open_now": "Open now",
            "toogle_now_failed": /*SFN*/(x: number) => `Could not toggle SG Ready output ${x}`/*NF*/,
            "extended_logging": "Extended logging",
            "extended_logging_enabled": "Extended logging enabled",
            "extended_logging_description": "More information about the control decisions in the event log",
            "minimum_holding_time": "Minimum holding time",
            "minimum_holding_time_muted": "for SG Ready output 1 and SG Ready output 2",
            "remaining_holding_time": "Remaining holding time",
            "remaining_holding_time_muted": "",
            "minutes": "Minutes",
            "minute": "Minute",
            "dpc_low": "For cheapest",
            "dpc_high": "Blocking for the most expensive",
            "pv_excess_control": "If PV excess is above",
            "pv_yield_forecast": "PV yield is below",
            "p14_enwg_control_enabled": "Control of heating based on §14 EnWG enabled (only relevant for Germany)",
            "input": "Input",
            "output": "Output",
            "throttled_if_input": "Throttled if Input",
            "output_active_closed": "Closed, if active",
            "output_active_open": "Open, if active",
            "input_active_closed": "Active, if closed",
            "input_active_open": "Active, if open",
            "meter_slot_grid_power": "Power meter",
            "meter_slot_grid_power_muted": "typically at the grid connection",
            "meter_slot_grid_power_none": "No power meter configured",
            "sg_ready": "SG Ready",
            "sg_ready_output": "SG Ready Output",
            "sg_ready_output1_muted": "Output 1 is used for blocking operation (SG Ready state 1).",
            "sg_ready_output1_help": "This operating state is backwards compatible with the EVU lock switched at fixed times and includes a maximum of 2 hours of hard blocking time.",
            "sg_ready_output2_muted": "Output 2 is used for the activation recommendation (SG Ready state 3).",
            "sg_ready_output2_help": "In this operating state, the heat pump runs within the controller in the reinforced mode for room heating and hot water preparation. This is not a definitive start command, but a start recommendation according to today's increase.",
            "control": "Control",
            "active": "Active",
            "inactive": "Inactive",
            "watt": "W",
            "price_based_heating_plan": "Price-based Heating Plan",
            "price_based_heating_plan_muted": "Heating plan based on dynamic prices: Red = blocking operation, Green = activation recommendation",
            "status": "Status",
            "status_help": <>The <i>price-based heating plan</i> shows the heating plan based on the configured cheapest and most expensive hours. The heating plan does not have to correspond to the actual control of the heating, as e.g. an unexpectedly high PV excess can prevent the blocking operation.</>,
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
            "solar_forecast_needs_activation": <>To be able to include the expected PV excess into the heating plan, the <a href="#solar_forecast">solar forecast</a> must be enabled.</>,
            "day_ahead_prices_needs_activation": <>To be able to include the dynamic energy prices into the heating plan, the <a href="#day_ahead_prices">dynamic prices</a> must be enabled.</>,
            "meter_needs_activation": <>To be able to include the PV excess into the heating plan, a meter must be selected.</>,
            "hours": /*SFN*/(h: number) => "" + h + " hours"/*NF*/,
            "update_now": "Update now",
            "extended_operation": "Extended operation",
            "extended_operation_help": <>
                <p>These settings control the extended operation (SG Ready output 2).</p>
                <p>If the PV excess control is enabled, the heating is put into extended operation as soon as the excess at the grid connection exceeds the set value.</p>
                <p>If the dynamic price control is enabled, the heating is put into extended operation for the cheapest x hours per day.</p>
                <p>If the PV yield forecast control is enabled, the heating is put into extended operation if the forecast is below the set value.</p>
                <p>Example configuration: Extended operation with PV excess <b>above 1200 W</b> or for the <b>cheapest four hours per day</b>, but only if PV yield forecast is <b>below 10 kWh per day</b>.</p>
                <p>In this example, with a PV excess of 1200 W, regardless of the following settings, the extended operation would be enabled. In addition, the extended operation would be enabled for the cheapest four hours per day, but only if the PV yield forecast was less than 10 kWh for the previous day. If a higher yield is expected, no extended operation is enabled.</p>
                <p>Objective of these settings: If a high PV yield is expected, no electricity is drawn from the grid for extended operation, even if the electricity price is cheap.</p>
            </>,
            "blocking_operation": "Blocking operation",
            "blocking_operation_help": <>
                <p>These settings control the blocking operation (SG Ready output 1).</p>
                <p>If the dynamic price control is enabled, the heating is additionally put into blocking operation for the most expensive x hours per day.</p>
                <p>Example configuration: Blocking operation for the most expensive 5 hours. The heating is blocked for the 5 most expensive hours per day. Exception: If the PV excess control is enabled and the set threshold is exceeded, the blocking operation is lifted and the extended operation for the heating is enabled.</p>
            </>,
            "or": "or",
            "but_only_if": "but only if",
            "kwh_per_day": "kWh/day",
            "h_per_x": /*SFN*/(x: number) => x == 24 ? "h/day" : "h/" + x + "h"/*NF*/,
            "h_per_day": "h/day",
            "for_the_most_expensive": "For the most expensive",
            "p14_enwg": "§14a EnWG",
            "p14_enwg_help": <>
                <p>From January 1st 2024, §14a EnWG requires the controllability of heat pumps in Germany. Specifically, newly installed heat pumps must have a communication interface to the grid operator such that the grid operator can adjust the power of the device in exceptional cases.</p>
                <p>The control according to §14a EnWG can be enabled here. For this purpose, an input is controlled by the grid operator and in return the heat pump is controlled via the SG Ready output 1 according to the input.</p>
                <p>Note: The control according to §14a EnWG takes precedence over the heating control configured above.</p>
            </>,
            "control_period": "Control period",
            "control_period_help": <>
                <p>The control period determines for which period the control rules are active.</p>
                <p>Example configuration:</p>
                <ul>
                    <li>Control period: 24 hours</li>
                    <li>Extended operation: Cheapest 8 hours</li>
                    <li>Blocking operation: Most expensive 8 hours</li>
                </ul>
                <p>In this case, it can happen that up to 8 hours at a time the extended operation and up to 8 hours at a time the blocking operation is switched on. Assuming the heating must run at least 2 times a day to maintain a desired temperature, the following configuration might be appropriate:</p>
                <ul>
                    <li>Control period: 12 hours</li>
                    <li>Extended operation: Cheapest 4 hours</li>
                    <li>Blocking operation: Most expensive 4 hours</li>
                </ul>
                <p>In this configuration, the day is divided into two 12-hour control periods in which the cheapest/most expensive hours are determined and accordingly the heating can be brought into extended or blocking operation for a maximum of 4 hours at a time per 12 hours.</p>
            </>,
            "control_period_muted": "Period in which the cheapest hours for the extended and the most expensive hours for the blocking operation are determined. The better the storage capacity of the building, the larger the periods can be chosen."
        },
        "script": {
            "save_failed": "Failed to save the heating settings"
        }
    }
}
