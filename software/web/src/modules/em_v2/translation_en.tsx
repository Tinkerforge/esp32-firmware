/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "em_v2": {
        "automation": {
            "input_switches": "Input switched",
            "input_index": "Input number",

            "relay_switches": "Relay output switched",
            "relay_index": "Relay output",

            "sgready_switch": "Switch SG Ready output",
            "sgready_switches": "SG Ready output switched",
            "sgready_index": "SG Ready output",
            "sgready_action_text": /*FFN*/(index: number, state: boolean) => {
                let ret = state ? <><b>close</b></> : <><b>open</b></>
                return <>{ret} SG Ready output {index+1}.</>
            }/*NF*/,

            "automation_relay_text": /*FFN*/(input: number, state: boolean) => {
                return <>If <b>relay output {input}</b> switches to state {state ? <><b>closed</b></> : <><b>open</b></>}, </>
            }/*NF*/,
            "automation_sgready_text": /*FFN*/(input: number, state: boolean) => {
                return <>If <b>SG Ready output {input}</b> switches to state {state ? <><b>closed</b></> : <><b>open</b></>}, </>
            }/*NF*/
        }
    }
}
