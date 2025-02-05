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
            "charge_plan_enable": "Ladeplanung aktivieren",
            "charge_plan_enable_desc": "Ladeplanung anhand von dynamischen Strompreisen und PV-Ertragsprognose.",
            "park_time": "Maximale Standzeit",
            "charge_below": "Immer laden wenn Preis unter",
            "block_above": "Nie laden wenn Preis über",
            "yield_forecast_threshold": "Nur wenn PV-Ertragsprognose unter",

            "active": "Aktiv",
            "inactive": "Inaktiv",
            "solar_forecast_needs_activation": <>Um den erwarteten PV-Überschuss in den Ladeplan einbeziehen zu können, muss die <a href="#solar_forecast">PV-Ertragsprognose</a> aktiviert werden.</>,
            "day_ahead_prices_needs_activation": <>Um den Ladeplan berechnen zu können, müssen die <a href="#day_ahead_prices">dynamischen Strompreise</a> aktiviert werden.</>,
            "charge_plan_enable_help": <>
                <p>Wenn die Ladeplanung aktiviert ist wird auf der Status-Seite ein konfigurierbarer Ladeplan angezeigt.</p>
                <p>Mit Hilfe des Ladeplans sowie dynamischen Strompreisen und einer optionalen PV-Ertragsprognose kann die Ladung des Elektroautos optimiert werden.</p>
            </>,
            "park_time_help": <>
                <p>Gibt die maximale Standzeit eine Fahrzeugs an.</p>
                <p>Beispiel: Es ist 08:00, der Ladeplan ist konfiguriert auf 'Nutze die günstigsten <b>4 Stunden</b> bis <b>Heute um 20:00 Uhr</b>' und die maximale <b>Standzeit ist auf 8 Stunden</b> eingestellt. In diesem Fall würde der Ladeplan dafür sorgen dass die <b>günstigsten 4 Stunden</b> genutzt werden <b>bevor die maximale Standzeit abläuft</b> (also bis 16:00 wenn das Auto um 08:00 eingesteckt wird).</p>
                <p>Mit dieser Konfiguration kann man z.B. die Arbeitszeit von Mitarbeitern berücksichtigen.</p>
            </>,
            "charge_below_help": <>
                <p>Gibt einen Preis in ct an unter dem eine Ladung immer freigegeben wird, unabhängig vom Ladeplan.</p>
                <p>Es wird mit dem Preis ohne Netzentgelte und Steuern verglichen.</p>
            </>,
            "block_above_help": <>
                <p>Gibt einen Preis in ct an über dem eine Ladung nie freigegeben wird, unabhängig vom Ladeplan.</p>
                <p>Es wird mit dem Preis ohne Netzentgelte und Steuern verglichen.</p>
            </>,
            "yield_forecast_threshold_help": <>
                <p>Gibt einen Schwellwert in kWh an über dem eine Ladung nur bei PV-Überschuss freigegeben wird (der Ladeplan wird ignoriert).</p>
                <p>Beispiel: 'Nur wenn PV-Ertragsprognose unter' ist auf <b>75kWh/Tag</b> konfiguriert und die <b>PV-Ertragsprognose für heute liegt bei 80kWh/Tag</b> (über 75kWh). In diesem Fall würde der <b>Ladeplan nicht ausgeführt</b> und nur geladen wenn PV-Überschuss verfügbar ist.</p>
                <p>Die Idee ist keinen Strom einzukaufen (auch nicht wenn der Strom günstig ist) wenn bekannt ist dass genug PV-Überschuss verfügbar sein wird.</p>
            </>
        },
        "script": {
            "save_failed": "Speichern der Eco-Einstellungen fehlgeschlagen",
            "reboot_content_changed": "Eco-Einstellungen",

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
                    const status = `Status: Kein Auto angeschlossen.`;
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
