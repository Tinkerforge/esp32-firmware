/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "evse": {
        "cron": {
            "gpio_out_high": "Hochohmig",
            "gpio_out_low": "Verbunden mit Masse",
            "gpio_out": "Konfigurierbarer Ausgang",
            "cron_trigger_button": "Fronttaster gedrückt",
            "cron_trigger_shutdown_input": "Abschalteingang geschaltet",
            "cron_trigger_gp_input": "Konfigurierbarer Eingang geschaltet",
            "cron_trigger_active_low": "auf geschlossen",
            "cron_trigger_active_high": "auf geöffnet",
            "cron_sd_trigger_text": /*FFN*/(state: boolean) => <>Wenn der Abschalteingang <b>{state ? "geöffnet" : "geschlossen"}</b> wird,{" "}</>/*NF*/,
            "cron_gpin_trigger_text": /*FFN*/(state: boolean) => <>Wenn der Konfigurierbare Eingang <b>{state ? "geöffnet" : "geschlossen"}</b> wird,{" "}</>/*NF*/,
            "cron_button_trigger_text": /*FFN*/(state: boolean) => <>Wenn der Fronttaster <b>{state ? "gedrückt" : "losgelassen"}</b> wird,{" "}</>/*NF*/,
            "cron_gpout_action_text": /*FFN*/(state: number) => state ? <><b>schalte</b> den Konfigurierbaren Ausgang <b>hochohmig</b>.</> : <><b>verbinde</b> den Konfigurierbaren Ausgang <b>mit Masse</b>.</>/*NF*/
        }
    }
}
