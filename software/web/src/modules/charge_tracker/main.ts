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

import feather from "../../ts/feather";

declare function __(s: string): string;

// Creates a date and time string that will be understood by Excel, Libreoffice, etc.
// (At least for de and en locales)
function timestamp_min_to_date(timestamp_minutes: number) {
    if (timestamp_minutes == 0) {
        return __("charge_tracker.script.unknown_charge_start");
    }
    let date_fmt: any = { year: 'numeric', month: '2-digit', day: '2-digit'};
    let time_fmt: any = {hour: '2-digit', minute:'2-digit' };
    let fmt = Object.assign({}, date_fmt, time_fmt);

    let date = new Date(timestamp_minutes * 60000);
    let result = date.toLocaleString([], fmt);

    let date_result = date.toLocaleDateString([], date_fmt);
    let time_result = date.toLocaleTimeString([], time_fmt);

    // By default there is a comma between the date and time part of the string.
    // This comma (even if the whole date is marked as string for CSV) prevents office programs
    // to understand that this is a date.
    // Remove this (and only this) comma without assuming anything about the localized string.
    if (result == date_result + ", " + time_result) {
        return date_result + " " + time_result;
    }
    if (result == time_result + ", " + date_result) {
        return time_result + " " + date_result;
    }

    return result;
}

function update_last_charges() {
    let charges = API.get('charge_tracker/last_charges');
    let users_config = API.get('users/config');

    let last_charges_html = charges.map((user) => {
        let display_name = __("charge_tracker.script.unknown_user")

        if (user.user_id != 0) {
            display_name = __("charge_tracker.script.deleted_user")
            let filtered = users_config.users.filter(x => x.id == user.user_id);
            if (filtered.length == 1)
                display_name = filtered[0].display_name
        }

        return `<div class="list-group-item">
        <div class="row">
            <div class="col">
                <div class="mb-2"><span class="mr-1" data-feather="user"></span><span style="vertical-align: middle;">${display_name}</span></div>
                <div><span class="mr-1" data-feather="calendar"></span><span style="vertical-align: middle;">${timestamp_min_to_date(user.timestamp_minutes)}</span></div>
            </div>
            <div class="col-auto">
                <div class="mb-2"><span class="mr-1" data-feather="battery-charging"></span><span style="vertical-align: middle;">${user.energy_charged === null ? "N/A" : util.toLocaleFixed(user.energy_charged, 3)} kWh</span></div>
                <div><span class="mr-1" data-feather="clock"></span><span style="vertical-align: middle;">${util.format_timespan(user.charge_duration)}</span></div>
            </div>
        </div>
        </div>`
    }).reverse();

    $('#charge_tracker_status_last_charges').html(last_charges_html.slice(0, 3).join(""));
    $('#charge_tracker_last_charges').html(last_charges_html.join(""))
    feather.replace();
}

function update_state() {
    let state = API.get('charge_tracker/state');
    $('#charge_tracker_tracked_charges').val(state.tracked_charges);
    $('#charge_tracker_first_charge_timestamp').val(timestamp_min_to_date(state.first_charge_timestamp));
}

