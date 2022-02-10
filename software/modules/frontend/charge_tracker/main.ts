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

declare function __(s: string): string;

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

            return `<li class="list-group-item">${user.timestamp_minutes + ": " + user.energy_charged + " kWh geladen von " + display_name + " in " + util.format_timespan(user.charge_duration)}</li>`
        }).join(""));
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

export function init() {
    $('#charge_tracker_download').on("click", downloadChargeRecords);
}

export function addEventListeners(source: API.ApiEventTarget) {
    source.addEventListener('charge_tracker/last_charges', update_last_charges);
}

export function updateLockState(module_init: any) {
    $('#sidebar-charge_tracker').prop('hidden', !module_init.charge_tracker);
}
