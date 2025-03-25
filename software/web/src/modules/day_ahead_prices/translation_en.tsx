/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "day_ahead_prices": {
        "status": {},
        "navbar": {
            "day_ahead_prices": "Dynamic Tariff"
        },
        "content": {
            "day_ahead_prices": "Dynamic Tariff",
            "enable_day_ahead_prices": "Dynamic tariff enabled",
            "no_data": "No data available!",
            "loading": "Loading data...",
            "time": "Time of day",
            "electricity_price": "Electricity price",
            "grid_fees_plus_taxes": "Grid fees and taxes",
            "surcharge": "Surcharge",
            "day_ahead_prices_muted": /*FFN*/(api_url: string) => <>The data is provided by <a href="https://www.entsoe.eu/about/" target="_blank">ENTSO-E</a> and fetched from <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a>.</>/*NF*/,
            "day_ahead_prices_desc": <>Automatic download of day-ahead spot market prices. {__("The_device")} must be connected to the Internet.</>,
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
            "day_ahead_market_prices_heading": "Status",
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
            "extra_costs": "Extra costs"
        },
        "automation": {
            "trigger_text": /*FFN*/(type: number, comparison: number, value: number) => {
                if (type == 0) {// average
                    if (comparison == 0) { // greater
                        return (<>If the current market price is <b>greater</b> than <b>{value} %</b> of the daily average,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>If the current market price is <b>less</b> than <b>{value} %</b> of the daily average,{" "}</>)
                    }
                } else if (type == 1) { // absolute
                    if (comparison == 0) { // greater
                        return (<>If the current market price is <b>greater</b> than <b>{value} ct</b>,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>If the current market price is <b>less</b> than <b>{value} ct</b>,{" "}</>)
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
