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
            "price_invalid": "The price must be set in cent per kWh, not euro per kWh.",
            "no_data": "No data available!",
            "loading": "Loading data...",
            "time": "Time",
            "electricity_price": "Electricity price",
            "grid_fees_plus_taxes": "Grid fees and taxes",
            "surcharge": "Surcharge",
            "day_ahead_prices_muted": <>The data is provided by <a href="https://www.entsoe.eu/about/" target="_blank">ENTSO-E</a> and fetched from api.warp-charger.com.</>,
            "day_ahead_prices_desc": "Enables automatic download of day-ahead market prices. For this feature the device requires internet access",
            "region": "Region",
            "germany": "Germany",
            "austria": "Austria",
            "luxembourg": "Luxembourg",
            "resolution": "Resolution",
            "resolution_muted": "Which resolution for market prices does the electricity provider use.",
            "minutes15": "15 minutes",
            "minutes60": "60 minutes",
            "optional_muted": "Optional: Only used for correct display of the price, not for control",
            "euro_divided_by_month": "€/Month",
            "grid_fees_and_taxes": "Grid fees and taxes",
            "electricity_provider_surcharge": "Electricity provider surcharge",
            "electricity_provider_base_fee": "Electricity provider base fee",
            "day_ahead_market_prices_heading": "Day-Ahead Market Prices",
            "price_ct_per_kwh": "Price [ct/kWh]",
            "current_price": "Current market price",
            "average_price": "Average price",
            "undefined": "Undefined",
            "now": "Now",
            "vat": "VAT",
            "vat_help": <>
                <p>Here the VAT for the net market price is set. There are three sensible options for the optional settings:</p>
                <p>
                    <ul>
                        <li>Leave everything at 0: The net market price is displayed.</li>
                        <li>Leave VAT at 0 and enter the net price for the fees: The net electricity price including fees is displayed.</li>
                        <li>Enter VAT correctly and enter the gross price for the fees: The gross electricity price including fees is displayed.</li>
                    </ul>
                </p>
            </>
        },
        "automation": {
            "trigger_text": /*FFN*/(type: number, comparison: number, value: number) => {
                if (type == 0) {// average
                    if (comparison == 0) { // greater
                        return (<>If the current market price is greater than {value}% of the daily average,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>If the current market price is less than {value}% of the daily average,{" "}</>)
                    }
                } else if (type == 1) { // absolute
                    if (comparison == 0) { // greater
                        return (<>If the current market price is greater than {value} ct,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>If the current market price is less than {value} ct,{" "}</>)
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
