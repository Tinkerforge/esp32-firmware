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

import * as API from "../../ts/api";

import { h, render } from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";

render(<PageHeader title={__("tutorial_phase_4.content.tutorial_phase_4")} />, $('#tutorial_phase_4_header')[0]);

function update_config()
{
    // Get current config from state "tutorial_phase_4/config" after receiving
    // a change from the backend
    let config = API.get("tutorial_phase_4/config");

    // Update HTML element with current color value
    $("#tutorial_phase_4_color").val(config.color);
}

function save_config()
{
    // Get current color value from the HTML element and create new config
    let config = {"color": $("#tutorial_phase_4_color").val().toString()}

    // Send new config to backend as state "tutorial_phase_4/config"
    API.save("tutorial_phase_4/config", config, __("tutorial_phase_4.script.save_config_failed"));
}

export function init()
{
    // Attach the save_config function to the change event of the HTML
    // element to be able to send color changes to the backend.
    $("#tutorial_phase_4_color").on("change", save_config);
}

export function add_event_listeners(source: API.APIEventTarget)
{
    // Create event listener for state "tutorial_phase_4/config" to call the
    // update_config function if changes to that state are reported.
    source.addEventListener("tutorial_phase_4/config", update_config);
}

export function update_sidebar_state(module_init: any)
{
    $("#sidebar-tutorial-phase-4").prop("hidden", !module_init.tutorial_phase_4);
}
