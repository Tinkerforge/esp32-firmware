/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "energy_manager": {
        "status": {
            "status": "Energy manager",
            "error_ok": "OK",
            "error_network": "Network error",
            "error_contactor": "Contactor error",
            "error_config": "Incomplete configuration",
            "error_internal": "Internal error",

            "no_bricklet": "Internal error, Bricklet not found"
        },
        "automation": {
            "input": /*SFN*/(input: number) => "Input " + input + " switched"/*NF*/,
            "state": "",
            "closed": "To closed",
            "open": "To open",
            "automation_input_text": /*FFN*/(input: number, state: boolean) => {
                return <>If <b>Input {input}</b> switches to state {state ? <><b>closed</b></> : <><b>open</b></>}, </>
            }/*NF*/,

            "switch_relay": "Switch relay",
            "relay_index": "Relay number",
            "relay_state": "Switch to",
            "relay_state_open": "Open",
            "relay_state_closed": "Closed",
            "relay_action_text": /*FFN*/(index: number, state: boolean) => {
                const index_text = index >= 0 ? " " + index.toString() : "";
                let ret = state ? <><b>close</b></> : <><b>open</b></>
                return <>{ret} relay{index_text}.</>
            }/*NF*/
        }
    }
}
