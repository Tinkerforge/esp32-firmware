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

declare function __(s: string): string;

function update_ethernet_config() {
    let config = API.get('ethernet/config');

    // Remove the was-validated class to fix a visual bug
    // where saving the config triggers an update
    // that fills the elements, but clears the passphrase field.
    // An empty passphrase is invalid, so the input
    // field is marked as non-validated, confusing the user.
    let form = <HTMLFormElement>$('#ethernet_config_form')[0];
    form.classList.remove('was-validated');

    $('#ethernet_enable').prop("checked", config.enable_ethernet);

    if(config.ip == "0.0.0.0") {
        $('#ethernet_show_static').val("hide");
        ethernet_cfg_toggle_static_ip_collapse("hide");
    } else {
        $('#ethernet_show_static').val("show");
        ethernet_cfg_toggle_static_ip_collapse("show");
    }
    $('#ethernet_ip').val(config.ip);
    $('#ethernet_gateway').val(config.gateway);
    $('#ethernet_subnet').val(config.subnet);
    $('#ethernet_dns').val(config.dns);
    $('#ethernet_dns2').val(config.dns2);
}

function update_ethernet_state() {
    let state = API.get('ethernet/state');

    util.update_button_group("btn_group_ethernet_state", state.connection_state);

    if (state.ip != "0.0.0.0") {
        $('#status_ethernet_ip').html(state.ip);
    } else {
        $('#status_ethernet_ip').html("");
    }
}

function ethernet_cfg_toggle_static_ip_collapse(value: string) {
    if (value == "hide") {
        $('#ethernet_static_ip_cfg').collapse('hide');
        $('#ethernet_ip').prop('required', false);
        $('#ethernet_subnet').prop('required', false);
        $('#ethernet_gateway').prop('required', false);
    }
    else if (value == "show") {
        $('#ethernet_static_ip_cfg').collapse('show');
        $('#ethernet_ip').prop('required', true);
        $('#ethernet_subnet').prop('required', true);
        $('#ethernet_gateway').prop('required', true);
    }
}

function save_ethernet_config() {
    let dhcp = $('#ethernet_show_static').val() != "show";

    API.save('ethernet/config',{
            enable_ethernet: $('#ethernet_enable').is(':checked'),
            ip: dhcp ? "0.0.0.0" : $('#ethernet_ip').val().toString(),
            subnet: dhcp ? "0.0.0.0" : $('#ethernet_subnet').val().toString(),
            gateway: dhcp ? "0.0.0.0" : $('#ethernet_gateway').val().toString(),
            dns: dhcp ? "0.0.0.0" : $('#ethernet_dns').val().toString(),
            dns2: dhcp ? "0.0.0.0" : $('#ethernet_dns2').val().toString()
        },
        __("ethernet.script.config_failed"),
        __("ethernet.script.reboot_content_changed"));
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('ethernet/state', update_ethernet_state);
    source.addEventListener('ethernet/config', update_ethernet_config);
}

export function init() {
    // No => here: we want "this" to be the changed element
    $("#ethernet_show_static").on("change", function(this: HTMLInputElement) {ethernet_cfg_toggle_static_ip_collapse(this.value);});

    // Use bootstrap form validation
    $('#ethernet_config_form').on('submit', function (this: HTMLFormElement, event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }
        save_ethernet_config();
    });
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-ethernet').prop('hidden', !module_init.ethernet);
}
