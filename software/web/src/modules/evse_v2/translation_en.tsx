/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "evse": {
        "content": {
            "error_dc_fault_current": "DC fault protector error",
            "energy_meter_type": "Energy meter type",
            "adc_names": <>CP/PE before resistor (PWM high), CP/PE after resistor (PWM high)<br/>CP/PE before resistor (PWM low), CP/PE after resistor (PWM low)<br/>PP/PE, +12V rail<br/>-12V rail</>,
            "voltage_names": <>CP/PE before resistor (PWM high), CP/PE after resistor (PWM high)<br/>CP/PE before resistor (PWM low), CP/PE after resistor (PWM low)<br/>PP/PE, +12V rail<br/>-12V rail</>,
            "gpio_names_0": "current configuration 0, motor fault, DC fault, current configuration 1",
            "gpio_names_1": "DC fault protector test, shutdown, button, CP-PWM",
            "gpio_names_2": "motor input switch, contactor, GP output, CP separation",
            "gpio_names_3": "motor active, motor phase, contactor check before, contactor check behind",
            "gpio_names_4": "GP input, DC X6, DC X30, LED",
            "gpio_names_5": "unused",
            "gpio_shutdown_muted": <><a href="{{{manual_url}}}">see manual for details</a></>,
            "gpio_shutdown_not_configured": "Not configured",
            "gpio_shutdown_on_open": "Shut down on open",
            "gpio_shutdown_on_close": "Shut down on close",
            "not_configured": "Not configured",
            "active_low_blocked": "Blocks if closed",
            "active_low_prefix": "Limits charge current to ",
            "active_low_suffix": " if closed",
            "active_high_blocked": "Blocks if open",
            "active_high_prefix": "Limits charge current to ",
            "active_high_suffix": " if open",

            "todo": "Have a feature request? Write an e-mail to info@tinkerforge.com",
            "gpio_in_muted": "Readable on GPIO 16",
            "gpio_out_muted": <><a href="https://en.wikipedia.org/wiki/Open_collector">open collector</a></>,
            "button_configuration_muted": "Action to be executed when the button is pressed",
            "button_configuration_deactivated": "No action",
            "button_configuration_start_charging": "Start charging",
            "button_configuration_stop_charging": "Stop charging",
            "button_configuration_start_and_stop_charging": "Start or stop charging",

            "ev_wakeup_desc": "EV Wakeup",
            "ev_wakeup_desc_muted": <><a href="{{{manual_url}}}">see manual for details</a></>,
            "ev_wakeup": "Attempts to wake up the charge controller of the connected vehicle by fake-un and -re-plugging the charge cable.",

            "dc_fault_current_state": "DC fault protector state",
            "dc_fault_current_state_desc": "",
            "dc_fault_current_ok": "OK",
            "dc_fault_current_6_ma": "DC fault",
            "dc_fault_current_system": "System error",
            "dc_fault_current_unknown": "Unknown error",
            "dc_fault_current_calibration": "Calibration error",
            "dc_fault_current_reset": "Reset",

            "reset_dc_fault_title": "Reset the DC fault protector",
            "reset_dc_fault_content": <>Resetting the DC fault protector restores the ability to charge. <b>Ensure that the reason why the DC fault protector triggered is resolved!</b> <a href="{{{manual_url}}}">See manual for details.</a> Really reset the DC fault protector?</>,
            "abort": "Abort",
            "reset": "Reset",
            "trigger_dc_fault_test": "Test DC fault protector",
            "time_since_dc_fault_check": "Time since last DC fault protector test"
        },
        "script": {
            "reset_dc_fault_current_failed": "Reset of the DC fault protector failed",
            "gpio_configuration_failed": "Updating the GPIO configuration failed",

            "meter_type_0": "No energy meter found",
            "meter_type_1": "SDM72",
            "meter_type_2": "SDM630",
            "meter_type_3": "SDM72V2",
            "meter_type_254": "internal",

            "slot_4": "Auto-start or button"
        }
    }
}
