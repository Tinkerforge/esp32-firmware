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
            "contactor_monitoring": "Contactor error",
            "automation_contactor_monitoring_text": /*FFN*/(contactor: boolean) => {
                if (contactor) {
                    return <>When <b>no contactor error</b> is detected on boot, </>
                } else {
                    return <>When <b>a contactor error</b> is detected, </>
                }
            }/*NF*/,
            "contactor_monitoring_state": "",
            "contactor_error": "Detected",
            "contactor_okay": "Not detected on boot",

            "phase_switch": "Phases switched",
            "phase": "",
            "single_phase": "To single phase",
            "three_phase": "To three phases",
            "automation_phase_switch_text": /*FFN*/(phase: number) => {
                let ret = <></>;
                switch (phase) {
                    case 1:
                        ret = <><b>single-phase</b></>
                        break;

                    case 3:
                        ret = <><b>three-phase</b></>
                        break;
                }
                return <>When the contactor switched to {ret}, </>
            }/*NF*/,

            "input": /*SFN*/(input: number) => "Input " + input + " switched"/*NF*/,
            "state": "",
            "closed": "To closed",
            "open": "To open",
            "automation_input_text": /*FFN*/(input: number, state: boolean) => {
                return <>If <b>Input {input}</b> switches to state {state ? <><b>closed</b></> : <><b>open</b></>}, </>
            }/*NF*/,

            "switch_relay": "Switch relay",
            "relay_state": "Switch to",
            "relay_state_open": "Open",
            "relay_state_closed": "Closed",
            "relay_action_text": /*FFN*/(state: boolean) => {
                let ret = state ? <><b>close</b></> : <><b>open</b></>
                return <>{ret} relay.</>
            }/*NF*/
        }
    }
}
