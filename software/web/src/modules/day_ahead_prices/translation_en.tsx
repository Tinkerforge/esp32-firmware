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
            "day_ahead_prices_desc": "Aktiviert das automatischer herunterladen der Day-Ahead-Börsenstrompreise. Für dieses Feature muss die Wallbox Zugriff zum Internet haben.",
            "region": "Bereich",
            "germany": "Deutschland",
            "austria": "Österreich",
            "luxembourg": "Luxenburg",
            "resolution": "Auflösung",
            "resolution_muted": "Welche Auflösung für Börsenstrompreise verwendet der Stromanbieter.",
            "minutes15": "15 Minuten",
            "minutes60": "60 Minuten",
            "optional_muted": "Optional: Wird nur für die korrekte Darstellung des Preises verwendet, nicht für die Regelung",
            "euro_divided_by_month": "€/Monat",
            "grid_fees_and_taxes": "Netzentgelte und Steuern",
            "electricity_provider_surcharge": "Stromanbieter Preisaufschlag",
            "electricity_provider_base_fee": "Stromanbieter Grundgebühr",
            "day_ahead_market_prices_heading": "Day-Ahead-Börsenpreise",
            "price_ct_per_kwh": "Preis [ct/kWh]",
            "current_price": "Current market price",
            "undefined": "Undefined",
            "now": "Now",
            "vat": "VAT",
            "vat_help": <>
                <p>Hier wird die MwSt für den Netto-Börsenpreis angegeben. Es gibt drei Möglichkeiten für die optionalen Einstellungen:</p>
                <p>
                    <ul>
                        <li>Alles auf 0 lassen: Es wird der Netto-Börsenpreis angezeigt</li>
                        <li>MwSt auf 0 lassen und Bei den Gebühren den Nettopreis eintragen: Es wird der Netto-Strompreis inkl. Gebühren angezeigt</li>
                        <li>MwSt korrekt eintragen und die Gebühren als Bruttopreis eintragen: Es wird der Brutto-Strompreis inkl. Gebühren angezeigt</li>
                    </ul>
                </p>
            </>
        },
        "automation": {
            "automation_trigger_text": /*FFN*/(type: number, comparison: number, value: number) => {
                if (type == 0) {// average
                    if (comparison == 0) { // greater
                        return (<>Wenn der aktuelle Strompreis größer als {value}% des Strompreis-Tagesdurchschnitt ist,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>Wenn der aktuelle Strompreis kleiner als {value}% des Strompreis-Tagesdurchschnitt ist,{" "}</>)
                    }
                } else if (type == 1) { // absolute
                    if (comparison == 0) { // greater
                        return (<>Wenn der aktuelle Strompreis größer als {value} ct ist,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>Wenn der aktuelle Strompreis kleiner als {value} ct ist,{" "}</>)
                    }
                }
                return (<>Unbekannt,{" "}</>)
            }/*NF*/
        },
        "script": {
            "save_failed": "Failed to save the day ahead prices settings",
            "reboot_content_changed": "day ahead prices settings"
        }
    }
}
