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
import { ConfigPageHeader } from "../../ts/config_page_header"

render(<ConfigPageHeader page="ethernet" />, $('#ethernet_header')[0]);

declare function __(s: string): string;

function update_ethernet_config() {
    let config = API.default_updater('ethernet/config');

    if(config.ip == "0.0.0.0") {
        $('#ethernet_config_show_static').val("hide");
        ethernet_cfg_toggle_static_ip_collapse("hide");
    } else {
        $('#ethernet_config_show_static').val("show");
        ethernet_cfg_toggle_static_ip_collapse("show");
    }
}

function update_ethernet_state() {
    let state = API.default_updater('ethernet/state', ['ip', 'full_duplex', 'link_speed'], false);

    if (state.ip != "0.0.0.0") {
        $('#ethernet_state_ip').html(state.ip);
    } else {
        $('#ethernet_state_ip').html("");
    }
}

function ethernet_cfg_toggle_static_ip_collapse(value: string) {
    if (value == "hide") {
        $('#ethernet_static_ip_cfg').collapse('hide');
        $('#ethernet_config_ip').prop('required', false);
        $('#ethernet_config_subnet').prop('required', false);
        $('#ethernet_config_gateway').prop('required', false);
    }
    else if (value == "show") {
        $('#ethernet_static_ip_cfg').collapse('show');
        $('#ethernet_config_ip').prop('required', true);
        $('#ethernet_config_subnet').prop('required', true);
        $('#ethernet_config_gateway').prop('required', true);
    }
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('ethernet/state', update_ethernet_state);
    source.addEventListener('ethernet/config', update_ethernet_config);
}

export function init() {
    API.register_config_form('ethernet/config', () => {
            let dhcp = $('#ethernet_config_show_static').val() != "show";
            return {
                ip: dhcp ? "0.0.0.0" : $('#ethernet_config_ip').val().toString(),
                subnet: dhcp ? "0.0.0.0" : $('#ethernet_config_subnet').val().toString(),
                gateway: dhcp ? "0.0.0.0" : $('#ethernet_config_gateway').val().toString(),
                dns: dhcp ? "0.0.0.0" : $('#ethernet_config_dns').val().toString(),
                dns2: dhcp ? "0.0.0.0" : $('#ethernet_config_dns2').val().toString()
            };
        },
        undefined,
        __("ethernet.script.config_failed"),
        __("ethernet.script.reboot_content_changed")
    );

    // No => here: we want "this" to be the changed element
    $("#ethernet_config_show_static").on("change", function(this: HTMLInputElement) {ethernet_cfg_toggle_static_ip_collapse(this.value);});
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-ethernet').prop('hidden', !module_init.ethernet);
}
