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

function update_energy_manager_state() {
    let state = API.get('energy_manager/state');

    $('#state_contactor').val(state.contactor ? "Drei Phasen aktiv (Schütz geschaltet)" : "Eine Phase aktiv (Schütz nicht geschaltet)");
    $('#state_led_r').val(state.led_rgb[0]);
    $('#state_led_g').val(state.led_rgb[1]);
    $('#state_led_b').val(state.led_rgb[2]);
    util.update_button_group(`btn_group_gpio0`, state.gpio_input_state[0] ? 1 : 0);
    util.update_button_group(`btn_group_gpio1`, state.gpio_input_state[1] ? 1 : 0);
    util.update_button_group(`btn_group_gpio2`, state.gpio_output_state ? 1 : 0);
    $('#state_input_configuration_0').val(state.gpio_input_configuration[0]);
    $('#state_input_configuration_1').val(state.gpio_input_configuration[1]);
    $('#state_input_voltage').val(state.input_voltage);
    $('#state_contactor_check').val(state.contactor_check_state);

    update_energy_manager_html_visibility();
}

function update_energy_manager_config() {
    let config = API.get('energy_manager/config');
    console.log("update_energy_manager_config", config);

    $('#energy_manager_excess_charging_enable').prop('checked', config.excess_charging_enable);
    $('#energy_manager_phase_switching_config').val(config.phase_switching.toString()).trigger('change');
    $('#energy_manager_mains_power_reception').val(config.mains_power_reception);
    $('#energy_manager_minimum_charging').val(config.minimum_charging);
    $('#energy_manager_relay_config').val(config.relay_config.toString()).trigger('change');
    $('#energy_manager_relay_config_if').val(config.relay_config_if.toString()).trigger('change');
    $('#energy_manager_relay_config_is').val(config.relay_config_is.toString()).trigger('change');
    $('#energy_manager_relay_config_then').val(config.relay_config_then.toString()).trigger('change');
    $('#energy_manager_input3_config').val(config.input3_config.toString()).trigger('change');
    $('#energy_manager_input3_config_if').val(config.input3_config_if.toString()).trigger('change');
    $('#energy_manager_input3_config_then').val(config.input3_config_then.toString()).trigger('change');
    $('#energy_manager_input4_config').val(config.input4_config.toString()).trigger('change');
    $('#energy_manager_input4_config_if').val(config.input4_config_if.toString()).trigger('change');
    $('#energy_manager_input4_config_then').val(config.input4_config_then.toString()).trigger('change');
}

// Only show the relevant html elements, drop-down boxes and options
function update_energy_manager_html_visibility() {
    // Only updates the option elements if there are actual changes.
    // Otherwise we may overwrite the user-selected element
    function update_options(element: any, options: Array<{value: number, name: string}>) {
        let element_html = element.html();
        let found = true;
        for (let option of options) {
            if(!element_html.includes(__("energy_manager.content." + option.name))) {
                found = false;
                break;
            }
        }

        if(!found) {
            element.empty();
            let new_element_html = "";
            for (let option of options) {
                new_element_html += '<option value="' + option.value + '">' + __("energy_manager.content." + option.name) + '</option>';
            }
            element.html(new_element_html);
        }
    }

    // Update relay section
    if($('#energy_manager_relay_config').val()== '1') {
        $('#energy_manager_relay_rules').collapse('show');
    } else {
        $('#energy_manager_relay_rules').collapse('hide');
    }

    let relay_config_is_dd = $('#energy_manager_relay_config_is')
    let relay_config_if = $('#energy_manager_relay_config_if').val();
    if((relay_config_if == '0') || (relay_config_if == '1')) {
        update_options(relay_config_is_dd, [{"value": 0, name: "high"}, {"value": 1, name: "low"}]);
    } else if(relay_config_if == '2') {
        update_options(relay_config_is_dd, [{"value": 2, name: "1phase"}, {"value": 3, name: "3phase"}]);
    } else if(relay_config_if == '3') {
        update_options(relay_config_is_dd, [{"value": 4, name: "greater0"}, {"value": 5, name: "smaller0"}]);
    }

    // Update input section
    if($('#energy_manager_input3_config').val() == "1") {
        $('#energy_manager_input3_rules').collapse('show');
    } else {
        $('#energy_manager_input3_rules').collapse('hide');
    }

    if($('#energy_manager_input4_config').val() == "1") {
        $('#energy_manager_input4_rules').collapse('show');
    } else {
        $('#energy_manager_input4_rules').collapse('hide');
    }
}

