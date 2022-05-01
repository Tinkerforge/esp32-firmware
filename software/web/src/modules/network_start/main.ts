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

function update_network_config() {
    let config = API.get('network/config');
    $('#network_hostname').val(config.hostname);
    $('#network_enable_mdns').prop("checked", config.enable_mdns);
}

function save_network_config() {
    API.save('network/config', {
            hostname: $('#network_hostname').val().toString(),
            enable_mdns: $('#network_enable_mdns').is(':checked')
        },
        __("network.script.save_failed"),
        __("network.script.reboot_content_changed"));
}

export function init() {
    $('#network-group').on('hide.bs.collapse', () => $('#network-chevron').removeClass("rotated-chevron"));
    $('#network-group').on('show.bs.collapse', () => $('#network-chevron').addClass("rotated-chevron"));

    // Use bootstrap form validation
    $('#network_form').on('submit', function (this: HTMLFormElement, event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }
        save_network_config();
    });
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('network/config', update_network_config);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-network-group').prop('hidden', false);
    $('#sidebar-network').prop('hidden', false);
}
