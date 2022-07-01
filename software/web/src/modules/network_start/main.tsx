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
import { ConfigPageHeader } from "../../ts/config_page_header"

render(<ConfigPageHeader page="network" />, $('#network_header')[0]);

declare function __(s: string): string;

export function init() {
    $('#network-group').on('hide.bs.collapse', () => $('#network-chevron').removeClass("rotated-chevron"));
    $('#network-group').on('show.bs.collapse', () => $('#network-chevron').addClass("rotated-chevron"));

    API.register_config_form('network/config',
        undefined,
        undefined,
        __("network.script.save_failed"),
        __("network.script.reboot_content_changed"));
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('network/config', () => API.default_updater('network/config'));
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-network-group').prop('hidden', false);
    $('#sidebar-network').prop('hidden', false);
}
