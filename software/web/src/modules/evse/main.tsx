/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

import $ from "../../ts/jq";

import feather from "../../ts/feather";

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render } from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import { PageHeader } from "../../ts/page_header";

render(<PageHeader title={__("evse.content.evse")} />, $('#evse_header')[0]);

function update_evse_status_start_charging_button() {
    let state = API.get('evse/state');
    let slots = API.get('evse/slots');
    let ll_state = API.get('evse/low_level_state');

    // It is not helpful to enable the button if auto start is active, but we are blocked for some other reason.
    $('#status_start_charging').prop("disabled", state.iec61851_state != 1 || slots[4].max_current != 0 || ll_state.gpio[0]);
}

function update_evse_state() {
    let state = API.get('evse/state');

    util.update_button_group("btn_group_iec_state", state.iec61851_state);

    util.update_button_group("btn_group_evse_state", state.charger_state);

    $('#status_stop_charging').prop("disabled", state.charger_state != 2 && state.charger_state != 3);

    let allowed_charging_current = util.toLocaleFixed(state.allowed_charging_current / 1000.0, 3) + " A";
    $('#allowed_charging_current').val(allowed_charging_current);

    util.update_button_group("btn_group_ac1", (state.contactor_state & 1) == 1 ? 1 : 0);
    util.update_button_group("btn_group_ac2", state.contactor_state > 1 ? 1 : 0);
    util.update_button_group("btn_group_contactor_error", state.contactor_error != 0 ? 1 : 0, state.contactor_error != 0 ? __("evse.script.error_code") + " " + state.contactor_error : null);
    util.update_button_group("btn_group_error_state", state.error_state == 0 ? 0 : state.error_state - 1); // 1 is not a valid error state
    util.update_button_group("btn_group_lock_state", state.lock_state);
}


function update_evse_hardware_configuration() {
    let cfg = API.get('evse/hardware_configuration');

    util.update_button_group("btn_group_has_lock_switch", cfg.has_lock_switch ? 1 : 0);
    util.update_button_group("btn_group_jumper_config", cfg.jumper_configuration);
    $('#evse_version').val((cfg.evse_version / 10).toFixed(1));

    $('#evse_row_lock_switch').prop('hidden', !cfg.has_lock_switch);
}


function update_evse_low_level_state() {
    let state = API.get('evse/low_level_state');
    let last_iec_state = API.get('evse/state').iec61851_state;

    util.update_button_group("btn_group_led_state", state.led_state);


    for(let i = 0; i < state.gpio.length; ++i) {
        //intentionally inverted: the high button is the first
        util.update_button_group(`btn_group_gpio${i}`, state.gpio[i] ? 0 : 1);
    }

    $('#pwm_duty_cycle').val(util.toLocaleFixed(state.cp_pwm_duty_cycle / 10, 1) + " %");

    for(let i = 0; i < state.adc_values.length; ++i) {
        $(`#adc_value_${i}`).val(state.adc_values[i]);
    }

    for(let i = 0; i < state.voltages.length; ++i) {
        $(`#voltage_${i}`).val(util.toLocaleFixed(state.voltages[i] / 1000.0, 3) + " V");
    }

    for(let i = 0; i < state.resistances.length; ++i) {
        $(`#resistance_${i}`).val(state.resistances[i] + " Ω");
    }

    $('#charging_time').val(util.format_timespan(Math.floor(state.charging_time / 1000)));
    $('#uptime').val(util.format_timespan(Math.floor(state.uptime / 1000)));
    $('#time_since_state_change').val(util.format_timespan(Math.floor(state.time_since_state_change / 1000)));
}

let status_charging_current_dirty = false;


function set_charging_current(current: number) {
    if (status_plus_minus_timeout != null) {
        window.clearTimeout(status_plus_minus_timeout);
        status_plus_minus_timeout = null;
    }

    status_charging_current_dirty = false;
    util.setNumericInput("status_charging_current", current / 1000, 3);

    API.save('evse/global_current', {"current": current}, __("evse.script.set_charging_current_failed"));
}


function update_evse_auto_start_charging() {
    let x = API.get('evse/auto_start_charging');

    $('#status_auto_start_charging').prop("checked", x.auto_start_charging);
}

