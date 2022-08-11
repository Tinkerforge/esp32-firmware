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
import { __ } from "../../ts/translation";
import { ConfigPageHeader } from "../../ts/config_page_header";

render(<ConfigPageHeader prefix="ntp" title={__("ntp.content.ntp")} />, $('#ntp_header')[0]);

import timezones from "./timezones";

function update_timezone(timezone: string) {
    let splt = timezone.split("/");
    $('#ntp_config_timezone_area').val(splt[0]);
    $('#ntp_config_timezone_area').trigger('change');

    $('#ntp_config_timezone_location').val(splt[1]);
    $('#ntp_config_timezone_location').trigger('change');

    if (splt.length == 3) {
        $('#ntp_config_timezone_sublocation').val(splt[2]);
    }
}

function update_config() {
    let cfg = API.default_updater('ntp/config', ['timezone']);
    update_timezone(cfg.timezone);
}

function update_state() {
    let state = API.get('ntp/state');
    $('#ntp_state_time').html(util.timestamp_min_to_date(state.time, ""));
    util.update_button_group('ntp_state_synced_group', !API.get('ntp/config').enable ? 0 : (state.synced ? 2 : 1))
}

export function init() {
    $('#ntp_browser_timezone').on('click', () => {
        update_timezone(Intl.DateTimeFormat().resolvedOptions().timeZone);
        $('#ntp_config_form').trigger('input');
    });

    $('#ntp_config_timezone_area').on('change', () => {
        let area = $('#ntp_config_timezone_area').val() as string;
        let options = Object.keys(timezones[area]).sort().map((x) => `<option value="${x}">${x}</option>`).join("");
        $('#ntp_config_timezone_location').empty().append(options);
        $('#ntp_config_timezone_location').trigger('change');
    });

    $('#ntp_config_timezone_location').on('change', () => {
        let area = $('#ntp_config_timezone_area').val() as string;
        let loc = $('#ntp_config_timezone_location').val() as string;
        let sub_loc = timezones[area][loc];
        if (sub_loc === null) {
            $('#ntp_config_timezone_sublocation').prop("hidden", true);
            $('#ntp_config_timezone_location').addClass("rounded-right");
        } else {
            let options = Object.keys(sub_loc).sort().map((x) => `<option value="${x}">${x}</option>`).join("");
            $('#ntp_config_timezone_sublocation').empty().append(options);
            $('#ntp_config_timezone_sublocation').prop("hidden", false);
            $('#ntp_config_timezone_location').removeClass("rounded-right");
        }
    });

    let options = Object.keys(timezones).sort().map((x) => `<option value="${x}">${x}</option>`).join("");
    $('#ntp_config_timezone_area').empty().append(options);
    $('#ntp_config_timezone_area').trigger('change');

    API.register_config_form('ntp/config', {
            overrides: () => {
                    let timezone = $('#ntp_config_timezone_area').val() +
                                    "/" + $('#ntp_config_timezone_location').val() +
                                    ($('#ntp_config_timezone_sublocation').prop("hidden") ? "" : ("/" + $('#ntp_config_timezone_sublocation').val()));
                    return {
                        timezone: timezone
                    };
                },
            error_string: __("ntp.script.save_failed"),
            reboot_string: __("ntp.script.reboot_content_changed")
        }
    );
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('ntp/config', update_config);
    source.addEventListener('ntp/state', update_state);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-ntp').prop('hidden', !module_init.ntp);
}
