/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "day_ahead_prices": {
        "status": {},
        "navbar": {
            "day_ahead_prices": "Dynamischer Strompreis"
        },
        "content": {
            "day_ahead_prices": "Dynamischer Strompreis",
            "enable_day_ahead_prices": "Dynamischer Strompreis aktiviert",
            "no_data": "Keine Daten vorhanden!",
            "loading": "Lade Daten...",
            "time": "Uhrzeit",
            "electricity_price": "Strompreis",
            "grid_fees_plus_taxes": "Netzentgelte + Steuern",
            "surcharge": "Preisaufschlag",
            "day_ahead_prices_muted": /*FFN*/(api_url: string) => <>Die Daten werden von <a href="https://www.entsoe.eu/about/" target="_blank">ENTSO-E</a> bereitgestellt und über <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a> bezogen.</> /*NF*/,
            "day_ahead_prices_desc": <>Lädt automatisch die Day-Ahead-Börsenstrompreise herunter. Für diese Funktion muss {__("the_device")} Internetzugriff haben.</>,
            "region": "Region",
            "germany": "Deutschland",
            "austria": "Österreich",
            "luxembourg": "Luxenburg",
            "resolution": "Auflösung",
            "resolution_muted": "abhängig vom Stromanbieter",
            "minutes15": "15 Minuten",
            "minutes60": "60 Minuten",
            "euro_divided_by_month": "€/Monat",
            "grid_fees_and_taxes": "Netzentgelte und Steuern",
            "electricity_provider_surcharge": "Preisaufschlag des Stromtarifs",
            "electricity_provider_base_fee": "Grundgebühr des Stromtarifs",
            "day_ahead_market_prices_heading": "Status",
            "price_ct_per_kwh": "Preis [ct/kWh]",
            "current_price": "Aktueller Börsenpreis",
            "average_price": "Durchschnittspreis",
            "now": "Jetzt",
            "vat": "Mehrwertsteuer",
            "vat_muted": "auf Börsenstrompreis",
            "extra_costs_help": <>
                <p><strong>Optional: Zusatzkosten werden nur für die Darstellung des Preises verwendet, nicht für die Regelung!</strong></p>
                <p>Die dynamischen Börsenstrompreise sind nur ein Teil der Stromkosten. Es fallen zusätzliche fixe Steuern, Netzentgelte und Aufschläge des spezifischen Stromtarifs an.</p>
                Eine der folgenden drei Einstellungen der Zusatzkosten wird empfohlen:
                <ul class="mb-0">
                    <li>Zusatzkosten nicht konfigurieren: Es wird der Netto-Börsenpreis angezeigt.</li>
                    <li>Mehrwertsteuer auf 0% konfigurieren, bei den anderen Kosten jeweils den Nettopreis eintragen: Es wird der Netto-Strompreis inkl. Gebühren angezeigt.</li>
                    <li>Mehrwertsteuer auf z.B. 19% eintragen, bei den anderen Kosten jeweils den Bruttopreis eintragen: Es wird der Brutto-Strompreis inkl. Gebühren angezeigt.</li>
                </ul>
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
            "save_failed": "Speichern der Strompreiseinstellungen fehlgeschlagen"
        }
    }
}
