/* esp32-firmware
 * Copyright (C) 2022 Olaf Lüke <olaf@tinkerforge.com>
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
import { ConfigPageHeader } from "../../ts/config_page_header"

render(<ConfigPageHeader page="em_meter_config" />, $('#em_meter_config_header')[0]);

declare function __(s: string): string;

function update_em_meter_config() {
    let config = API.get('energy_manager/meter_config');
    $('#em-meter-config-type').val(config.meter_type.toString()).trigger('change');

    update_em_meter_config_html_visibility();
    console.log("meter_type", config);
}

function update_em_meter_config_state() {
    let state = API.get('energy_manager/state');
    util.update_button_group("btn_group_meter_available", state.energy_meter_type == 0 ? 0 : 1);
    $('#energy_manager_meter_power').val(util.toLocaleFixed(state.energy_meter_power, 0) + " W");
    $('#energy_manager_meter_energy_rel').val(util.toLocaleFixed(state.energy_meter_energy_rel, 3) + " kWh");
    $('#energy_manager_meter_energy_abs').val(util.toLocaleFixed(state.energy_meter_energy_abs, 3) + " kWh");
}

// Only show the relevant html elements, drop-down boxes and options
function update_em_meter_config_html_visibility() {
    let meter_type = $('#em-meter-config-type').val();

    $('#em-meter-config-sdm630').collapse('hide')
    $('#em-meter-config-sunspec').collapse('hide')
    $('#em-meter-config-modbus-tcp').collapse('hide')
    if (meter_type == "1") {
        $('#em-meter-config-sdm630').collapse('show')
    }
    else if (meter_type == "2") {
        $('#em-meter-config-sunspec').collapse('show')
    }
    else if (meter_type == "3") {
        $('#em-meter-config-modbus-tcp').collapse('show')
    }

    if (meter_type == "1") {
        $('#em-meter-config-sdm630-details').collapse('show')
    } else {
        $('#em-meter-config-sdm630-details').collapse('hide')    
    }
}

function save_em_meter_config() {
    console.log("save_energy_manager_meter_config");

    let meter_type = Number($('#em-meter-config-type').val());
    API.save('energy_manager/meter_config', {
            meter_type: meter_type
        },
        __("em_meter_config.script.config_failed"),
        __("em_meter_config.script.reboot_content_changed")
    );
}

export function init() {
    console.log("em-meter-config init");
    $("#em-meter-config-type").on("change", update_em_meter_config_html_visibility);

    // Use bootstrap form validation
    $('#em_meter_config_config_form').on('submit', function (this: HTMLFormElement, event: Event) {
        console.log("em_meter_config_config_form on submit");
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }
        save_em_meter_config();
    });
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('energy_manager/state', update_em_meter_config_state);
    source.addEventListener('energy_manager/meter_config', update_em_meter_config);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-em-meter-config').prop('hidden', !module_init.energy_manager);
}
