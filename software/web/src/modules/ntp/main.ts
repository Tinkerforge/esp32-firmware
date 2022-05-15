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

import timezones from "./timezones";

declare function __(s: string): string;

function update_timezone(timezone: string) {
    let splt = timezone.split("/");
    $('#ntp_timezone_area').val(splt[0]);
    $('#ntp_timezone_area').trigger('change');

    $('#ntp_timezone_location').val(splt[1]);
    $('#ntp_timezone_location').trigger('change');

    if (splt.length == 3) {
        $('#ntp_timezone_sublocation').val(splt[2]);
    }
}

function update_config(force: boolean = false) {
    if (!force && !$('#ntp_save_button').prop("disabled"))
        return;

    let cfg = API.get('ntp/config');
    $('#ntp_enabled').prop("checked", cfg.enable);
    $('#ntp_use_dhcp').prop("checked", cfg.use_dhcp);
    $('#ntp_server').val(cfg.server);
    $('#ntp_server2').val(cfg.server2);
    update_timezone(cfg.timezone);
}

function update_state() {

}

function save_config() {
    let timezone = $('#ntp_timezone_area').val() + "/" + $('#ntp_timezone_location').val() + ($('#ntp_timezone_sublocation').prop("hidden") ? "" : ("/" + $('#ntp_timezone_sublocation').val()));

    API.save("ntp/config", {
                enable: $('#ntp_enabled').prop("checked"),
                use_dhcp: $('#ntp_use_dhcp').prop("checked"),
                timezone: timezone,
                server: $('#ntp_server').val().toString(),
                server2: $('#ntp_server2').val().toString()
            },
            __("ntp.script.save_failed"),
            __("ntp.script.reboot_content_changed"))
        .then(() => $('#ntp_save_button').prop("disabled", true));
}

export function init() {
    $('#ntp_browser_timezone').on('click', () => {
        update_timezone(Intl.DateTimeFormat().resolvedOptions().timeZone);
        $('#ntp_config_form').trigger('input');
    });

    $('#ntp_config_form').on('input', () => $('#ntp_save_button').prop("disabled", false));

    $('#ntp_timezone_area').on('change', () => {
        let area = <string>$('#ntp_timezone_area').val();
        let options = Object.keys(timezones[area]).sort().map((x) => `<option value="${x}">${x}</option>`).join("");
        $('#ntp_timezone_location').empty().append(options);
        $('#ntp_timezone_location').trigger('change');
    });

    $('#ntp_timezone_location').on('change', () => {
        let area = <string>$('#ntp_timezone_area').val();
        let loc = <string>$('#ntp_timezone_location').val();
        let sub_loc = timezones[area][loc];
        if (sub_loc === null) {
            $('#ntp_timezone_sublocation').prop("hidden", true);
            $('#ntp_timezone_location').addClass("rounded-right");
        } else {
            let options = Object.keys(sub_loc).sort().map((x) => `<option value="${x}">${x}</option>`).join("");
            $('#ntp_timezone_sublocation').empty().append(options);
            $('#ntp_timezone_sublocation').prop("hidden", false);
            $('#ntp_timezone_location').removeClass("rounded-right");
        }
    });

    let options = Object.keys(timezones).sort().map((x) => `<option value="${x}">${x}</option>`).join("");
    $('#ntp_timezone_area').empty().append(options);
    $('#ntp_timezone_area').trigger('change');

    $('#ntp_config_form').on('submit', function (this: HTMLFormElement, event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        save_config();
    });
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('ntp/state', update_state);
    source.addEventListener('ntp/config', () => update_config());
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-ntp').prop('hidden', !module_init.ntp);
}
