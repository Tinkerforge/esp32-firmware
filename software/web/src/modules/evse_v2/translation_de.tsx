/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "evse": {
        "automation": {
            "gpio_out_high": "Hochohmig",
            "gpio_out_low": "Verbunden mit Masse",
            "gpio_out": "Konfigurierbarer Ausgang",
            "automation_trigger_button": "Fronttaster gedrückt",
            "automation_trigger_shutdown_input": "Abschalteingang geschaltet",
            "automation_trigger_gp_input": "Konfigurierbarer Eingang geschaltet",
            "automation_trigger_active_low": "auf geschlossen",
            "automation_trigger_active_high": "auf geöffnet",
            "automation_sd_trigger_text": /*FFN*/(state: boolean) => <>Wenn der <b>Abschalt</b>eingang <b>{state ? "geöffnet" : "geschlossen"}</b> wird,{" "}</>/*NF*/,
            "automation_gpin_trigger_text": /*FFN*/(state: boolean) => <>Wenn der <b>konfigurierbare</b> Eingang <b>{state ? "geöffnet" : "geschlossen"}</b> wird,{" "}</>/*NF*/,
            "automation_button_trigger_text": <>Wenn der <b>Fronttaster gedrückt</b> wird,{" "}</>,
            "automation_gpout_action_text": /*FFN*/(state: number) => state ? <><b>schalte</b> den konfigurierbaren Ausgang <b>hochohmig</b>.</> : <><b>verbinde</b> den Konfigurierbaren Ausgang <b>mit Masse</b>.</>/*NF*/
        }
    }
}
