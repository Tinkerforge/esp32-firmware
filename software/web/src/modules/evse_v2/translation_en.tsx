/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "evse": {
        "automation": {
            "gpio_out_high": "High impedance",
            "gpio_out_low": "Connected to ground",
            "gpio_out": "General purpose output",
            "automation_trigger_button": "Front button pressed",
            "automation_trigger_shutdown_input": "Shutdown input switched",
            "automation_trigger_gp_input": "General purpose input switched",
            "automation_trigger_input_closed": "to closed",
            "automation_trigger_input_opened": "to open",
            "automation_sd_trigger_text": /*FFN*/(closed: boolean) => <>If the <b>shutdown</b> input switches to <b>{closed ? "closed" : "open"}</b>{" "}</>/*NF*/,
            "automation_gpin_trigger_text": /*FFN*/(closed: boolean) => <>If the <b>general purpose</b> input switches to <b>{closed ? "closed" : "open"}</b>{" "}</>/*NF*/,
            "automation_button_trigger_text": <>If the <b>button</b> gets <b>pressed</b>{" "}</>,
            "automation_gpout_action_text": /*FFN*/(closed: boolean) => closed ? <><b>connect</b> general purpose output <b>to ground</b>.</> : <>set general purpose output to <b>high impedance</b>.</>/*NF*/
        }
    }
}
