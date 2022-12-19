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

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render } from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import { ConfigPageHeader } from "../../ts/components/config_page_header";

render(<ConfigPageHeader prefix="energy_manager" title={__("energy_manager.content.energy_manager")} />, $('#energy_manager_header')[0]);

function update_energy_manager_state() {
    let state = API.get('energy_manager/state');

    $('#state_contactor').val(translate_unchecked(state.contactor ?  'energy_manager.content.three_phases_active' : 'energy_manager.content.one_phase_active'));
    $('#state_led_r').val(state.led_rgb[0]);
    $('#state_led_g').val(state.led_rgb[1]);
    $('#state_led_b').val(state.led_rgb[2]);
    util.update_button_group(`btn_group_gpio0`, state.gpio_input_state[0] ? 1 : 0);
    util.update_button_group(`btn_group_gpio1`, state.gpio_input_state[1] ? 1 : 0);
    util.update_button_group(`btn_group_gpio2`, state.gpio_output_state ? 1 : 0);
    $('#state_input_voltage').val(`${state.input_voltage} mV`);
    $('#state_contactor_check').val(state.contactor_check_state);
}

function update_energy_manager_config() {
    let config = API.default_updater('energy_manager/config', ['maximum_power_from_grid', 'maximum_available_current', 'minimum_current']);

    util.setNumericInput("energy_manager_config_maximum_power_from_grid", config.maximum_power_from_grid / 1000, 3);
    util.setNumericInput("energy_manager_config_maximum_available_current", config.maximum_available_current / 1000, 3);
    util.setNumericInput("energy_manager_config_minimum_current", config.minimum_current / 1000, 3);
}

// Only show the relevant html elements, drop-down boxes and options
function update_energy_manager_html_visibility() {
    // Only updates the option elements if there are actual changes.
    // Otherwise we may overwrite the user-selected element
    function update_options(element: any, options: Array<{value: number, name: string}>) {
        let element_html = element.html();
        let found = true;
        for (let option of options) {
            if(element_html.indexOf(translate_unchecked(`energy_manager.content.${option.name}`)) < 0) {
                found = false;
                break;
            }
        }

        if(!found) {
            element.empty();
            let new_element_html = "";
            for (let option of options) {
                new_element_html += '<option value="' + option.value + '">' + translate_unchecked(`energy_manager.content.${option.name}`) + '</option>';
            }
            element.html(new_element_html);
        }
    }

    // Update contactor section
    let phase_switching_config_is_dd = $('#energy_manager_config_phase_switching_mode')
    if ($('#energy_manager_config_contactor_installed').is(':checked')) {
        update_options(phase_switching_config_is_dd, [{"value": 0, name: "automatic"}, {"value": 1, name: "always_one_phase"}, {"value": 3, name: "always_three_phases"}]);
    } else {
        update_options(phase_switching_config_is_dd, [{"value": 1, name: "fixed_one_phase"}, {"value": 3, name: "fixed_three_phases"}]);
    }

    // Update relay section
    if($('#energy_manager_config_relay_config').val()== '1') {
        $('#energy_manager_config_relay_rules').collapse('show');
    } else {
        $('#energy_manager_config_relay_rules').collapse('hide');
    }

    let relay_config_is_dd = $('#energy_manager_config_relay_config_is')
    let relay_config_if = $('#energy_manager_config_relay_config_if').val();
    if((relay_config_if == '0') || (relay_config_if == '1')) {
        update_options(relay_config_is_dd, [{"value": 0, name: "high"}, {"value": 1, name: "low"}]);
    } else if(relay_config_if == '2') {
        update_options(relay_config_is_dd, [{"value": 2, name: "one_phase"}, {"value": 3, name: "three_phase"}]);
    } else if(relay_config_if == '3') {
        update_options(relay_config_is_dd, [{"value": 4, name: "greater0"}, {"value": 5, name: "smaller0"}]);
    }

    // Update input section
    if($('#energy_manager_config_input3_config').val() == "1") {
        $('#energy_manager_config_input3_rules').collapse('show');
    } else {
        $('#energy_manager_config_input3_rules').collapse('hide');
    }

    if($('#energy_manager_config_input4_config').val() == "1") {
        $('#energy_manager_config_input4_rules').collapse('show');
    } else {
        $('#energy_manager_config_input4_rules').collapse('hide');
    }
}

