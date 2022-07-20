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

import { h, render } from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/page_header";

render(<PageHeader title={__("proxy.content.proxy")} />, $('#proxy_header')[0]);

function update_devices() {
    let devices = API.get('proxy/devices');

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


function update_error_counters() {
    let error_counters = API.get('proxy/error_counters');

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

export function init() {

}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('proxy/devices', update_devices);
    source.addEventListener('proxy/error_counters', update_error_counters);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-proxy').prop('hidden', !module_init.proxy);
}
