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

declare function __(s: string): string;


function update_mqtt_config() {
    let config = API.get('mqtt/config');

    // Remove the was-validated class to fix a visual bug
    // where saving the config triggers an update
    // that fills the elements, but clears the passphrase field.
    // An empty passphrase is invalid, so the input
    // field is marked as non-validated, confusing the user.
    let form = <HTMLFormElement>$('#mqtt_config_form')[0];
    form.classList.remove('was-validated');

    $('#mqtt_enable').prop("checked", config.enable_mqtt);
    $('#mqtt_broker_host').val(config.broker_host);
    $('#mqtt_broker_port').val(config.broker_port);
    $('#mqtt_broker_username').val(config.broker_username);
    // omit password
    $('#mqtt_topic_prefix').val(config.global_topic_prefix);
    $('#mqtt_client_name').val(config.client_name);
    $('#mqtt_interval').val(config.interval);
}

function save_mqtt_config() {
    API.save('mqtt/config',{
            enable_mqtt: $('#mqtt_enable').is(':checked'),
            broker_host: $('#mqtt_broker_host').val().toString(),
            broker_port: parseInt($('#mqtt_broker_port').val().toString(), 10),
            broker_username: $('#mqtt_broker_username').val().toString(),
            broker_password: util.passwordUpdate('#mqtt_broker_password'),
            global_topic_prefix: $('#mqtt_topic_prefix').val().toString(),
            client_name: $('#mqtt_client_name').val().toString(),
            interval: parseInt($('#mqtt_interval').val().toString(), 10)
        },
        __("mqtt.script.save_failed"),
        __("mqtt.script.reboot_content_changed"));
}

function update_mqtt_state() {
    let state = API.get('mqtt/state');

    util.update_button_group("btn_group_mqtt_state", state.connection_state);
    if(state.connection_state == 3) {
        $('#mqtt_status_error').html(" " + state.last_error);
    }
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('mqtt/config', update_mqtt_config);
    source.addEventListener('mqtt/state', update_mqtt_state);
}

export function init() {
    $("#mqtt_show_password").on("change", util.toggle_password_fn("#mqtt_broker_password"));
    $("#mqtt_clear_password").on("change", util.clear_password_fn("#mqtt_broker_password"));

    $('#mqtt_config_form').on('submit', function (this: HTMLFormElement, event: Event) {
        $('#mqtt_broker_host').prop("required", $('#mqtt_enable').is(':checked'));

        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        save_mqtt_config();
    });
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-mqtt').prop('hidden', !module_init.mqtt);
}
