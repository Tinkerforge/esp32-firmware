/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_limits": {
        "content": {
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
            "h12": "12 Hours"
        },
        "cron": {
            "restart": "Reset charge limit",
            "charge_limits_expiration": "Charge limit expired",
            "energy": "Charging energy limit",
            "duration": "Charging duration limit",
            "charge_limits": "Charge limits",
            "unlimited": "Unlimited",
            "unchanged": "Unchanged",
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
            "cron_action_text": /*FFN*/(duration: string, energy: number, restart: boolean) => {
                if (duration === undefined && energy === -1 && restart) {
                    return <>restart the current charge limit.</>;
                } else if (duration === undefined && energy === -1) {
                    return <>leave the charge limit <b>unchanged</b>.</>
                } else if (duration === "Unlimited" && energy === 0) {
                    return <>set the charge limit to <b>unlimited</b>.</>
                }

                if (restart) {
                        let duration_limit = <></>;
                        if (duration === undefined) {
                            duration_limit = <>restart the current time limit</>
                        } else if (duration !== "Unlimited") {
                            duration_limit = <>allow another <b>{duration.toLowerCase()}</b></>
                        } else if (duration === "Unlimited") {
                            duration_limit = <>remove the time limit</>
                        }

                        let glue = <></>;
                        if (energy !== 0 && energy !== -1 && (duration === "Unlimited" || duration === undefined)) {
                            glue = <> and allow another </>;
                        } else {
                            glue = <> and </>;
                        }


                        let energy_limit = <></>;
                        if (energy === -1) {
                            energy_limit = <>restart the current energy limit</>
                        } else if (energy !== 0) {
                            energy_limit = <><b>{energy / 1000} kWh</b></>
                        } else if (energy === 0) {
                            energy_limit = <>remove the energy limit</>
                        }

                        return (
                        <>
                            {duration_limit}{glue}{energy_limit}.
                        </>
                        );
                }

                let duration_limit = <></>;
                if (duration === undefined) {
                    duration_limit = <>leave the time limit unchanged</>
                } else {
                    duration_limit = <>set the time limit to <b>{duration.toLowerCase()}</b></>
                }

                let glue = <></>;
                if (duration !== undefined && energy !== -1) {
                    glue = <>and</>;
                } else {
                    glue = <>and set</>;
                }

                let energy_limit = <></>;
                if (energy === -1) {
                    energy_limit = <> leave the energy limit unchanged</>
                } else {
                    const energy_limit_value = energy === 0 ? <b>unlimited</b> : <b>{energy / 1000} kWh</b>;
                    energy_limit = <> set the energy_limit to {energy_limit_value}</>
                }

                return (
                  <>
                    {duration_limit} {glue} {energy_limit}.
                  </>
                );
            }/*NF*/
        },
        "script": {
            "override_failed": "Override failed"
        }
    }
}
