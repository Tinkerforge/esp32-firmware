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
            "enable_day_ahead_prices": "Dynamischer Strompreis aktiviert",
            "price_invalid": "Der Preis muss in Cent pro kWh, nicht in Euro pro kWh angegeben werden.",
            "no_data": "Keine Daten vorhanden!",
            "loading": "Lade Daten...",
            "time": "Uhrzeit",
            "electricity_price": "Strompreis",
            "grid_fees_plus_taxes": "Netzentgelte + Steuern",
            "surcharge": "Preisaufschlag",
            "day_ahead_prices_muted": <>Die Daten werden von <a href="https://www.entsoe.eu/about/" target="_blank">ENTSO-E</a> bereitgestellt und über api.warp-charger.com bezogen.</>,
            "day_ahead_prices_desc": "Lädt automatisch die Day-Ahead-Börsenstrompreise herunter. Für diese Funktion muss das Gerät Zugriff auf das Internet haben.",
            "region": "Bereich",
            "germany": "Deutschland",
            "austria": "Österreich",
            "luxembourg": "Luxenburg",
            "resolution": "Auflösung",
            "resolution_muted": "Welche Auflösung für Börsenstrompreise verwendet der Stromanbieter.",
            "minutes15": "15 Minuten",
            "minutes60": "60 Minuten",
            "optional_muted": "Optional: Wird nur für die korrekte Darstellung des Preises verwendet, nicht für die Regelung.",
            "euro_divided_by_month": "€/Monat",
            "grid_fees_and_taxes": "Netzentgelte und Steuern",
            "electricity_provider_surcharge": "Stromanbieter Preisaufschlag",
            "electricity_provider_base_fee": "Stromanbieter Grundgebühr",
            "day_ahead_market_prices_heading": "Status",
            "price_ct_per_kwh": "Preis [ct/kWh]",
            "current_price": "Aktueller Börsenpreis",
            "average_price": "Durchschnittspreis",
            "undefined": "Unbekannt",
            "now": "Jetzt",
            "vat": "MwSt",
            "extra_costs_help": <>
                <p>Die dynamischen Börsenstrompreise sind nur ein Teil der Stromkosten. Es fallen zusätzliche fixe Steuern, Netzentgelte und Aufschläge an.</p>
                <p>Die MwSt bezieht sich auf die MwSt die auf den Börstenstrompreis anfällt (in DE aktuell 19%). Weitere Zusatzkosten können in der Rechnung des Stromanbieters nachgeschaut werden.</p>
                <p>Es gibt drei sinnvolle Möglichkeiten für die optionalen Einstellungen:</p>
                <ul>
                    <li>Alles auf 0 lassen: Es wird der Netto-Börsenpreis angezeigt.</li>
                    <li>MwSt auf 0 lassen und bei den Gebühren den Nettopreis eintragen: Es wird der Netto-Strompreis inkl. Gebühren angezeigt.</li>
                    <li>MwSt korrekt eintragen und bei den Gebühren den Bruttopreis eintragen: Es wird der Brutto-Strompreis inkl. Gebühren angezeigt.</li>
                </ul>
                <p>Zur Regelung werden unabhängig von den Zusatzkosten immer nur die dynamischen Netto-Börsenpsreise verwendet.</p>
            </>,
            "incl_all_costs": "inkl. aller Kosten",
            "extra_costs": "Zusatzkosten"
        },
        "automation": {
            "trigger_text": /*FFN*/(type: number, comparison: number, value: number) => {
                if (type == 0) {// average
                    if (comparison == 0) { // greater
                        return (<>Wenn der aktuelle Börsenstrompreis <b>größer</b> als <b>{value} %</b> des Tagesdurchschnitt ist,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>Wenn der aktuelle Börsenstrompreis <b>kleiner</b> als <b>{value} %</b> des Tagesdurchschnitt ist,{" "}</>)
                    }
                } else if (type == 1) { // absolute
                    if (comparison == 0) { // greater
                        return (<>Wenn der aktuelle Börsenstrompreis <b>größer</b> als <b>{value} ct</b> ist,{" "}</>)
                    } else if (comparison == 1) { // less
                        return (<>Wenn der aktuelle Börsenstrompreis <b>kleiner</b> als <b>{value} ct</b> ist,{" "}</>)
                    }
                }
                return (<>Unbekannt,{" "}</>)
            }/*NF*/,
            "comparative_value": "Vergleichswert",
            "comparative_value_muted": "Der Vergleich findet auf dem Netto-Börsenstrompreis statt (ohne Netzentgelte, Steuern, Preisaufschläge etc.)",
            "comparative_value_percent": "Prozentueller Vergleich zum Tagesdurchschnitt",
            "comparative_value_absolute": "Vergleich zum absoluten Wert in Cent",
            "comparison": "Vergleich",
            "comparison_greater_than": "Größer als",
            "comparison_less_than": "Kleiner als",
            "value": "Wert",
            "current_electricity_price": "Aktueller Strompreis"
        },
        "script": {
            "save_failed": "Speichern der Strompreiseinstellungen fehlgeschlagen",
            "reboot_content_changed": "Strompreiseinstellungen"
        }
    }
}
