/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "em_v1": {
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
            }/*NF*/
        }
    }
}
