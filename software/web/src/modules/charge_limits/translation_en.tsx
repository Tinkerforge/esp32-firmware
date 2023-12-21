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
            "min15": "15 minutes",
            "min30": "30 minutes",
            "min45": "45 minutes",
            "h1": "1 hour",
            "h2": "2 hours",
            "h3": "3 hours",
            "h4": "4 hours",
            "h6": "6 hours",
            "h8": "8 hours",
            "h12": "12 hours"
        },
        "automation": {
            "restart": "Reset charge limit",
            "charge_limits_expiration": "Charge limit expired",
            "energy": "Charging energy limit",
            "duration": "Charging duration limit",
            "charge_limits": "Charge limits",
            "unlimited": "Unlimited",
            "unchanged": "Unchanged",
            "min15": "15 minutes",
            "min30": "30 minutes",
            "min45": "45 minutes",
            "h1": "1 hour",
            "h2": "2 hours",
            "h3": "3 hours",
            "h4": "4 hours",
            "h6": "6 hours",
            "h8": "8 hours",
            "h12": "12 hours",
            "automation_trigger_text": <>When the <b>charge limit</b> is reached, </>,
            "automation_action_text": /*FFN*/(duration: string, energy: number, restart: boolean) => {
                if (duration === undefined && energy === -1 && restart) {
                    return <>allow the current charge limit again.</>;
                } else if (duration === undefined && energy === -1) {
                    return <>leave the charge limit <b>unchanged</b>.</>
                } else if (duration === "Unlimited" && energy === 0) {
                    return <>set the charge limit to <b>unlimited</b>.</>
                }

                if (restart) {
                        let duration_limit = <></>;
                        if (duration === undefined) {
                            duration_limit = <>allow the current time limit again</>
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
                            energy_limit = <>allow the current energy limit again</>
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
