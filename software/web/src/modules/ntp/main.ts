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

function update_config() {
    let cfg = API.get('ntp/config');
    $('#ntp_enabled').prop("checked", cfg.enable);
    $('#ntp_use_dhcp').prop("checked", cfg.use_dhcp);
    $('#ntp_timezone').val(cfg.timezone);
    $('#ntp_server').val(cfg.server);
}

function update_state() {

}

function save_config() {
    API.save("ntp/config", {
        enable: $('#ntp_enabled').prop("checked"),
        use_dhcp: $('#ntp_use_dhcp').prop("checked"),
        timezone: $('#ntp_timezone').val().toString(),
        server: $('#ntp_server').val().toString()
    },
    __("ntp.script.save_failed"),
    __("ntp.script.reboot_content_changed"));
}

export function addEventListeners(source: API.ApiEventTarget) {
    source.addEventListener('ntp/state', update_state);
    source.addEventListener('ntp/config', update_config);
}

export function init() {
    $('#ntp_form').on('submit', function (this: HTMLFormElement, event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        save_config();
    });
}

export function updateLockState(module_init: any) {
    $('#sidebar-ntp').prop('hidden', !module_init.ntp);
}
