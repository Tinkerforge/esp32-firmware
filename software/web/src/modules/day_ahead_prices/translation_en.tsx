/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "day_ahead_prices": {
        "status": {},
        "navbar": {
            "day_ahead_prices": "Day Ahead Prices"
        },
        "content": {
            "day_ahead_prices": "Day Ahead Prices",
            "enable_day_ahead_prices": "Day ahead prices enabled",
            "price_invalid": "The price must be set in cent per kWh, not euro per kWh.",
            "no_data": "No data available!",
            "loading": "Loading data...",
            "time": "Time of day",
            "electricity_price": "Electricity price",
            "grid_fees_plus_taxes": "Grid fees and taxes",
            "surcharge": "Surcharge",
            "day_ahead_prices_muted": <>The data is provided by <a href="https://www.entsoe.eu/about/" target="_blank">ENTSO-E</a> and fetched from api.warp-charger.com.</>,
            "day_ahead_prices_desc": "Automatic download of day ahead market prices. For this feature the device requires Internet access.",
            "region": "Region",
            "germany": "Germany",
            "austria": "Austria",
            "luxembourg": "Luxembourg",
            "resolution": "Resolution",
            "resolution_muted": "Which resolution for market prices does the electricity provider use.",
            "minutes15": "15 minutes",
            "minutes60": "60 minutes",
            "optional_muted": "Optional: Only used for correct display of the price, not for control.",
            "euro_divided_by_month": "€/Month",
            "grid_fees_and_taxes": "Grid fees and taxes",
            "electricity_provider_surcharge": "Electricity provider surcharge",
            "electricity_provider_base_fee": "Electricity provider base fee",
            "day_ahead_market_prices_heading": "Status",
            "price_ct_per_kwh": "Price [ct/kWh]",
            "current_price": "Current market price",
            "average_price": "Average price",
            "undefined": "Undefined",
            "now": "Now",
            "vat": "VAT",
            "extra_costs_help": <>
                <p>The day ahead prices are only a part of the electricity costs. Additional fixed taxes, grid fees, and surcharges apply.</p>
                <p>The VAT refers to the VAT that applies to the day ahead price (currently 19% in DE). Further additional costs can be found in the electricity provider's invoice.</p>
                <p>There are three sensible options for the optional settings:</p>
                <ul>
                    <li>Leave everything at 0: The net market price is displayed.</li>
                    <li>Leave VAT at 0 and enter the net price for the fees: The net electricity price including fees is displayed.</li>
                    <li>Enter VAT correctly and enter the gross price for the fees: The gross electricity price including fees is displayed.</li>
                </ul>
                <p>For control, only the dynamic net day ahead prices are used, regardless of the additional costs.</p>
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
            "save_failed": "Failed to save the day ahead prices settings",
            "reboot_content_changed": "day ahead prices settings"
        }
    }
}
