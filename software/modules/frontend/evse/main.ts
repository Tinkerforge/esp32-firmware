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

import $ from "../../../web/src/ts/jq";

import * as util from "../../../web/src/ts/util";

import feather from "../../../web/src/ts/feather";

declare function __(s: string): string;

function update_evse_state(state: EVSEState) {
    util.update_button_group("btn_group_iec_state", state.iec61851_state);

    util.update_button_group("btn_group_evse_state", state.vehicle_state);

    $('#status_start_charging').prop("disabled", state.vehicle_state != 1);
    $('#status_stop_charging').prop("disabled", state.vehicle_state != 2);

    let allowed_charging_current = util.toLocaleFixed(state.allowed_charging_current / 1000.0, 3) + " A";
    $('#allowed_charging_current').val(allowed_charging_current);

    util.update_button_group("btn_group_ac1", (state.contactor_state & 1) == 1 ? 1 : 0);
    util.update_button_group("btn_group_ac2", state.contactor_state > 1 ? 1 : 0);
    util.update_button_group("btn_group_contactor_error", state.contactor_error != 0 ? 1 : 0, state.contactor_error != 0 ? __("evse.script.error_code") + " " + state.contactor_error : null);
    util.update_button_group("btn_group_charge_release", state.charge_release);
    util.update_button_group("btn_group_error_state", state.error_state == 0 ? 0 : state.error_state - 1); // 1 is not a valid error state
    util.update_button_group("btn_group_lock_state", state.lock_state);

    $('#uptime').val(util.format_timespan(Math.floor(state.uptime / 1000)));
    $('#time_since_state_change').val(util.format_timespan(Math.floor(state.time_since_state_change / 1000)));

    if (state.iec61851_state == 2) {
        $("#evse_state_charging_text").html(__("evse.script.charging_for") + " ");
        $('#evse_state_charging_charge_time').html(util.format_timespan(Math.floor(state.time_since_state_change / 1000)));
    } else if ($('#evse_state_charging_charge_time').html() != "") {
        $("#evse_state_charging_text").html(__("evse.script.last_charge_took") + " ");
    } else {
        $("#evse_state_charging_text").html(__("evse.status.charging"));
    }
}

function update_evse_hardware_configuration(cfg: EVSEHardwareConfiguration) {
    util.update_button_group("btn_group_has_lock_switch", cfg.has_lock_switch ? 1 : 0);
    util.update_button_group("btn_group_jumper_config", cfg.jumper_configuration);
    $('#evse_row_lock_switch').prop('hidden', !cfg.has_lock_switch);
}


function update_evse_low_level_state(state: EVSELowLevelState) {
    util.update_button_group("btn_group_hardware_version", state.hardware_version - 14);
    util.update_button_group("btn_group_led_state", state.led_state);

    for(let i = 0; i < 5; ++i) {
        util.update_button_group(`btn_group_gpio${i}`, state.gpio[i] ? 1 : 0);
    }

    $('#pwm_duty_cycle').val(util.toLocaleFixed(state.cp_pwm_duty_cycle / 10, 1) + " %");

    $('#adc_value_0').val(state.adc_values[0]);
    $('#adc_value_1').val(state.adc_values[1]);

    $('#voltage_0').val(util.toLocaleFixed(state.voltages[0] / 1000.0, 3) + " V");
    $('#voltage_1').val(util.toLocaleFixed(state.voltages[1] / 1000.0, 3) + " V");
    $('#voltage_2').val(util.toLocaleFixed(state.voltages[2] / 1000.0, 3) + " V");

    $('#resistance_0').val(state.resistances[0] + " Ω");
    $('#resistance_1').val(state.resistances[1] + " Ω");
    $('#charging_time').val(util.format_timespan(Math.floor(state.charging_time / 1000)));
}


let last_max_charging_current: EVSEMaxChargingCurrent = null;
let last_managed: EVSEManaged = null;

function update_allowed_current_status() {
    let real_maximum = Math.min(last_max_charging_current.max_current_configured,
                                last_max_charging_current.max_current_incoming_cable,
                                last_max_charging_current.max_current_outgoing_cable);

    let managed = last_managed != null && last_managed.managed;
    if (managed) {
        real_maximum = Math.min(real_maximum, last_max_charging_current.max_current_managed);
    }

    let status_string = util.toLocaleFixed(real_maximum / 1000.0, 3) + " A " + __("evse.script.by") + " ";

    let status_list = [];
    if (real_maximum == last_max_charging_current.max_current_configured)
        status_list.push(__("evse.script.configuration"));
    if (real_maximum == last_max_charging_current.max_current_managed)
        status_list.push(__("evse.script.management"));
    if (real_maximum == last_max_charging_current.max_current_outgoing_cable)
        status_list.push(__("evse.script.outgoing"));
    if (real_maximum == last_max_charging_current.max_current_incoming_cable)
        status_list.push(__("evse.script.incoming"));

    status_string += status_list.join(", ");

    $('#evse_status_allowed_charging_current').val(status_string);
}

