/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "day_ahead_prices": {
        "status": {},
        "navbar": {
            "day_ahead_prices": "Dynamischer Strompreis"
        },
        "content": {
            "day_ahead_prices": "Dynamischer Strompreis",
            "price_invalid": "Der Preis muss in Cent pro kWh, nicht in Euro pro kWh angegeben werden.",
            "no_data": "Keine Daten vorhanden!",
            "loading": "Lade Daten...",
            "time": "Zeit",
            "electricity_price": "Strompreis",
            "grid_fees_plus_taxes": "Netzentgelte + Steuern",
            "surcharge": "Preisaufschlag",
            "day_ahead_prices_muted": <>Die Daten werden von <a href="https://www.entsoe.eu/about/" target="_blank">ENTSO-E</a> bereitgestellt und über api.warp-charger.com bezogen.</>,
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
            "current_price": "Aktueller Börsenpreis",
            "undefined": "Unbekannt",
            "now": "Jetzt",
            "vat": "MwSt",
            "vat_help": <>
                <p>Hier wird die MwSt für den Netto-Börsenpreis angegeben. Es gibt drei sinnvolle Möglichkeiten für die optionalen Einstellungen:</p>
                <p>
                    <ul>
                        <li>Alles auf 0 lassen: Es wird der Netto-Börsenpreis angezeigt.</li>
                        <li>MwSt auf 0 lassen und bei den Gebühren den Nettopreis eintragen: Es wird der Netto-Strompreis inkl. Gebühren angezeigt.</li>
                        <li>MwSt korrekt eintragen und bei den Gebühren den Bruttopreis eintragen: Es wird der Brutto-Strompreis inkl. Gebühren angezeigt.</li>
                    </ul>
                </p>
            </>
        },
        "automation": {
            "automation_trigger_text": /*FFN*/(type: number, comparison: number, value: number) => {
                if (type == 0) {// average
                    if (comparison == 0) { // greater
                        return (<>Wenn der aktuelle Börsenstrompreis größer als {value}% des Tagesdurchschnitt ist,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>Wenn der aktuelle Börsenstrompreis kleiner als {value}% des Tagesdurchschnitt ist,{" "}</>)
                    }
                } else if (type == 1) { // absolute
                    if (comparison == 0) { // greater
                        return (<>Wenn der aktuelle Börsenstrompreis größer als {value} ct ist,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>Wenn der aktuelle Börsenstrompreis kleiner als {value} ct ist,{" "}</>)
                    }
                }
                return (<>Unbekannt,{" "}</>)
            }/*NF*/
        },
        "script": {
            "save_failed": "Speichern der Strompreiseinstellungen fehlgeschlagen",
            "reboot_content_changed": "Strompreiseinstellungen"
        }
    }
}
