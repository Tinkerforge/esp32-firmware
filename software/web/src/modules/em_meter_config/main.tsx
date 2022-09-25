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
import { __ } from "../../ts/translation";
import { ConfigPageHeader } from "../../ts/components/config_page_header";

render(<ConfigPageHeader prefix="energy_manager_meter" title={__("em_meter_config.content.em_meter_config")} />, $('#em_meter_config_header')[0]);

function update_em_meter_config() {
    API.default_updater('energy_manager/meter_config');
}

function update_em_meter_config_state() {
    let state = API.get('energy_manager/state');
    util.update_button_group("btn_group_em_meter_config_meter_available", Math.max(state.energy_meter_type - 1, 0));
    $('#em_meter_config_meter_power').val(util.toLocaleFixed(state.energy_meter_power, 0) + " W");
    $('#em_meter_config_meter_energy_rel').val(util.toLocaleFixed(state.energy_meter_energy_rel, 3) + " kWh");
    $('#em_meter_config_meter_energy_abs').val(util.toLocaleFixed(state.energy_meter_energy_abs, 3) + " kWh");
    $('#em-meter-config-sdm-details').prop('hidden', state.energy_meter_type == 0);
}

// Only show the relevant html elements, drop-down boxes and options
function update_em_meter_config_html_collapse(value: string) {
    let state: { [id: string]: "show" | "hide" } = {
        '#em-meter-config-sdm': 'hide',
        '#em-meter-config-sunspec': 'hide',
        '#em-meter-config-modbus-tcp': 'hide'
    }

    if (value == "1") {
        state['#em-meter-config-sdm'] = 'show';
    }
    else if (value == "2") {
        state['#em-meter-config-sunspec'] = 'show';
    }
    else if (value == "3") {
        state['#em-meter-config-modbus-tcp'] = 'show';
    }

    for (let key in state) {
        $(key).collapse(state[key])
    }
}

export function init() {
    API.register_config_form('energy_manager/meter_config', {
            error_string: __("em_meter_config.script.config_failed"),
            reboot_string: __("em_meter_config.script.reboot_content_changed")
        });

    $("#energy_manager_meter_config_meter_type").on("change", function(this: HTMLInputElement) {update_em_meter_config_html_collapse(this.value);});
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('energy_manager/state', update_em_meter_config_state);
    source.addEventListener('energy_manager/meter_config', update_em_meter_config);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-em-meter-config').prop('hidden', !module_init.energy_manager);
}