let status_charging_current_dirty = false;

function update_evse_max_charging_current(state: EVSEMaxChargingCurrent) {
    $('#max_current_configured').val(util.toLocaleFixed(state.max_current_configured / 1000.0, 3) + " A");
    $('#max_current_incoming_cable').val(util.toLocaleFixed(state.max_current_incoming_cable / 1000.0, 3) + " A");
    $('#max_current_outgoing_cable').val(util.toLocaleFixed(state.max_current_outgoing_cable / 1000.0, 3) + " A");
    $('#max_current_managed').val(util.toLocaleFixed(state.max_current_managed / 1000.0, 3) + " A");

    let theoretical_maximum = Math.min(state.max_current_incoming_cable, state.max_current_outgoing_cable);
    let theoretical_maximum_str = util.toLocaleFixed(theoretical_maximum / 1000.0, 0) + " A";
    $('#status_charging_current').prop("max", theoretical_maximum / 1000);
    $("#status_charging_current_maximum").on("click", () => set_charging_current(theoretical_maximum));
    $('#status_charging_current_maximum').html(theoretical_maximum_str);

    if(!status_charging_current_dirty) {
        let shown_current = Math.min(state.max_current_configured, theoretical_maximum);
        util.setNumericInput("status_charging_current", shown_current / 1000.0, 3);
    }

    last_max_charging_current = state;
    update_allowed_current_status();
}

function set_charging_current(current: number) {
    if (status_plus_minus_timeout != null) {
        window.clearTimeout(status_plus_minus_timeout);
        status_plus_minus_timeout = null;
    }

    status_charging_current_dirty = false;
    util.setNumericInput("status_charging_current", current / 1000, 3);

    $.ajax({
        url: '/evse/current_limit',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify({"current": current}),
        error: (xhr, status, error) => {
            util.add_alert("evse_set_charging_current_failed", "alert-danger", __("evse.script.set_charging_current_failed"), error + ": " + xhr.responseText);
        }
    });
}


function update_evse_auto_start_charging(x: EVSEAutoStart) {
    $('#status_auto_start_charging').prop("checked", x.auto_start_charging);
}

function set_auto_start_charging(auto_start_charging: boolean) {
    $.ajax({
        url: '/evse/auto_start_charging_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify({"auto_start_charging": auto_start_charging}),
        error: (xhr, status, error) => util.add_alert("evse_set_auto_start_charging_failed", "alert-danger",  __("evse.script.auto_start_charging_update"), error + ": " + xhr.responseText)
    });
}

function start_charging() {
    $.ajax({
        url: '/evse/start_charging',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(null),
        error: (xhr, status, error) => util.add_alert("evse_set_start_charging_failed", "alert-danger", __("evse.script.start_charging_failed"), error + ": " + xhr.responseText)
    });
}

function stop_charging() {
    $.ajax({
        url: '/evse/stop_charging',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(null),
        error: (xhr, status, error) => util.add_alert("evse_set_stop_charging_failed", "alert-danger",  __("evse.script.stop_charging_failed"), error + ": " + xhr.responseText)
    });
}


function update_evse_user_calibration(c: EVSEUserCalibration) {
    util.update_button_group("btn_group_user_calibration_enabled", c.user_calibration_active ? 1 : 0);
    $('#voltage_diff').val(c.voltage_diff);
    $('#voltage_mul').val(c.voltage_mul);
    $('#voltage_div').val(c.voltage_div);
    $('#resistance_2700').val(c.resistance_2700);
    $('#resistance_880').val(c.resistance_880.join(", "));
}


function update_evse_managed(m: EVSEManaged) {
    $('#evse_charge_management').prop("checked", m.managed);
    $('#evse_charging_current_managed_ignored').html(m.managed ? "" : __("evse.script.managed_current_ignored"));
    last_managed = m;
    update_allowed_current_status();
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
            e.returnValue = <any>__("evse.script.tab_close_warning");
        }
    } else {
        window.onbeforeunload = null;
    }
}

function debug_start() {
    debug_log = "";
    let status = <HTMLInputElement>$('#debug_label')[0];
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
    let status = <HTMLInputElement>$('#debug_label')[0];

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

                            util.downloadToFile(debug_log, "evse-debug-log-" + (new Date()).toISOString().replace(/:/gi, "-").replace(/\./gi, "-") + ".txt", "text/plain");
                        });
                });
        });
}

let status_plus_minus_timeout = null;

