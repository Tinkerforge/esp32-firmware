/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "em_v2": {
        "automation": {
            "input_switches": "Input switched",
            "input_index": "Input number",

            "sgready_switch": "Switch SG Ready output",
            "sgready_index": "SG Ready output",
            "sgready_action_text": /*FFN*/(index: number, state: boolean) => {
                let ret = state ? <><b>close</b></> : <><b>open</b></>
                return <>{ret} SG Ready output {index+1}.</>
            }/*NF*/
        }
    }
}