function set_auto_start_charging(auto_start_charging: boolean) {
    API.save('evse/auto_start_charging', {"auto_start_charging": auto_start_charging}, __("evse.script.auto_start_charging_update"));
}

function start_charging() {
    API.call('evse/start_charging', {}, __("evse.script.start_charging_failed"));
}

function stop_charging() {
    API.call('evse/stop_charging', {}, __("evse.script.stop_charging_failed"));
}

function update_evse_user_calibration() {
    let c = API.get('evse/user_calibration');
    util.update_button_group("btn_group_user_calibration_enabled", c.user_calibration_active ? 1 : 0);
    $('#voltage_diff').val(c.voltage_diff);
    $('#voltage_mul').val(c.voltage_mul);
    $('#voltage_div').val(c.voltage_div);
    $('#resistance_2700').val(c.resistance_2700);
    $('#resistance_880').val(c.resistance_880.join(", "));
}

function update_evse_managed() {
    let x = API.get('evse/management_enabled');
    $('#evse_charge_management').prop("checked", x.enabled);
}


function update_evse_user() {
    let x = API.get('evse/user_enabled');
    $('#evse_user').prop("checked", x.enabled);
}


function update_evse_external() {
    let x = API.get('evse/external_enabled');
    $('#evse_external').prop("checked", x.enabled);
}

function update_evse_slots() {
    let slots = API.get('evse/slots');

    let real_maximum = 32000;
    for(let i = 0; i < slots.length; ++i) {
        let s = slots[i];

        if (!s.active) {
            $(`#slot_${i}`).val(__("evse.script.slot_disabled"));
            $(`#slot_${i}`).css("border-left-color", "#6c757d");
        }
        else if (s.max_current == 0) {
            $(`#slot_${i}`).val(__("evse.script.slot_blocks"));
            $(`#slot_${i}`).css("border-left-color", "#dc3545");
        }
        else if (s.max_current == 32000 && i > 1) {
            $(`#slot_${i}`).val(__("evse.script.slot_no_limit"));
            $(`#slot_${i}`).css("border-left-color", "#28a745");
        }
        else {
            $(`#slot_${i}`).val(util.toLocaleFixed(s.max_current / 1000, 3) + " A");
            $(`#slot_${i}`).css("border-left-color", "#007bff");
        }

        if (s.active)
            real_maximum = Math.min(real_maximum, s.max_current);
    }

    let theoretical_maximum = Math.min(slots[0].max_current, slots[1].max_current);
    let theoretical_maximum_str = util.toLocaleFixed(theoretical_maximum / 1000.0, 0) + " A";
    $('#status_charging_current').prop("max", theoretical_maximum / 1000);
    $("#status_charging_current_maximum").on("click", () => set_charging_current(theoretical_maximum));
    $('#status_charging_current_maximum').html(theoretical_maximum_str);

    if(!status_charging_current_dirty) {
        let shown_current = Math.min(slots[5].max_current, theoretical_maximum);
        util.setNumericInput("status_charging_current", shown_current / 1000.0, 3);
    }

    if (real_maximum == 32000) {
        $('#evse_status_allowed_charging_current').val(util.toLocaleFixed(real_maximum / 1000.0, 3) + " A");
        return;
    }

    let status_string = util.toLocaleFixed(real_maximum / 1000.0, 3) + " A " + __("evse.script.by") + " ";

    let status_list = [];
    for(let i = 0; i < slots.length; ++i) {
        let s = slots[i];
        if (s.active && s.max_current == real_maximum && real_maximum > 0)
            $(`#slot_${i}`).css("border-left-color", "#ffc107");
        if (!s.active || s.max_current != real_maximum)
            continue;

        status_list.push(translate_unchecked(`evse.script.slot_${i}`));
    }

    status_string += status_list.join(", ");

    $('#evse_status_allowed_charging_current').val(status_string);
}

let debug_log = "";
let meter_chunk = "";

function allow_debug(b: boolean) {
    $('#debug_start').prop("disabled", !b);
    $('#debug_stop').prop("disabled", b);
    if (!b) {
        window.onbeforeunload = (e: Event) => {
            e.preventDefault();
            // returnValue is not a boolean, but the string to be shown
            // in the "are you sure you want to close this tab" message
            // box. However this string is only shown in some browsers.
            e.returnValue = __("evse.script.tab_close_warning") as any;
        }
    } else {
        window.onbeforeunload = null;
    }
}

