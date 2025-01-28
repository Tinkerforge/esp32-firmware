/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eco": {
        "status": {},
        "navbar": {
            "eco": "Eco Mode"
        },
        "content": {
            "eco": "Eco Mode",
            "active": "Active",
            "inactive": "Inactive",
            "solar_forecast_needs_activation": <>The <a href="#solar_forecast">solar forecast</a> must be enabled for this configuration.</>,
            "day_ahead_prices_needs_activation": <>The <a href="#day_ahead_prices">day ahead prices</a> must be enabled for this configuration.</>,
            "charge_plan_enable_help": <>
                <p>If the charge plan is enabled, a configurable charge plan is shown on the status page.</p>
                <p>With the help of the charging plan as well as day ahead prices and an optional solar forecast, the charging of electric cars can be optimized.</p>
            </>,
            "mode_after_help": <>
                <p>Specifies which mode to switch to when the charge plan has expired.</p>
                <p>Example: The charge plan is configured to 'Use the cheapest <b>4 hours</b> until <b>Tomorrow at 08:00</b>' and the mode after the charge plan has expired is set to 'PV'. In this case, the charge plan would be executed and <b>from tomorrow 08:00</b> the mode would be switched to <b>PV</b>. After that, only PV excess charging is possible (until a new charge plan is set).</p>
                <p>If the charge plan is configured to 'Daily' this setting does not apply.</p>
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
            "reboot_content_changed": "eco settings"
        }
    }
}
