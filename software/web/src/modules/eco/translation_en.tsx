/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eco": {
        "status": {
            "charge_plan": "TODO",
            "daily": "TODO",
            "departure": "TODO",
            "amount": "TODO",
            "enable_charge_plan": "TODO",
            "disable_charge_plan": "TODO"
        },
        "navbar": {
            "eco": "Eco Mode"
        },
        "content": {
            "eco": "Eco Mode",
            "charge_plan_enable": "TODO",
            "charge_plan_enable_desc": "TODO",
            "park_time": "TODO",
            "charge_below": "TODO",
            "block_above": "TODO",
            "yield_forecast_threshold": "TODO",


            "active": "Active",
            "inactive": "Inactive",
            "solar_forecast_needs_activation": <>To be able to include the expected PV excess into the charge plan, the <a href="#solar_forecast">solar forecast</a> must be enabled.</>,
            "day_ahead_prices_needs_activation": <>To be able to calculate the charge plan, the <a href="#day_ahead_prices">day ahead prices</a> must be enabled.</>,
            "charge_plan_enable_help": <>
                <p>If the charge plan is enabled, a configurable charge plan is shown on the status page.</p>
                <p>With the help of the charging plan as well as day ahead prices and an optional solar forecast, the charging of electric cars can be optimized.</p>
            </>,
            "park_time_help": <>
                <p>Specifies the maximum parking time of a vehicle.</p>
                <p>Example: It is 08:00, the charge plan is configured to 'Use the cheapest <b>4 hours</b> until <b>Today at 20:00</b>' and the maximum <b>parking time is set to 8 hours</b>. In this case, the charge plan would ensure that the <b>cheapest 4 hours</b> are used <b>before the maximum parking time expires</b> (until 16:00 if the car is plugged in at 08:00).</p>
                <p>With this configuration it is possible to take the the working hours of employees into account.</p>
            </>,
            "charge_below_help": <>
                <p>Specifies a price in ct below which a charging will always be started, regardless of the charge plan.</p>
                <p>It is compared with the price without grid fees and taxes.</p>
            </>,
            "block_above_help": <>
                <p>Specifies a price in ct above which a charging will never be started, regardless of the charge plan.</p>
                <p>It is compared with the price without grid fees and taxes.</p>
            </>,
            "yield_forecast_threshold_help": <>
                <p>Specifies a threshold in kWh above which a charging will only be started if PV excess is available (the charge plan is ignored).</p>
                <p>Example: 'Only if PV yield forecast below' is configured to <b>75kWh/day</b> and the <b>PV yield forecast for today is 80kWh/day</b> (above 75kWh). In this case the <b>charge plan is not executed</b> and charging is only possible if PV excess is available.</p>
                <p>The idea is not to buy electricity (even if the electricity is cheap) if it is known that enough PV excess will be available.</p>
            </>
        },
        "script": {
            "save_failed": "Failed to save the eco settings",
            "reboot_content_changed": "eco settings",

            "charge_plan": /*FFN*/(charge_plan: {departure: number, enable: boolean, time: Date, amount: number}, charger_zero_start: number, charger_zero_amount: number) => {
                return <><marquee>TODOTODOTODO!!!</marquee></>;
                let day = "bis Heute um";
                if (charge_plan.departure === 1) {
                    day = "bis Morgen um";
                } else if (charge_plan.departure === 2) {
                    day = "täglich bis";
                }

                const active = charge_plan.enable ? "aktiv" : "nicht aktiv";
                const time_str   = charge_plan.time.toLocaleTimeString([], {hour: '2-digit', minute: '2-digit'});
                const plan   = `Aktueller Ladeplan: Nutze die günstigsten ${charge_plan.amount} Stunden ${day} ${time_str} Uhr. Der Ladeplan ist ${active}.`;
                if (!charge_plan.enable || charger_zero_start == 0) {
                    return <>plan</>;
                }

                const start  = charger_zero_start*60*1000;

                if (start == 0) {
                    const status = `Status: Kein Auto angeschlossen.`;
                    return <div>{plan}<br/>{status}</div>;
                }

                const today     = new Date().setHours(0, 0, 0, 0);
                const start_day = new Date(start).setHours(0, 0, 0, 0);

                const begin = today == start_day ?
                    `Ladebeginn: Heute, ${new Date(start).toLocaleString([], {hour: '2-digit', minute: '2-digit'})}` :
                    `Ladebeginn: ${new Date(start).toLocaleString([], {weekday: 'long', hour: '2-digit', minute: '2-digit'})}`;
                const charging_done = `Ladedauer bisher: ${charger_zero_amount} Minuten.`;
                const charging_todo = `Ladedauer ausstehend: ${charge_plan.amount*60 - charger_zero_amount} Minuten.`;

                return <div>{plan}<br/>{begin}<br/>{charging_done}<br/>{charging_todo}</div>;
            }/*NF*/
        }
    }
}