function debug_start() {
    debug_log = "";
    let status = $('#debug_label')[0] as HTMLInputElement;
    status.value = __("evse.script.loading_debug_report");
    allow_debug(false);
    $.get("/debug_report")
        .fail(() => {
            status.value = __("evse.script.loading_debug_report_failed");
            allow_debug(true);
        })
        .done((result) => {
            debug_log += JSON.stringify(result) + "\n\n";

            status.value = __("evse.script.loading_event_log");

            $.get("/event_log")
                .fail(() => {
                    status.value = __("evse.script.loading_event_log_failed");
                    allow_debug(true);
                })
                .done((result) => {
                    debug_log += result + "\n";

                    status.value = __("evse.script.starting_debug");

                    $.get("/evse/start_debug")
                        .fail(() => {
                            status.value = __("evse.script.starting_debug_failed");
                            allow_debug(true);
                        })
                        .done((result) => {
                            status.value = __("evse.script.debug_running");
                        });
                });
        });
}


function debug_stop() {
    let status = $('#debug_label')[0] as HTMLInputElement;

    allow_debug(true);

    $.get("/evse/stop_debug")
        .fail(() => {
            status.value = __("evse.script.debug_stop_failed");
        })
        .done((result) => {
            status.value = __("evse.script.debug_stopped");
            $.get("/debug_report")
                .fail(() => {
                    status.value = __("evse.script.loading_debug_report_failed");
                })
                .done((result) => {
                    debug_log += "\n" + JSON.stringify(result) + "\n\n";

                    status.value = __("evse.script.loading_event_log");

                    $.get("/event_log")
                        .fail(() => {
                            status.value = __("evse.script.loading_event_log_failed");
                        })
                        .done((result) => {
                            debug_log += result + "\n";
                            status.value = __("evse.script.debug_done");

                            util.downloadToFile(debug_log, "evse-debug-log", "txt", "text/plain");
                        });
                });
        });
}

let status_plus_minus_timeout: number = null;

