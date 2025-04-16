/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eco": {
        "status": {
            "charge_plan": "Ladeplan",
            "daily": "Täglich",
            "departure": "Abfahrt",
            "amount": "Ladedauer",
            "enable_charge_plan": "Ladeplan aktivieren",
            "disable_charge_plan": "Ladeplan deaktivieren"
        },
        "navbar": {
            "eco": "Eco-Modus"
        },
        "content": {
            "eco": "Eco-Modus",
            "charge_plan_enable": "Ladeplanung aktiviert",
            "charge_plan_enable_desc": "Optimiert Ladevorgänge ökonomisch und ökologisch anhand der gewünschten Abfahrtszeit, dynamischer Strompreise und PV-Ertragsprognose. Ladepläne werden in jedem der \"Eco\"-Lademodi verwendet.",
            "park_time": "Maximale Standzeit",
            "charge_below": "Immer laden, wenn Preis unter",
            "block_above": "Nie laden, wenn Preis über",
            "yield_forecast_threshold": "Nur wenn PV-Ertragsprognose unter",
            "kwh_per_day": "kWh/Tag",

            "active": "Aktiv",
            "inactive": "Inaktiv",
            "solar_forecast_needs_activation": <>Um den erwarteten PV-Überschuss in den Ladeplan einbeziehen zu können, muss die <a href="#solar_forecast">PV-Ertragsprognose</a> aktiviert werden.</>,
            "day_ahead_prices_needs_activation": <>Um den Ladeplan berechnen zu können, müssen die <a href="#day_ahead_prices">dynamischen Strompreise</a> aktiviert werden.</>,
            "charge_plan_enable_help": "Wenn die Ladeplanung aktiviert ist, wird auf der Status-Seite ein konfigurierbarer Ladeplan angezeigt, wenn als Lademodus einer der \"Eco\"-Lademodi ausgewählt ist.",
            "park_time_help": <>
                <p>Gibt an, für wie viele Stunden <strong>nach Anstecken eines Fahrzeugs</strong> ein Ladeplan erstellt wird.</p>
                <p>Beispiel: Es ist 08:00, der Ladeplan ist konfiguriert auf 'Nutze die günstigsten <strong>4 Stunden</strong> bis <strong>Heute um 20:00 Uhr</strong>' und die maximale <strong>Standzeit</strong> ist auf <strong>8 Stunden</strong> eingestellt. Der Ladevorgang wird dann für die <strong>günstigsten 4 Stunden</strong>, aber <strong>bevor die maximale Standzeit abläuft</strong>, geplant, also z.B. bis 16:00, wenn das Auto um 08:00 eingesteckt wird, aber bis 18:00 wenn das Auto um 10:00 eingesteckt wird.</p>
                <p>Mit dieser Konfiguration kann z.B. die Arbeitszeit von Mitarbeitern berücksichtigt werden.</p>
            </>,
            "charge_below_help": <>
                <p>Gibt einen Preis in Cent an, unter dem ein Ladevorgang, unabhängig vom Ladeplan, <strong>immer</strong> freigegeben wird.</p>
                <p>Es wird mit dem Netto-Börsenstrompreis (ohne eventuell konfigurierte Zusatzkosten) verglichen.</p>
            </>,
            "block_above_help": <>
                <p>Gibt einen Preis in Cent an, über dem ein Ladevorgang, unabhängig vom Ladeplan, <strong>nie</strong> freigegeben wird.</p>
                <p>Es wird mit dem Netto-Börsenstrompreis (ohne eventuell konfigurierte Zusatzkosten) verglichen.</p>
            </>,
            "yield_forecast_threshold_help": <>
                <p>Gibt einen Schwellwert in Kilowattstunden an, über dem ein Ladevorgang nur bei PV-Überschuss freigegeben wird (der Ladeplan wird ignoriert).</p>
                <p>Beispiel: 'Nur wenn PV-Ertragsprognose unter' ist auf <strong>75 kWh/Tag</strong> konfiguriert und die <strong>PV-Ertragsprognose für heute liegt bei 80 kWh/Tag</strong>. In diesem Fall würde der <strong>Ladeplan nicht ausgeführt</strong> und nur geladen, wenn PV-Überschuss verfügbar ist.</p>
                <p>Es wird also kein Strom eingekauft, auch nicht, wenn er sehr günstig wäre, wenn bekannt ist, dass genug PV-Überschuss verfügbar sein wird.</p>
            </>
        },
        "script": {
            "save_failed": "Speichern der Eco-Einstellungen fehlgeschlagen",

            "charge_plan": /*FFN*/(charge_plan: {departure: number, enable: boolean, time: Date, amount: number}, charger_zero_start: number, charger_zero_amount: number) => {
                let day = "bis Heute um";
                if (charge_plan.departure === 1) {
                    day = "bis Morgen um";
                } else if (charge_plan.departure === 2) {
                    day = "täglich bis";
                }

                const active = charge_plan.enable ? "aktiv" : "nicht aktiv";
                const time_str   = charge_plan.time.toLocaleTimeString([], {hour: '2-digit', minute: '2-digit'});
                const plan   = `Aktueller Ladeplan: Nutze die günstigsten ${charge_plan.amount} Stunden ${day} ${time_str} Uhr. Der Ladeplan ist ${active}.`;
                if (!charge_plan.enable || charger_zero_start == -1) {
                    return <>{plan}</>;
                }

                const start  = charger_zero_start*60*1000;

                if (start == 0) {
                    const status = `Kein Auto angeschlossen.`;
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
