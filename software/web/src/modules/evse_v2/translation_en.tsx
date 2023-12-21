/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "evse": {
        "cron": {
            "gpio_out_high": "High impedance",
            "gpio_out_low": "Connected to ground",
            "gpio_out": "General purpose output",
            "cron_trigger_button": "Front button pressed",
            "cron_trigger_shutdown_input": "Shutdown input switched",
            "cron_trigger_gp_input": "General purpose input switched",
            "cron_trigger_active_low": "to closed",
            "cron_trigger_active_high": "to open",
            "cron_sd_trigger_text": /*FFN*/(state: boolean) => <>If the shutdown input switches to <b>{state ? "open" : "closed"}</b>{" "}</>/*NF*/,
            "cron_gpin_trigger_text": /*FFN*/(state: boolean) => <>If the General Purpose input switches to <b>{state ? "open" : "closed"}</b>{" "}</>/*NF*/,
            "cron_button_trigger_text": /*FFN*/(state: boolean) => <>If the button is <b>{state ? "pressed" : "released"}</b>{" "}</>/*NF*/,
            "cron_gpout_action_text": /*FFN*/(state: number) => state ? <>set general purpose output to <b>high impedance</b>.</> : <><b>connect</b> general purpose output <b>to ground</b>.</>/*NF*/
        }
    }
}
