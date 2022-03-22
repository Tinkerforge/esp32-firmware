/* esp32-firmware
 * Copyright (C) 2022 Matthias Bolte <matthias@tinkerforge.com>
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

function update_tutorial_state() {
    let state = API.get('tutorial/state');

    $('#tutorial_color').val(state.color);
}

export function init() {

}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('tutorial/state', update_tutorial_state);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-tutorial').prop('hidden', !module_init.tutorial);
}
