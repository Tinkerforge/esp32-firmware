/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "evse": {
        "automation": {
            "gpio_out_open": "Hochohmig",
            "gpio_out_closed": "Verbunden mit Masse",
            "gpio_out": "Konfigurierbarer Ausgang",
            "action_gpio_out": "Schalte konfigurierbaren Ausgang",
            "automation_trigger_button": "Fronttaster gedrückt",
            "automation_trigger_shutdown_input": "Abschalteingang geschaltet",
            "automation_trigger_gp_input": "Konfigurierbarer Eingang geschaltet",
            "automation_trigger_input_closed": "auf geschlossen",
            "automation_trigger_input_opened": "auf geöffnet",
            "automation_sd_trigger_text": /*FFN*/(closed: boolean) => <>Wenn der <b>Abschalt</b>eingang <b>{closed ? "geschlossen" : "geöffnet"}</b> wird,{" "}</>/*NF*/,
            "automation_gpin_trigger_text": /*FFN*/(closed: boolean) => <>Wenn der <b>konfigurierbare</b> Eingang <b>{closed ? "geschlossen" : "geöffnet"}</b> wird,{" "}</>/*NF*/,
            "automation_button_trigger_text": <>Wenn der <b>Fronttaster gedrückt</b> wird,{" "}</>,
            "automation_gpout_action_text": /*FFN*/(closed: boolean) => closed ? <><b>verbinde</b> den konfigurierbaren Ausgang <b>mit Masse</b>.</> : <><b>schalte</b> den konfigurierbaren Ausgang <b>hochohmig</b>.</> /*NF*/
        }
    }
}
