/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "energy_manager": {
        "status": {
            "status": "Energiemanager",
            "error_ok": "OK",
            "error_network": "Netzwerkfehler",
            "error_contactor": "Schützfehler",
            "error_config": "Unvollständige Konfiguration",
            "error_internal": "Interner Fehler",

            "no_bricklet": "Interner Fehler, Bricklet nicht gefunden"
        },
        "automation": {
            "contactor_monitoring": "Schützfehler",
            "automation_contactor_monitoring_text": /*FFN*/(contactor: boolean) => {
                if (contactor) {
                    return <>Wenn beim Start <b>kein Schützfehler</b> erkannt wird, </>
                } else {
                    return <>Wenn <b>ein Schützfehler</b> erkannt wird, </>
                }
            }/*NF*/,
            "contactor_monitoring_state": "",
            "contactor_error": "Erkannt",
            "contactor_okay": "Beim Start nicht erkannt",

            "phase_switch": "Phasen umgeschaltet",
            "phase": "",
            "single_phase": "Auf einphasig",
            "three_phase": "Auf dreiphasig",
            "automation_phase_switch_text": /*FFN*/(phase: number) => {
                let ret = <></>;
                switch (phase) {
                    case 1:
                        ret = <><b>einphasig</b></>
                        break;

                    case 3:
                        ret = <><b>dreiphasig</b></>
                        break;
                }
                return <>Wenn das Schütz auf {ret} geschaltet hat, </>
            }/*NF*/,

            "input": /*SFN*/(input: number) => "Eingang " + input + " geschaltet"/*NF*/,
            "state": "",
            "closed": "Auf geschlossen",
            "open": "Auf offen",
            "automation_input_text": /*FFN*/(input: number, state: boolean) => {
                return <>Wenn <b>Eingang {input}</b> {state ? <b>geschlossen</b> : <b>geöffnet</b>} wird, </>
            }/*NF*/,

            "switch_relay": "Schalte Relais",
            "relay_state": "Schalten auf",
            "relay_state_open": "Offen",
            "relay_state_closed": "Geschlossen",
            "relay_action_text": /*FFN*/(state: boolean) => {
                let ret = state ? <><b>schließen</b></> : <><b>öffnen</b></>
                return <>Relais {ret}.</>
            }/*NF*/
        }
    }
}
