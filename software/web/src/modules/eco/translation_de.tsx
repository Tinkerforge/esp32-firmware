/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eco": {
        "status": {},
        "navbar": {
            "eco": "Eco-Modus"
        },
        "content": {
            "eco": "Eco-Modus",
            "active": "Aktiv",
            "inactive": "Inaktiv",
            "solar_forecast_needs_activation": <>
                <p style="color:red;">Für diese Konfiguration muss die <a href="#solar_forecast" onClick={() => {window.scrollTo(0, 0); return true;}}>PV-Ertragsprognose</a> aktiviert werden.</p>
            </>,
            "day_ahead_prices_needs_activation": <>
                <p style="color:red;">Für diese Konfiguration müssen die <a href="#day_ahead_prices" onClick={() => {window.scrollTo(0, 0); return true;}}>dynamischen Strompreise</a> aktiviert werden.</p>
            </>,
            "charge_plan_enable_help": <>
                <p>Wenn die Ladeplanung aktiviert ist wird auf der Status-Seite ein konfigurierbarer Ladeplan angezeigt.</p>
                <p>Mit Hilfe des Ladeplans sowie dynamischen Strompreisen und einer optionalen PV-Ertragsprognose kann die Ladung des Elektroautos optimiert werden.</p>
            </>,
            "mode_after_help": <>
                <p>Gibt an auf welchen Modus gewechselt werden soll, wenn der Ladeplan abgelaufen ist.</p>
                <p>Beispiel: Der Ladeplan ist konfiguriert auf 'Nutze die günstigsten <b>4 Stunden</b> bis <b>Morgen um 08:00 Uhr</b>' und der Modus nach Ablauf des Ladeplans ist auf 'PV' konfiguriert. In diesem Fall würde der Ladeplan ausgeführt und <b>ab morgen 08:00 Uhr</b> der Modus auf <b>PV gewechselt</b>. Danach ist dann (bis ein neuer Ladeplan eingestellt wird) nur noch PV-Überschussladen möglich.</p>
                <p>Ist der Ladeplan auf 'Täglich' konfiguriert, kommt diese Einstellung nicht zum tragen.</p>
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
            "reboot_content_changed": "Eco-Einstellungen"
        }
    }
}
