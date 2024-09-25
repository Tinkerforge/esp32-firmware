/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "em_v1": {
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
            }/*NF*/
        }
    }
}