export function init() {
    $("#status_charging_current_minimum").on("click", () => set_charging_current(6000));
    $("#status_charging_current_maximum").on("click", () => set_charging_current(32000));
    $("#reset_current_configured").on("click", () => set_charging_current(32000));

    $("#status_stop_charging").on("click", stop_charging);
    $("#status_start_charging").on("click", start_charging);

    $('#status_auto_start_charging').on("change", () => set_auto_start_charging($('#status_auto_start_charging').prop('checked')));

    let input = $('#status_charging_current');
    let save_btn = $('#status_charging_current_save');
    input.on("input", () => {
        save_btn.html(feather.icons.save.toSvg());
        save_btn.prop("disabled", false);
    });

    $('#evse_status_charging_current_form').on('submit', function (this: HTMLFormElement, event: Event) {
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        set_charging_current(Math.round(<number>input.val() * 1000));
    });

    $('#user_calibration_upload').on("change",(evt: Event) => {
        let files = (<HTMLInputElement>evt.target).files;
        if (files.length < 1) {
            return;
        }
        let reader = new FileReader();
        reader.onload = (event) => {
            $.ajax({
                url: '/evse/user_calibration_update',
                method: 'PUT',
                contentType: 'application/json',
                data: event.target.result
            });
        };
        reader.readAsText(files[0]);
    });

    $("#user_calibration_reset").on("click", () => {
        let reset_calibration : EVSEUserCalibration = {
            "user_calibration_active": false,
            "voltage_diff": 0,
            "voltage_mul": 0,
            "voltage_div": 0,
            "resistance_2700": 0,
            "resistance_880": [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        };
        $.ajax({
            url: '/evse/user_calibration_update',
            method: 'PUT',
            contentType: 'application/json',
            data: JSON.stringify(reset_calibration)
        });
    });

    $("#debug_start").on("click", debug_start);
    $("#debug_stop").on("click", debug_stop);

    allow_debug(true);

    $('#evse_charge_management').on("change", () => {
        let enable = $('#evse_charge_management').is(":checked");
        $.ajax({
            url: '/evse/managed_update',
            method: 'PUT',
            contentType: 'application/json',
            data: JSON.stringify({"managed": enable, "password": enable ? 0x00363702 : 0x036370FF}),
            error: (xhr, status, error) => util.add_alert("evse_managed_update_failed", "alert-danger", __("evse.script.save_failed"), error + ": " + xhr.responseText)
        });
    });

    $("#evse_reset").on("click", () => $.ajax({
        url: '/evse/reset',
        method: 'PUT',
        contentType: 'application/json',
        data: "null"})
    );

    $("#evse_reflash").on("click", () => $.ajax({
        url: '/evse/reflash',
        method: 'PUT',
        contentType: 'application/json',
        data: "null"})
    );

    $('#status_charging_current_minus').on("click", () => {
        let val: number = input.val();
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
        let val: number = input.val();
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

        let val: number = input.val();
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

export function addEventListeners(source: API.ApiEventTarget) {
    source.addEventListener('evse/state', function (e: util.SSE) {
        update_evse_state(<EVSEState>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('evse/low_level_state', function (e: util.SSE) {
        update_evse_low_level_state(<EVSELowLevelState>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('evse/max_charging_current', function (e: util.SSE) {
        update_evse_max_charging_current(<EVSEMaxChargingCurrent>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('evse/hardware_configuration', function (e: util.SSE) {
        update_evse_hardware_configuration(<EVSEHardwareConfiguration>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('evse/auto_start_charging', function (e: util.SSE) {
        update_evse_auto_start_charging(<EVSEAutoStart>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('evse/user_calibration', function (e: util.SSE) {
        update_evse_user_calibration(<EVSEUserCalibration>(JSON.parse(e.data)));
    }, false);

    source.addEventListener("evse/managed", function (e: util.SSE) {
        update_evse_managed(<EVSEManaged>(JSON.parse(e.data)));
    }, false);

    source.addEventListener("evse/debug_header", function (e: util.SSE) {
        debug_log += e.data.slice(1, -1);
        if (meter_chunk.length > 0) {
            debug_log += ",power,energy_rel,energy_abs";
        }
        debug_log += "\n";
    }, false);

    source.addEventListener("evse/debug", function (e: util.SSE) {
        debug_log += e.data.slice(1, -1) + meter_chunk + "\n";
    }, false);

    source.addEventListener("meter/state", function (e: util.SSE) {
        let ms = <MeterState>(JSON.parse(e.data));
        meter_chunk = "," + ms.power + "," + ms.energy_rel + "," + ms.energy_abs;
    }, false);
}

export function updateLockState(module_init: any) {
    $('#sidebar-evse').prop('hidden', !module_init.evse);
    $('#status-evse').prop('hidden', !module_init.evse);
}
