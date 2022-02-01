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

interface Device {
    uid: string,
    port: string,
    name: string
}

function update_devices(devices: Device[]) {
    if (devices.length == 0) {
        $("#bricklets_content").html("<tr><td colspan=\"3\">" + __("proxy.script.no_bricklets") + "</td></tr>");
        return;
    }

    let result = "";

    for (let device of devices) {
        let line = "<tr>";
        line += "<td>" + device.port + "</td>";
        line += "<td>" + device.uid + "</td>";
        line += "<td>" + (device.name == "unknown device" ? __("proxy.script.unknown_device") : device.name) + "</td>";
        line += "</tr>"

        result += line;
    }

    $("#bricklets_content").html(result);
}

interface ErrorCounter {
    SpiTfpChecksum: number,
    SpiTfpFrame: number,
    TfpFrame: number,
    TfpUnexpected: number
}

function update_error_counters(error_counters: {[index:string]: ErrorCounter}) {
    if (Object.keys(error_counters).length == 0) {
        $("#bricklets_error_counters").html("");
        return;
    }

    let result = "";

    for (let port in error_counters) {
        let counters = error_counters[port];

        let line = "<tr>";
        line += "<td>" + port + "</td>";
        line += "<td>" + counters.SpiTfpChecksum + "</td>";
        line += "<td>" + counters.SpiTfpFrame + "</td>";
        line += "<td>" + counters.TfpFrame + "</td>";
        line += "<td>" + counters.TfpUnexpected + "</td>";
        line += "</tr>"

        result += line;
    }

    $("#bricklets_error_counters").html(result);
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('proxy/devices', function (e: util.SSE) {
        update_devices(<Device[]>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('proxy/error_counters', function (e: util.SSE) {
        update_error_counters(JSON.parse(e.data));
    }, false);
}

export function init() {

}

export function updateLockState(module_init: any) {
    $('#sidebar-bricklets').prop('hidden', !module_init.proxy);
}
