/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
import { toLocaleFixed } from "../../ts/util";
let x = {
    "day_ahead_prices": {
        "status": {},
        "navbar": {
            "day_ahead_prices": "Dynamic Tariff"
        },
        "content": {
            "day_ahead_prices": "Dynamic Tariff",
            "enable_day_ahead_prices": "Dynamic tariff enabled",
            "no_data": "No data available",
            "loading": "Loading data...",
            "time": "Time of day",
            "electricity_price": "Electricity price",
            "grid_fees_plus_taxes": "Grid fees and taxes",
            "surcharge": "Surcharge",
            "day_ahead_prices_desc": <>Enable day-ahead spot market prices.</>,
            "source": "Source",
            "source_spot_market_desc": /*FFN*/(api_url: string) => <>The data is provided by <a href="https://www.entsoe.eu/about/" target="_blank">ENTSO-E</a> and fetched from <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a>.</>/*NF*/,
            "source_spot_market": "Automatic via spot market",
            "source_push": "Push via API",
            "source_push_desc": <>In push mode, price data is provided via the API endpoint <a href="https://docs.warp-charger.com/docs/interfaces/mqtt_http/api_reference/day_ahead_prices" target="_blank">day_ahead_prices/prices_update</a>.</>,
            "region": "Region",
            "germany": "Germany",
            "austria": "Austria",
            "luxembourg": "Luxembourg",
            "resolution": "Resolution",
            "resolution_muted": "depends on electric utility",
            "minutes15": "15 minutes",
            "minutes60": "60 minutes",
            "euro_divided_by_month": "€/month",
            "grid_fees_and_taxes": "Grid fees and taxes",
            "electricity_provider_surcharge": "Electric utility surcharge",
            "electricity_provider_base_fee": "Electric utility basic fee",
            "price_ct_per_kwh": "Price [ct/kWh]",
            "current_price": "Current market price",
            "average_price": "Average price",
            "now": "Now",
            "vat": "Value-added tax",
            "vat_muted": "on market price",
            "extra_costs_help": <>
                <p><strong>Optional: For control, only the net day-ahead spot market price is used; extra costs are only displayed.</strong></p>
                <p>The day-ahead spot price is only a part of the electricity costs. Additional fixed taxes, grid fees and surcharges apply.</p>
                One of the following variants of configuring the extra cost settings is recommended:
                <ul class="mb-0">
                    <li>Don't configure extra costs: The net spot market price is displayed.</li>
                    <li>Leave VAT at 0, enter the <strong>net</strong> price for grid fees, utility surcharges and basic fee: The <strong>net</strong> electricity price including all fees etc. is displayed.</li>
                    <li>Configure VAT to for example 19%, enter the <strong>gross</strong> price for grid fees, utility surcharges and basic fee: The <strong>gross</strong> electricity price including fees etc. is displayed.</li>
                </ul>
            </>,
            "incl_all_costs": "incl. all costs",
            "extra_costs": "Extra costs",
            "enable_calendar": "Price calendar",
            "enable_calendar_help": <>
                <p>The price calendar can be used to implement <strong>Module 3 according to § 14a EnWG</strong> (German Energy Industry Act). Module 3 requires time-variable grid fees, where reduced grid fees apply during off-peak hours and higher fees during peak hours.</p>
                <p>To configure this, enter the grid fee differences as price offsets in the calendar. For example, set a negative value during off-peak hours (when reduced grid fees apply) and a positive value during peak hours. These offsets are added to the day-ahead spot market prices, so that the eco charging optimization automatically prefers the cheaper time slots.</p>
                <p>If the dynamic tariff is disabled, the calendar prices are used as-is without any day-ahead prices.</p>
            </>,
            "enable_calendar_desc": "Add fixed price offsets per weekday and time of day to the day-ahead prices.",
            "enable_calendar_desc_standalone": "Define a fixed price per weekday and time of day.",
            "calendar_mon": "Mon",
            "calendar_tue": "Tue",
            "calendar_wed": "Wed",
            "calendar_thu": "Thu",
            "calendar_fri": "Fri",
            "calendar_sat": "Sat",
            "calendar_sun": "Sun",
            "calendar_prices": "Calendar prices",
            "calendar_prices_muted": "Select cells and enter a price to apply",

            "calendar_edit": "Edit calendar",
            "calendar_apply": "Apply",
            "calendar_ct_kwh": "ct/kWh",
            "calendar_selection": "Selection"
        },
        "automation": {
            "trigger_text": /*FFN*/(type: number, comparison: number, value: number) => {
                if (type == 0) {// average
                    if (comparison == 0) { // greater
                        return (<>If the current market price is <b>greater</b> than <b>{toLocaleFixed(value, 1)} %</b> of the daily average,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>If the current market price is <b>less</b> than <b>{toLocaleFixed(value, 1)} %</b> of the daily average,{" "}</>)
                    }
                } else if (type == 1) { // absolute
                    if (comparison == 0) { // greater
                        return (<>If the current market price is <b>greater</b> than <b>{toLocaleFixed(value, 2)} ct</b>,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>If the current market price is <b>less</b> than <b>{toLocaleFixed(value, 2)} ct</b>,{" "}</>)
                    }
                }
                return (<>Unknown,{" "}</>)
            }/*NF*/,
            "comparative_value": "Comparative value",
            "comparative_value_muted": "Comparison is based on the net market price (without grid fees, taxes, surcharges etc.)",
            "comparative_value_percent": "Percentage comparison to the daily average",
            "comparative_value_absolute": "Comparison to the absolute value in cent",
            "comparison": "Comparison",
            "comparison_greater_than": "Greater than",
            "comparison_less_than": "Less than",
            "value": "Value",
            "current_electricity_price": "Current electricity price"
        },
        "script": {
            "save_failed": "Failed to save the day ahead prices settings"
        }
    }
}
