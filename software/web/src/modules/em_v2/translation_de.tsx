/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "em_v2": {
        "automation": {
            "input_switches": "Eingang geschaltet",
            "input_index": "Eingangsnummer",

            "relay_switches": "Relais-Ausgang geschaltet",
            "relay_index": "Relais-Ausgang",

            "sgready_switch": "Schalte SG-Ready-Ausgang",
            "sgready_switches": "SG-Ready-Ausgang geschaltet",
            "sgready_index": "SG-Ready-Ausgang",
            "sgready_action_text": /*FFN*/(index: number, state: boolean) => {
                let ret = state ? <><b>schließen</b></> : <><b>öffnen</b></>
                return <>SG-Ready-Ausgang {index+1} {ret}.</>
            }/*NF*/,

            "automation_relay_text": /*FFN*/(input: number, state: boolean) => {
                return <>Wenn <b>Relais-Ausgang {input}</b> {state ? <b>geschlossen</b> : <b>geöffnet</b>} wird, </>
            }/*NF*/,
            "automation_sgready_text": /*FFN*/(input: number, state: boolean) => {
                return <>Wenn <b>SG-Ready-Ausgang {input}</b> {state ? <b>geschlossen</b> : <b>geöffnet</b>} wird, </>
            }/*NF*/
        }
    }
}