function to_csv_line(vals: string[]) {
    let line = vals.map(entry => '"' + entry.replace(/\"/, '""') + '"');

    return line.join(",") + "\r\n";
}

export function getAllUsernames() {
    return fetch('/users/all_usernames')
        .then(response => response.arrayBuffer())
        .then(buffer => {
            let usernames: string[] = [];
            let display_names: string[] = [];

            if (buffer.byteLength != 256 * 64) {
                console.log("Unexpected length of all_usernames!");
                return [null, null];
            }

            const decoder = new TextDecoder();
            for(let i = 0; i < 256; ++i) {
                let view = new DataView(buffer, i * 64, 32);
                let username = decoder.decode(view).replace(/\0/g, "");

                view = new DataView(buffer, i * 64 + 32, 32);
                let display_name = decoder.decode(view).replace(/\0/g, "");

                usernames.push(username);
                display_names.push(display_name);
            }
            return [usernames, display_names];
        });
}

async function downloadChargeLog() {
    const [usernames, display_names] = await getAllUsernames()
        .catch(err => {
            util.add_alert("download-charge-log", "danger", __("charge_tracker.script.download_charge_log_failed"), err);
            return [null, null];
        });

    if (usernames == null || display_names == null)
        return;

    await fetch('/charge_tracker/charge_log')
        .then(response => response.arrayBuffer())
        .then(buffer => {
            let line = [
                __("charge_tracker.script.csv_header_start"),
                __("charge_tracker.script.csv_header_display_name"),
                __("charge_tracker.script.csv_header_energy"),
                __("charge_tracker.script.csv_header_duration"),
                "",
                __("charge_tracker.script.csv_header_meter_start"),
                __("charge_tracker.script.csv_header_meter_end"),
                __("charge_tracker.script.csv_header_username"),
            ];

            let header = to_csv_line(line);
            let result = header;
            let users_config = API.get('users/config');

            let start_s = $('#charge_tracker_start_date_filter').val().toString();
            let end_s = $('#charge_tracker_end_date_filter').val().toString();

            let start_date = start_s == "" ? new Date(0) : new Date(start_s);
            let start = start_date.getTime() / 1000 / 60;

            let end_date = end_s == "" ? new Date(Date.now()) : new Date(end_s);
            end_date.setHours(23, 59, 59, 999);
            let end = end_date.getTime() / 1000 / 60;

            let known_users = API.get('users/config').users.filter(u => u.id != 0).map(u => u.id);

            let user_filter = parseInt($('#charge_tracker_user_filter').val().toString());
            let user_filtered = (x: number) => {
                switch(user_filter) {
                    case -2:
                        return false;
                    case -1:
                        return !known_users.includes(x);
                    default:
                        return x != user_filter;
                }
            }

            if (start <= end) {
                for(let i = 0; i < buffer.byteLength; i += 16) {
                    let view = new DataView(buffer, i, 16);

                    let timestamp_minutes = view.getUint32(0, true);
                    let meter_start = view.getFloat32(4, true);
                    let user_id = view.getUint8(8);
                    let charge_duration = view.getUint32(9, true) & 0x00FFFFFF;
                    let meter_end = view.getFloat32(12, true);

                    if (timestamp_minutes != 0 && timestamp_minutes < start) {
                        result = header;
                        continue;
                    }

                    if (timestamp_minutes != 0 && timestamp_minutes > end)
                        break;

                    if (user_filtered(user_id))
                        continue;

                    let filtered = users_config.users.filter(x => x.id == user_id);

                    let display_name = "";
                    let username = ""
                    if (user_id == 0) {
                        display_name = __("charge_tracker.script.unknown_user");
                        username = __("charge_tracker.script.unknown_user");
                    }
                    else if (filtered.length == 1) {
                        display_name = filtered[0].display_name
                        username = filtered[0].username
                    }
                    else {
                        display_name = display_names[user_id];
                        username = usernames[user_id];
                    }

                    let charged = (Number.isNaN(meter_start) || Number.isNaN(meter_end)) ? NaN : (meter_end - meter_start);
                    let charged_string;
                    if (charged == NaN || charged < 0) {
                        charged_string = 'N/A';
                    } else {
                        charged_string = util.toLocaleFixed(charged, 3);
                    }

                    let line = [
                        timestamp_min_to_date(timestamp_minutes),
                        display_name,
                        charged_string,
                        charge_duration.toString(),
                        "",
                        Number.isNaN(meter_start) ? 'N/A' : util.toLocaleFixed(meter_start, 3),
                        Number.isNaN(meter_end) ? 'N/A' : util.toLocaleFixed(meter_end, 3),
                        username
                    ];

                    result += to_csv_line(line);
                }
            }

            let t = (new Date()).toISOString().replace(/:/gi, "-").replace(/\./gi, "-");
            util.downloadToFile(result, "charge-log", "csv", "text/csv; charset=utf-8; header=present");
        })
        .catch(err => util.add_alert("download-charge-log", "alert-danger", __("charge_tracker.script.download_charge_log_failed"), err));
}

function update_current_charge() {
    let cc = API.get('charge_tracker/current_charge');
    let evse_ll = API.get('evse/low_level_state');
    let mv = API.get('meter/values');
    let uc = API.get('users/config');

    $('#charge_tracker_current_charge').prop("hidden", cc.user_id == -1);

    if (cc.user_id == -1) {
        return;
    }

    let filtered = uc.users.filter((x) => x.id == cc.user_id);
    let user_display_name = __("charge_tracker.script.unknown_user");
    if (filtered.length > 0 && cc.user_id != 0)
        user_display_name = filtered[0].display_name;

    let energy_charged = mv.energy_abs - cc.meter_start;
    let time_charging = evse_ll.uptime - cc.evse_uptime_start
    if (evse_ll.uptime < cc.evse_uptime_start)
        time_charging += 0xFFFFFFFF;

    time_charging = Math.floor(time_charging / 1000);

    $('#users_status_charging_user').html(cc.user_id == 0 ? __("charge_tracker.script.unknown_user") : user_display_name);
    $('#users_status_charging_time').html(util.format_timespan(time_charging));
    $('#users_status_charged_energy').html(cc.meter_start == null ? "N/A" : util.toLocaleFixed(energy_charged, 3) + " kWh");
    $('#users_status_charging_start').html(timestamp_min_to_date(cc.timestamp_minutes));
}

function update_user_filter_dropdown() {
    let uc = API.get('users/config');

    let options = uc.users.map((x) => `<option value=${x.id}>${x.id == 0 ? __("charge_tracker.script.unknown_users") : x.display_name}</option>`);
    options.unshift(`<option value=-2>${__("charge_tracker.script.all_users")}</option>`, `<option value=-1>${__("charge_tracker.script.deleted_users")}</option>`);
    $('#charge_tracker_user_filter').empty().append(options.join(""));
}

export function init() {
    $('#charge_tracker_download').on("click", () =>{
        $('#charge_tracker_download_spinner').prop("hidden", false);
        downloadChargeLog().finally(() => $('#charge_tracker_download_spinner').prop("hidden", true));
    });

    $('#charge_tracker_remove').on("click", () => $('#charge_tracker_remove_modal').modal('show'));

    $('#charge_tracker_remove_confirm').on("click", () =>
        API.call('charge_tracker/remove_all_charges', {
            "do_i_know_what_i_am_doing": true
        }, __("charge_tracker.script.remove_failed"))
        .then(() => {
            util.postReboot(__("charge_tracker.script.remove_init"), __("util.reboot_text"));
        })
        .finally(() => {
            $('#charge_tracker_remove_modal').modal('hide');
        })
    );
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('charge_tracker/last_charges', update_last_charges);
    source.addEventListener('charge_tracker/current_charge', update_current_charge);
    source.addEventListener('evse/low_level_state', update_current_charge);
    source.addEventListener('meter/values', update_current_charge);
    source.addEventListener('users/config', update_current_charge);
    source.addEventListener('users/config', update_user_filter_dropdown);
    source.addEventListener('charge_tracker/state', update_state);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-charge_tracker').prop('hidden', !module_init.charge_tracker);
}
