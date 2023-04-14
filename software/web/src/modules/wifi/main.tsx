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

import * as API from "../../ts/api";

import { h, render } from "preact";
import { __ } from "../../ts/translation";

import { WifiAP } from "./wifi_ap";
import { WifiSTA } from "./wifi_sta";

render(<WifiAP/>, $('#wifi-ap')[0])
render(<WifiSTA/>, $('#wifi-sta')[0])

function wifi_symbol(rssi: number) {
    if(rssi >= -60)
        return `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: ${rssi}</title><path d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>`;
    if(rssi >= -70)
        return `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: ${rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>`;
    if(rssi >= -80)
        return `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: ${rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path stroke="#cccccc" d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>`;

    return `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: ${rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path stroke="#cccccc" d="M5 12.55a11 11 0 0 1 14.08 0"></path><path stroke="#cccccc" d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>`;
}

function update_wifi_state() {
    let state = API.default_updater('wifi/state', ['sta_ip', 'sta_rssi', 'sta_bssid', 'ap_bssid', 'connection_start', 'connection_end'], false);

    if (state.sta_ip != "0.0.0.0") {
        $('#wifi_state_sta_ip').html(state.sta_ip);
        $('#wifi_state_sta_rssi').html(wifi_symbol(state.sta_rssi));
    } else {
        $('#wifi_state_sta_ip').html("");
        $('#wifi_state_sta_rssi').html("");
    }
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('wifi/state', update_wifi_state);
    source.addEventListener('wifi/sta_config', () => $('#status-wifi-sta').prop("hidden", !API.get("wifi/sta_config").enable_sta));
    source.addEventListener('wifi/ap_config', () => $('#status-wifi-ap').prop("hidden", !API.get("wifi/ap_config").enable_ap));
}

export function init() {

}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-wifi-sta').prop('hidden', !module_init.wifi);
    $('#sidebar-wifi-ap').prop('hidden', !module_init.wifi);
}