let debug_log = "";

function allow_debug(b: boolean) {
    $('#debug_start').prop("disabled", !b);
    $('#debug_stop').prop("disabled", b);
    if (!b) {
        window.onbeforeunload = (e: Event) => {
            e.preventDefault();
            // returnValue is not a boolean, but the string to be shown
            // in the "are you sure you want to close this tab" message
            // box. However this string is only shown in some browsers.
            e.returnValue = __("energy_manager.script.tab_close_warning") as any;
        }
    } else {
        window.onbeforeunload = null;
    }
}

async function get_debug_report_and_event_log(status: HTMLInputElement) {
    status.value = __("energy_manager.script.loading_debug_report");

    try {
        debug_log += await util.download("/debug_report").then(blob => blob.text());
        debug_log += "\n\n";
    } catch {
        status.value = __("energy_manager.script.loading_debug_report_failed");
        allow_debug(true);
        return false;
    }

    status.value = __("energy_manager.script.loading_event_log");

    try {
        debug_log += await util.download("/event_log").then(blob => blob.text());
        debug_log += "\n";
    } catch {
        status.value = __("energy_manager.script.loading_event_log_failed");
        allow_debug(true);
        return false;
    }

    return true;
}

async function debug_start() {
    debug_log = "";
    let status = $('#debug_label')[0] as HTMLInputElement;
    allow_debug(false);

    if (!await get_debug_report_and_event_log(status))
        return;

    status.value = __("energy_manager.script.starting_debug");

    try {
        await util.download("/energy_manager/start_debug");
    } catch {
        status.value = __("energy_manager.script.starting_debug_failed");
        allow_debug(true);
        return;
    }

    status.value = __("energy_manager.script.debug_running");
}

async function debug_stop() {
    let status = $('#debug_label')[0] as HTMLInputElement;

    allow_debug(true);

    try {
        await util.download("/energy_manager/stop_debug");
    } catch {
        status.value = __("energy_manager.script.debug_stop_failed");
        return;
    }

    debug_log += "\n\n";

    status.value = __("energy_manager.script.debug_stopped");

    if (!await get_debug_report_and_event_log(status))
        return;

    status.value = __("energy_manager.script.debug_done");

    util.downloadToFile(debug_log, "energy_manager-debug-log", "txt", "text/plain");
}

export function init() {
    $("#debug_start").on("click", debug_start);
    $("#debug_stop").on("click", debug_stop);

    API.register_config_form('energy_manager/config', {
            overrides: () => ({
                maximum_power_from_grid: Math.round(($('#energy_manager_config_maximum_power_from_grid').val() as number) * 1000),
                maximum_available_current: Math.round(($('#energy_manager_config_maximum_available_current').val() as number) * 1000),
                minimum_current: Math.round(($('#energy_manager_config_minimum_current').val() as number) * 1000)
            }),
            error_string: __("energy_manager.script.config_failed"),
            reboot_string: __("energy_manager.script.reboot_content_changed")
        });

    $("#energy_manager_config_contactor_installed, \
       #energy_manager_config_relay_config, \
       #energy_manager_config_relay_config_if, \
       #energy_manager_config_input3_config, \
       #energy_manager_config_input4_config"
    ).on("change", update_energy_manager_html_visibility);

    allow_debug(true);
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('energy_manager/state', update_energy_manager_state);
    source.addEventListener('energy_manager/config', update_energy_manager_config);

    source.addEventListener("energy_manager/debug_header", function (e) {
        debug_log += e.data + "\n";
    }, false);

    source.addEventListener("energy_manager/debug", function (e) {
        debug_log += e.data + "\n";
    }, false);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-energy-manager').prop('hidden', !module_init.energy_manager);
}
