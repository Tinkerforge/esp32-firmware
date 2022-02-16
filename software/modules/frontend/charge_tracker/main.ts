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

import $ from "../../../web/src/ts/jq";

import * as util from "../../../web/src/ts/util";

import * as API from "../../../web/src/ts/api";

import feather from "../../../web/src/ts/feather";

declare function __(s: string): string;

// Creates a date and time string that will be understood by Excel, Libreoffice, etc.
// (At least for de and en locales)
function timestamp_min_to_date(timestamp_minutes: number) {
    if (timestamp_minutes == 0) {
        return __('charge_tracker.script.unknown_charge_start');
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

    $('#charge_tracker_last_charges').html(charges.map((user) => {
            let display_name = __("charge_tracker.script.unknown_user")

            if (user.user_id != 0) {
                let filtered = users_config.users.filter(x => x.id == user.user_id);
                if (filtered.length == 1)
                    display_name = filtered[0].display_name
            }

            return `<div class="list-group-item">
            <div class="row">
                <div class="col">
                    <div class="mb-2"><span class="mr-1" data-feather="user"></span><span>${display_name}</span></div>
                    <div><span class="mr-1" data-feather="calendar"></span><span>${timestamp_min_to_date(user.timestamp_minutes)}</span></div>
                </div>
                <div class="col-auto">
                    <div class="mb-2"><span class="mr-1" data-feather="battery-charging"></span>${util.toLocaleFixed(user.energy_charged, 3)} kWh</div>
                    <div><span class="mr-1" data-feather="clock"></span>${util.format_timespan(user.charge_duration)}</div>
                </div>
            </div>
            </div>`
        }).join(""));
    feather.replace();
}

}

async function downloadChargeRecords() {
    let users: string[] = [];

    await fetch('/users/all_usernames')
        .then(response => response.arrayBuffer())
        .then(buffer => {
            if (buffer.byteLength != 256 * 32) {
                console.log("Unexpected length of all_usernames!");
                return;
            }

            const decoder = new TextDecoder();
            for(let i = 0; i < 256; ++i) {
                let view = new DataView(buffer, i * 32, 32);
                users.push(decoder.decode(view));
            }
        })
        .catch(err => console.log(err));

    fetch('/charge_record')
        .then(response => response.arrayBuffer())
        .then(buffer => {
            let result = __("charge_tracker.script.csv_header") + "\n";
            let users_config = API.get('users/config');

            for(let i = 0; i < buffer.byteLength; i += 16) {
                let view = new DataView(buffer, i, 16);

                let timestamp_minutes = view.getUint32(0, true);
                let meter_start = view.getFloat32(4, true);
                let user_id = view.getUint8(8);
                let charge_duration = view.getUint32(8, true) & 0x00FFFFFF;
                let meter_end = view.getFloat32(12, true);

                let filtered = users_config.users.filter(x => x.id == user_id);

                let display_name = "";
                if (user_id == 0)
                    display_name = __("charge_tracker.script.unknown_user");
                else if (filtered.length == 1)
                    display_name = filtered[0].display_name
                else
                    display_name = users[user_id];

                result += timestamp_minutes + "," + display_name + "," + (meter_end - meter_start).toFixed(3)  + "," + charge_duration + ",," + meter_start.toFixed(3) + "," + meter_end.toFixed(3) + "\n";
            }
            util.downloadToFile(result, "charge_records.csv", "text/csv");
        })
        .catch(err => console.log(err));
}

function update_charge_info() {
    let ci = API.get('users/charge_info');
    let evse_ll = API.get('evse/low_level_state');
    let mv = API.get('meter/values');
    let uc = API.get('users/config');

    $('#charge_tracker_current_charge').prop("hidden", ci.id == -1);

    if (ci.id == -1) {
        return;
    }

    let user_display_name = uc.users.filter((x) => x.id == ci.id)[0].display_name;
    let energy_charged = mv.energy_abs - ci.meter_start;
    let time_charging = evse_ll.uptime - ci.evse_uptime_start
    if (evse_ll.uptime < ci.evse_uptime_start)
        time_charging += 0xFFFFFFFF;

    time_charging = Math.floor(time_charging / 1000);
    let mean_power = energy_charged / time_charging * 3600;

    $('#users_status_charging_user').html(ci.id == 0 ? "unbekannter Nutzer" : user_display_name);
    $('#users_status_charging_time').html(util.format_timespan(time_charging));
    $('#users_status_charged_energy').html(util.toLocaleFixed(energy_charged, 3) + " kWh");
    $('#users_status_energy_rate').html(util.toLocaleFixed(mean_power, 3) + " kW");
    $('#users_status_charging_start').html(timestamp_min_to_date(ci.timestamp_minutes));
}

export function init() {
    $('#charge_tracker_download').on("click", downloadChargeRecords);
}

export function addEventListeners(source: API.ApiEventTarget) {
    source.addEventListener('charge_tracker/last_charges', update_last_charges);

    source.addEventListener('users/charge_info', update_charge_info);
    source.addEventListener('evse/low_level_state', update_charge_info);
    source.addEventListener('meter/values', update_charge_info);
    source.addEventListener('users/config', update_charge_info);
}

export function updateLockState(module_init: any) {
    $('#sidebar-charge_tracker').prop('hidden', !module_init.charge_tracker);
}
