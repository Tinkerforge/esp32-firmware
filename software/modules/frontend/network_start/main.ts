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

import $ from "jquery";

import * as util from "../util";

declare function __(s: string): string;

interface NetworkConfig {
    hostname: string,
    enable_mdns: boolean,
}

function update_network_config(config: NetworkConfig) {
    $('#network_hostname').val(config.hostname);
    $('#network_enable_mdns').prop("checked", config.enable_mdns);
}

function save_network_config() {
    let payload: NetworkConfig = {
        hostname: $('#network_hostname').val().toString(),
        enable_mdns: $('#network_enable_mdns').is(':checked')
    };

    $.ajax({
        url: '/network/config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: util.getShowRebootModalFn(__("network.script.reboot_content_changed")),
        error: (xhr, status, error) => util.add_alert("network_config_update_failed", "alert-danger", __("network.script.save_failed"), error + ": " + xhr.responseText)
    });
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

export function addEventListeners(source: EventSource) {
    source.addEventListener('network/config', function (e: util.SSE) {
        update_network_config(<NetworkConfig>(JSON.parse(e.data)));
    }, false);
}

export function updateLockState(module_init: any) {
    $('#sidebar-network-group').prop('hidden',  false);
    $('#sidebar-network').prop('hidden',  false);

}