export function init() {
    $("#status_charging_current_minimum").on("click", () => set_charging_current(6000));
    $("#status_charging_current_maximum").on("click", () => set_charging_current(32000));
    $("#reset_current_configured").on("click", () => set_charging_current(32000));

    $('#reset_external_slot').on("click", () => {
        API.save('evse/external_defaults', {
                "current": 32000,
                "clear_on_disconnect": false
            },
            __("evse.script.reset_external_slot_failed"));


        API.save('evse/external_current',
            {"current": 32000},
            __("evse.script.reset_external_slot_failed"));

        API.save('evse/external_clear_on_disconnect',
            {"clear_on_disconnect": false},
            __("evse.script.reset_external_slot_failed"));
    });

    $("#status_stop_charging").on("click", stop_charging);
    $("#status_start_charging").on("click", start_charging);

    $('#status_auto_start_charging').on("change", () => set_auto_start_charging($('#status_auto_start_charging').prop('checked')));

    let input = $('#status_charging_current');

    $('#evse_status_charging_current_form').on('submit', function (this: HTMLFormElement, event: Event) {
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        set_charging_current(Math.round((input.val() as number) * 1000));
    });

    $('#user_calibration_upload').on("change",(evt: Event) => {
        let files = (evt.target as HTMLInputElement).files;
        if (files.length < 1) {
            return;
        }
        let reader = new FileReader();
        reader.onload = (event) => {
            API.save("evse/user_calibration",
                JSON.parse(event.target.result as string),
                __("evse.script.user_calibration_upload_failed")
            );
        };
        reader.readAsText(files[0]);
    });

    $("#user_calibration_reset").on("click", () => {
        API.save("evse/user_calibration",  {
            "user_calibration_active": false,
            "voltage_diff": 0,
            "voltage_mul": 0,
            "voltage_div": 0,
            "resistance_2700": 0,
            "resistance_880": [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        }, __("evse.script.user_calibration_upload_failed"));
    });


    $("#debug_start").on("click", debug_start);
    $("#debug_stop").on("click", debug_stop);

    allow_debug(true);

    $('#evse_charge_management').on("change", () => {
        let enable = $('#evse_charge_management').is(":checked");
        API.save('evse/management_enabled', {"enabled": enable}, __("evse.script.save_failed"));
    });

    $('#evse_user').on("change", () => {
        let enable = $('#evse_user').is(":checked");
        API.save('evse/user_enabled', {"enabled": enable}, __("evse.script.save_failed"));
    });

    $('#evse_external').on("change", () => {
        let enable = $('#evse_external').is(":checked");
        API.save('evse/external_enabled', {"enabled": enable}, __("evse.script.save_failed"));
    });

    $("#evse_reset").on("click", () => API.call('evse/reset', {}, ""));
    $("#evse_reflash").on("click", () => API.call('evse/reflash', {}, ""));

    $('#status_charging_current_minus').on("click", () => {
        let val: number = parseInt(input.val().toString());
        let target = (val % 1 === 0) ? (Math.floor(val) - 1) : Math.floor(val);

        if (target < $('#status_charging_current').prop("min"))
            return;

        if (status_plus_minus_timeout != null) {
            window.clearTimeout(status_plus_minus_timeout);
            status_plus_minus_timeout = null;
        }

        util.setNumericInput("status_charging_current", target, 3);

        status_plus_minus_timeout = window.setTimeout(() => {
            set_charging_current(target * 1000);
        }, 2000);
    });

    $('#status_charging_current_plus').on("click", () => {
        let val = parseFloat(input.val().toString());
        let target = Math.floor(val) + 1;

        if (target > $('#status_charging_current').prop("max"))
            return;

        if (status_plus_minus_timeout != null) {
            window.clearTimeout(status_plus_minus_timeout);
            status_plus_minus_timeout = null;
        }

        util.setNumericInput("status_charging_current", target, 3);

        status_plus_minus_timeout = window.setTimeout(() => {
            set_charging_current(target * 1000);
        }, 2000);
    });

    $('#status_charging_current').on("input", () => {
        status_charging_current_dirty = true;

        let val = parseFloat(input.val().toString());
        let target = val;

        if (target > parseInt($('#status_charging_current').prop("max"))) {
            return;
        }

        if (target < parseInt($('#status_charging_current').prop("min"))) {
            return;
        }

        if (status_plus_minus_timeout != null) {
            window.clearTimeout(status_plus_minus_timeout);
            status_plus_minus_timeout = null;
        }

        status_plus_minus_timeout = window.setTimeout(() => {
            // Use round here instead of float, as non-representable floats * 1000 create
            // confusing behaviour otherwise.
            // For example 8.123 (represented as 8.1229999...3 * 1000 is 8122.999...3, with floor results in 8122 instead of 8123.
            // This is only a problem here, as all other occurences only work on non-fractional numbers.
            set_charging_current(Math.round(target * 1000));
        }, 2000);
    });
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('evse/state', update_evse_state);
    source.addEventListener('evse/low_level_state', update_evse_low_level_state);
    source.addEventListener('evse/state', update_evse_low_level_state);
    source.addEventListener('evse/hardware_configuration', update_evse_hardware_configuration);
    source.addEventListener('evse/auto_start_charging', update_evse_auto_start_charging);
    source.addEventListener("evse/management_enabled", update_evse_managed);
    source.addEventListener("evse/user_enabled", update_evse_user);
    source.addEventListener("evse/external_enabled", update_evse_external);
    source.addEventListener("evse/slots", update_evse_slots);
    source.addEventListener("evse/user_calibration", update_evse_user_calibration);
    source.addEventListener("evse/state", update_evse_status_start_charging_button);
    source.addEventListener("evse/slots", update_evse_status_start_charging_button);
    source.addEventListener("evse/low_level_state", update_evse_status_start_charging_button);

    source.addEventListener("evse/debug_header", function (e) {
        debug_log += e.data + "\n";
    }, false);

    source.addEventListener("evse/debug", function (e) {
        debug_log += e.data + "\n";
    }, false);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-evse').prop('hidden', !module_init.evse);
    $('#status-evse').prop('hidden', !module_init.evse);
}
