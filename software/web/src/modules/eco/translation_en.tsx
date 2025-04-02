/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eco": {
        "status": {
            "charge_plan": "Charge plan",
            "daily": "Daily",
            "departure": "Departure",
            "amount": "Duration",
            "enable_charge_plan": "Enable charge plan",
            "disable_charge_plan": "Disable charge plan"
        },
        "navbar": {
            "eco": "Eco Mode"
        },
        "content": {
            "eco": "Eco Mode",
            "charge_plan_enable": "Charge planning enabled",
            "charge_plan_enable_desc": "Optimizes charges economically and ecologically. The desired departure time, dynamic tariff pricing and solar forecast are taken into account. Charge plans are used in all \"Eco\" charge modes.",
            "park_time": "Maximum parking time",
            "charge_below": "Always charge when cost below",
            "block_above": "Never charge when cost above",
            "yield_forecast_threshold": "Only if forecasted solar yield below",
            "kwh_per_day": "kWh/day",


            "active": "Active",
            "inactive": "Inactive",
            "solar_forecast_needs_activation": <>To be able to include the expected PV excess into the charge plan, the <a href="#solar_forecast">solar forecast</a> must be enabled.</>,
            "day_ahead_prices_needs_activation": <>To be able to calculate the charge plan, <a href="#day_ahead_prices">dynamic tariff</a> must be enabled.</>,
            "charge_plan_enable_help": "When the charge plan is enabled, a configurable charge plan is shown on the status page if the charge mode is one of the \"Eco\" modes.",
            "park_time_help": <>
                <p>Specifies for how long in the future a charge plan will be created <strong>starting at the moment a vehicle is plugged in</strong>.</p>
                <p>Example: It is 08:00, the charge plan is configured to 'Use the cheapest <b>4 hours</b> until <b>Today at 20:00</b>' and the maximum <b>parking time</b> is set to <b>8 hours</b>. The charge plan will ensure that the <b>cheapest 4 hours</b> are used <b>before the maximum parking time expires</b> (for example until 16:00 if the vehicle is plugged in at 08:00, but until 18:00 if the vehicle is plugged in at 10:00).</p>
                <p>With this configuration, it is possible to take the working hours of employees into account, for example.</p>
            </>,
            "charge_below_help": <>
                <p>Specifies a price in cent below which a charge will <strong>always</strong> be started, regardless of the charge plan.</p>
                <p>The configured price is compared to the net spot market price, excluding any fees, taxes or surcharges.</p>
            </>,
            "block_above_help": <>
                <p>Specifies a price in cent above which a charge will <strong>never</strong> be started, regardless of the charge plan.</p>
                <p>The configured price is compared to the net spot market price, excluding any fees, taxes or surcharges.</p>
            </>,
            "yield_forecast_threshold_help": <>
                <p>Specifies a threshold in kWh above which a charge will only be started if PV excess is available (the charge plan is ignored).</p>
                <p>Example: 'Only if PV yield forecast below' is configured to <b>75 kWh/day</b> and the <b>PV yield forecast for today is 80 kWh/day</b> (above 75 kWh). In this case, the <b>charge plan is not executed</b> and charging is only possible if PV excess is available.</p>
                <p>Therefore, energy is not bought–even if it would be cheap to do so–if it is known that enough PV excess will be available.</p>
            </>
        },
        "script": {
            "save_failed": "Failed to save the eco settings",

            "charge_plan": /*FFN*/(charge_plan: {departure: number, enable: boolean, time: Date, amount: number}, charger_zero_start: number, charger_zero_amount: number) => {
                let day = "until today at";
                if (charge_plan.departure === 1) {
                    day = "until tomorrow at";
                } else if (charge_plan.departure === 2) {
                    day = "daily until";
                }

                const active = charge_plan.enable ? "active" : "not active";
                const time_str   = charge_plan.time.toLocaleTimeString([], {hour: '2-digit', minute: '2-digit'});
                const plan   = `Current charge plan: Use the ${charge_plan.amount} cheapest hours ${day} ${time_str}. The charge plan is ${active}.`;
                if (!charge_plan.enable || charger_zero_start == -1) {
                    return <>{plan}</>;
                }

                const start  = charger_zero_start*60*1000;

                if (start == 0) {
                    const status = `No vehicle connected.`;
                    return <div>{plan}<br/>{status}</div>;
                }

                const today     = new Date().setHours(0, 0, 0, 0);
                const start_day = new Date(start).setHours(0, 0, 0, 0);

                const begin = today == start_day ?
                    `Start of charge: Today ${new Date(start).toLocaleString([], {hour: '2-digit', minute: '2-digit'})}` :
                    `Start of charge: ${new Date(start).toLocaleString([], {weekday: 'long', hour: '2-digit', minute: '2-digit'})}`;
                const charging_done = `Charged: ${charger_zero_amount} minutes.`;
                const charging_todo = `To charge: ${charge_plan.amount*60 - charger_zero_amount} minutes.`;

                return <div>{plan}<br/>{begin}<br/>{charging_done}<br/>{charging_todo}</div>;
            }/*NF*/
        }
    }
}
