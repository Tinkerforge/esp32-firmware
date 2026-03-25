/** @jsxImportSource preact */
import { h } from "preact";
import { toLocaleFixed } from "../../ts/util";
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
            "h12": "12 Stunden",

            "custom_energy_limit": "Anderes...",
            "custom_energy_modal_title": "Anderes Energielimit",
            "custom_energy_modal_apply": "Anwenden",
            "custom_energy_limit_label": "Energielimit",
            "custom_energy_limit_cost_label": "Preis ~",

            "soc_target": "SoC-Ziel",
            "soc_target_muted": "Kann auf der Statusseite für einen Ladevorgang überschrieben werden.",
            "override_soc": "SoC-Ziel",
            "soc_currently": "aktuell"
        },
        "automation": {
            "restart": "Ladelimit zurücksetzen",
            "charge_limits_expiration": "Ladelimit abgelaufen",
            "energy": "Energielimit",
            "duration": "Zeitlimit",
            "soc_target": "SoC-Ziel",
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
            "automation_action_text": /*FFN*/(duration: string, energy: number, soc_target_pct: number, restart: boolean) => {
                if (duration === undefined && energy === -1 && soc_target_pct === -1 && restart) {
                    return <>erlaube das aktuelle Ladelimit nochmal.</>;
                } else if (duration === undefined && energy === -1 && soc_target_pct === -1) {
                    return <>lasse das Ladelimit unverändert.</>
                } else if (duration === "Unbegrenzt" && energy === 0 && soc_target_pct === 0) {
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
                        energy_limit = <><b>{toLocaleFixed(energy / 1000, 3)} kWh</b></>
                    } else if (energy === 0) {
                        energy_limit = <>entferne das Energielimit</>
                    }

                    let soc_part = <></>;
                    if (soc_target_pct === -1) {
                        soc_part = <> und erlaube das aktuelle SoC-Ziel nochmal</>;
                    } else if (soc_target_pct > 0) {
                        soc_part = <> und setze das SoC-Ziel auf <b>{soc_target_pct} %</b></>;
                    } else if (soc_target_pct === 0) {
                        soc_part = <> und entferne das SoC-Ziel</>;
                    }

                    return (
                    <>
                        {duration_limit}{glue}{energy_limit}{soc_part}.
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
                    const energy_limit_value = energy === 0 ? <b>unbegrenzt</b> : <b>{toLocaleFixed(energy / 1000, 3)} kWh</b>;
                    energy_limit = <> das Energielimit auf {energy_limit_value}</>
                }

                let soc_part = <></>;
                if (soc_target_pct === -1) {
                    soc_part = <> und lasse das SoC-Ziel unverändert</>;
                } else if (soc_target_pct > 0) {
                    soc_part = <> und setze das SoC-Ziel auf <b>{soc_target_pct} %</b></>;
                } else if (soc_target_pct === 0) {
                    soc_part = <> und entferne das SoC-Ziel</>;
                }

                return (
                  <>
                    {duration_limit} {glue} {energy_limit}{soc_part}.
                  </>
                );
            }/*NF*/
        },
        "script": {
            "override_failed": "Überschreiben fehlgeschlagen"
        }
    }
}
