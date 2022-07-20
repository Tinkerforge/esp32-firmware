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
import { __ } from "../../ts/translation";
import { ConfigPageHeader } from "../../ts/config_page_header";

render(<ConfigPageHeader prefix="mqtt" title={__("mqtt.content.mqtt")} />, $('#mqtt_header')[0]);

function update_mqtt_state() {
    let state = API.default_updater('mqtt/state', ['last_error'], false);

    if(state.connection_state == 3) {
        $('#mqtt_status_error').html(" " + state.last_error);
    }
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('mqtt/config', () => API.default_updater('mqtt/config', ['broker_password']));
    source.addEventListener('mqtt/state', update_mqtt_state);
}

export function init() {
    $("#mqtt_config_show_broker_password").on("change", util.toggle_password_fn("#mqtt_config_broker_password"));
    $("#mqtt_config_clear_broker_password").on("change", util.clear_password_fn("#mqtt_config_broker_password"));

    API.register_config_form('mqtt/config', {
            overrides: () => ({
                broker_password: util.passwordUpdate('#mqtt_config_broker_password')
            }),
            pre_validation: () => {
                $('#mqtt_config_broker_host').prop("required", $('#mqtt_config_enable_mqtt').is(':checked'));
                return true;
            },
            error_string: __("mqtt.script.save_failed"),
            reboot_string: __("mqtt.script.reboot_content_changed")
        }
    );
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-mqtt').prop('hidden', !module_init.mqtt);
}
