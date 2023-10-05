/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_limits": {
        "content": {
            "charge_limits": "Charge limits",
            "charge_limits_expiration": "Charge limit expiration",
            "overridden": "(active)",
            "configured": "(configured)",
            "left": " left",
            "override_energy": "Energy limit",
            "override_duration": "Time limit",
            "duration": "Charging duration limit",
            "duration_muted": "May be overwritten for a charge on the status page.",
            "energy": "Charging energy limit",
            "energy_muted": "May be overwritten for a charge on the status page.",
            "unlimited": "Unlimited",
            "min15": "15 Minutes",
            "min30": "30 Minutes",
            "min45": "45 Minutes",
            "h1": "1 Hour",
            "h2": "2 Hours",
            "h3": "3 Hours",
            "h4": "4 Hours",
            "h6": "6 Hours",
            "h8": "8 Hours",
            "h12": "12 Hours",
            "cron_trigger_text": "If the charge limit is reached, ",
            "cron_action_text": /*FFN*/(duration: string, energy: number) => {
                return (
                  <>
                    set the charge limit to{" "}
                    {duration !== "Unlimited" && <b>{duration}</b>}
                    {duration !== "Unlimited" && energy !== 0 && " and "}
                    {energy !== 0 && <b>{energy / 1000} kWh</b>}
                    {duration === "Unlimited" && energy === 0 && "unlimited"}
                    {"."}
                  </>
                );
            }/*NF*/
        },
        "script": {
            "override_failed": "Override failed"
        }
    }
}
