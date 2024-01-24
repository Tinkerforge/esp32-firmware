/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_limits": {
        "content": {
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
            "h12": "12 Stunden"
        },
        "automation": {
            "restart": "Ladelimit zurücksetzen",
            "charge_limits_expiration": "Ladelimit abgelaufen",
            "energy": "Energielimit",
            "duration": "Zeitlimit",
            "charge_limits": "Setze Ladelimit",
            "unlimited": "Unbegrenzt",
            "unchanged": "Unverändert",
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
            "automation_trigger_text": <>Wenn das <b>Ladelimit</b> erreicht ist, </>,
            "automation_action_text": /*FFN*/(duration: string, energy: number, restart: boolean) => {
                if (duration === undefined && energy === -1 && restart) {
                    return <>erlaube das aktuelle Ladelimit nochmal.</>;
                } else if (duration === undefined && energy === -1) {
                    return <>lasse das Ladelimit unverändert.</>
                } else if (duration === "Unbegrenzt" && energy === 0) {
                    return <>setze das Ladelimit auf <b>unbegrenzt</b>.</>
                }

                if (restart) {
                    let duration_limit = <></>;
                    if (duration === undefined) {
                        duration_limit = <>erlaube das aktuelle Zeitlimit nochmal</>
                    } else if (duration !== "Unbegrenzt") {
                        duration_limit = <>erlaube weitere <b>{duration}</b></>
                    } else if (duration === "Unbegrenzt") {
                        duration_limit = <>entferne das Zeitlimit</>
                    }

                    let glue = <></>;
                    if (energy !== 0 && energy !== -1 && (duration === "Unbegrenzt" || duration === undefined)) {
                        glue = <> und erlaube weitere </>;
                    } else {
                        glue = <> und </>;
                    }

                    let energy_limit = <></>;
                    if (energy === -1) {
                        energy_limit = <>erlaube das aktuelle Energielimit nochmal</>
                    } else if (energy !== 0) {
                        energy_limit = <><b>{energy / 1000} kWh</b></>
                    } else if (energy === 0) {
                        energy_limit = <>entferne das Energielimit</>
                    }

                    return (
                    <>
                        {duration_limit}{glue}{energy_limit}.
                    </>
                    );
                }

                let duration_limit = <></>;
                if (duration === undefined) {
                    duration_limit = <>lasse das Zeitlimit unverändert</>
                } else {
                    duration_limit = <>setze das Zeitlimit auf <b>{duration}</b></>
                }

                let glue = <></>;
                if (duration !== undefined && energy !== -1) {
                    glue = <>und</>;
                } else {
                    glue = <>und setze</>;
                }

                let energy_limit = <></>;
                if (energy === -1) {
                    energy_limit = <>lasse das Energielimit unverändert</>
                } else {
                    const energy_limit_value = energy === 0 ? <b>unbegrenzt</b> : <b>{energy / 1000} kWh</b>;
                    energy_limit = <> das Energielimit auf {energy_limit_value}</>
                }

                return (
                  <>
                    {duration_limit} {glue} {energy_limit}.
                  </>
                );
            }/*NF*/
        },
        "script": {
            "override_failed": "Überschreiben fehlgeschlagen"
        }
    }
}