function save_energy_manager_config() {
    console.log("save_energy_manager_config");

    let excess_charging_enable: boolean = $('#energy_manager_excess_charging_enable').is(":checked");
    let phase_switching: number         = Number($('#energy_manager_phase_switching_config').val());
    let mains_power_reception: number   = Number($('#energy_manager_mains_power_reception').val());
    let minimum_charging: number        = Number($('#energy_manager_minimum_charging').val());
    let relay_config: number            = Number($('#energy_manager_relay_config').val());
    let relay_config_if: number         = Number($('#energy_manager_relay_config_if').val());
    let relay_config_is: number         = Number($('#energy_manager_relay_config_is').val());
    let relay_config_then: number       = Number($('#energy_manager_relay_config_then').val());
    let input3_config: number           = Number($('#energy_manager_input3_config').val());
    let input3_config_if: number        = Number($('#energy_manager_input3_config_if').val());
    let input3_config_then: number      = Number($('#energy_manager_input3_config_then').val());
    let input4_config: number           = Number($('#energy_manager_input4_config').val());
    let input4_config_if: number        = Number($('#energy_manager_input4_config_if').val());
    let input4_config_then: number      = Number($('#energy_manager_input4_config_then').val());
    console.log(excess_charging_enable, phase_switching, mains_power_reception,minimum_charging, relay_config, relay_config_if, relay_config_is, relay_config_then,
                input3_config, input3_config_if, input3_config_then, input4_config, input4_config_if, input4_config_then);


    API.save('energy_manager/config', {
            excess_charging_enable: excess_charging_enable,
            phase_switching:        phase_switching,
            mains_power_reception:  mains_power_reception,
            minimum_charging:       minimum_charging,
            relay_config:           relay_config,
            relay_config_if:        relay_config_if,
            relay_config_is:        relay_config_is,
            relay_config_then:      relay_config_then,
            input3_config:          input3_config,
            input3_config_if:       input3_config_if,
            input3_config_then:     input3_config_then,
            input4_config:          input4_config,
            input4_config_if:       input4_config_if,
            input4_config_then:     input4_config_then
        },
        __("energy_manager.script.config_failed"),
        __("energy_manager.script.reboot_content_changed")
    );
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
            e.returnValue = <any>__("energy_manager.script.tab_close_warning");
        }
    } else {
        window.onbeforeunload = null;
    }
}

function debug_start() {
    debug_log = "";
    let status = <HTMLInputElement>$('#debug_label')[0];
    status.value = __("energy_manager.script.loading_debug_report");
    allow_debug(false);
    $.get("/debug_report")
        .fail(() => {
            status.value = __("energy_manager.script.loading_debug_report_failed");
            allow_debug(true);
        })
        .done((result) => {
            debug_log += JSON.stringify(result) + "\n\n";

            status.value = __("energy_manager.script.loading_event_log");

            $.get("/event_log")
                .fail(() => {
                    status.value = __("energy_manager.script.loading_event_log_failed");
                    allow_debug(true);
                })
                .done((result) => {
                    debug_log += result + "\n";

                    status.value = __("energy_manager.script.starting_debug");

                    $.get("/energy_manager/start_debug")
                        .fail(() => {
                            status.value = __("energy_manager.script.starting_debug_failed");
                            allow_debug(true);
                        })
                        .done((result) => {
                            status.value = __("energy_manager.script.debug_running");
                        });
                });
        });
}


function debug_stop() {
    let status = <HTMLInputElement>$('#debug_label')[0];

    allow_debug(true);

    $.get("/energy_manager/stop_debug")
        .fail(() => {
            status.value = __("energy_manager.script.debug_stop_failed");
        })
        .done((result) => {
            status.value = __("energy_manager.script.debug_stopped");
            $.get("/debug_report")
                .fail(() => {
                    status.value = __("energy_manager.script.loading_debug_report_failed");
                })
                .done((result) => {
                    debug_log += "\n" + JSON.stringify(result) + "\n\n";

                    status.value = __("energy_manager.script.loading_event_log");

                    $.get("/event_log")
                        .fail(() => {
                            status.value = __("energy_manager.script.loading_event_log_failed");
                        })
                        .done((result) => {
                            debug_log += result + "\n";
                            status.value = __("energy_manager.script.debug_done");

                            util.downloadToFile(debug_log, "energy_manager-debug-log", "txt", "text/plain");
                        });
                });
        });
}

export function init() {
    $("#debug_start").on("click", debug_start);
    $("#debug_stop").on("click", debug_stop);

    $("#energy_manager_relay_config, \
       #energy_manager_relay_config_if, \
       #energy_manager_input3_config, \
       #energy_manager_input4_config"
    ).on("change", update_energy_manager_html_visibility);

    // Use bootstrap form validation
    $('#energy_manager_config_form').on('submit', function (this: HTMLFormElement, event: Event) {
        console.log("energy_manager on submit");
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            console.log("energy_manager form not valid!");
            return;
        }
        save_energy_manager_config();
    });

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
