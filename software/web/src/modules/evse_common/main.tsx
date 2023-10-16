/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

import { h, render } from "preact";

import { EVSE } from "./evse_content";
import { EVSESettings } from "./evse_settings";
import { EVSEStatus } from "./evse_status";

render(<EVSE />, $('#evse')[0]);

render(<EVSESettings/>, $('#evse-settings')[0]);

render(<EVSEStatus />, $("#status-evse")[0]);

export function init(){}
export function add_event_listeners(){}
export function update_sidebar_state(module_init: any) {
    $('#sidebar-evse').prop('hidden', !module_init.evse_v2 && !module_init.evse);
    $('#sidebar-evse-settings').prop('hidden', !module_init.evse_v2 && !module_init.evse);
    $('#status-evse').prop('hidden', !module_init.evse_v2 && !module_init.evse);
}
