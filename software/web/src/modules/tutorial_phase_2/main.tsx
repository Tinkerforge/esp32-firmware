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

import { h, render } from "preact";
import { PageHeader } from "../../ts/page_header"

render(<PageHeader page="tutorial_phase_2" />, $('#tutorial_phase_2_header')[0]);

function update_config()
{
    // Get current config from state "tutorial_phase_2/config" after receiving
    // a change from the backend
    let config = API.get("tutorial_phase_2/config");

    // Update HTML element with current color value
    $("#tutorial_phase_2_color").val(config.color);
}

export function init()
{
}

export function add_event_listeners(source: API.APIEventTarget)
{
    // Create event listener for state "tutorial_phase_2/config" to call the
    // update_config function if changes to that state are reported.
    source.addEventListener("tutorial_phase_2/config", update_config);
}

export function update_sidebar_state(module_init: any)
{
    $("#sidebar-tutorial-phase-2").prop("hidden", !module_init.tutorial_phase_2);
}
