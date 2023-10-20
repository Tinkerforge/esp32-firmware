/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_limits": {
        "content": {
            "charge_limits": "Ladelimit",
            "charge_limits_expiration": "Ladelimit abgelaufen",
            "overridden": "(aktiv)",
            "configured": "(konfiguriert)",
            "left": " verbleibend",
            "override_energy": "Energielimit",
            "override_duration": "Zeitlimit",
            "duration": "Zeitlimit",
            "duration_muted": "Kann auf der Statusseite für einen Ladevorgang überschrieben werden.",
            "energy": "Energielimit",
            "energy_muted": "Kann auf der Statusseite für einen Ladevorgang überschrieben werden.",
            "unlimited": "Unbegrenzt",
            "min15": "15 Minuten",
            "min30": "30 Minuten",
            "min45": "45 Minuten",
            "h1": "1 Stunde",
            "h2": "2 Stunden",
            "h3": "3 Stunden",
            "h4": "4 Stunden",
            "h6": "6 Stunden",
            "h8": "8 Stunden",
            "h12": "12 Stunden",
            "cron_trigger_text": "Wenn das Ladelimit erreicht ist, ",
            "cron_action_text": /*FFN*/(duration: string, energy: number) => {
                return (
                  <>
                    setze das Ladelimit auf{" "}
                    {duration !== "Unbegrenzt" && <b>{duration}</b>}
                    {duration !== "Unbegrenzt" && energy !== 0 && " und "}
                    {energy !== 0 && <b>{energy / 1000} kWh</b>}
                    {duration === "Unbegrenzt" && energy === 0 && <b>unbegrenzt</b>}
                    {"."}
                  </>
                );
            }/*NF*/
        },
        "script": {
            "override_failed": "Überschreiben fehlgeschlagen"
        }
    }
}
