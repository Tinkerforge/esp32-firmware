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

export function init() {
    $('#system-group').on('hide.bs.collapse', () => $('#system-chevron').removeClass("rotated-chevron"));
    $('#system-group').on('show.bs.collapse', () => $('#system-chevron').addClass("rotated-chevron"));
}

export function add_event_listeners(source: API.APIEventTarget) {

}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-system-group").prop("hidden", false);
}
