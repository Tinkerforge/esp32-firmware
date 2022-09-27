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

render(<PageHeader title={__("tutorial_phase_5.content.tutorial_phase_5")} />, $('#tutorial_phase_5_header')[0]);

function update_config()
{
    // Get current config from state "tutorial_phase_5/config" after receiving
    // a change from the backend
    let config = API.get("tutorial_phase_5/config");

    // Update HTML element with current color value
    $("#tutorial_phase_5_color").val(config.color);
}

function save_config()
{
    // Get current color value from the HTML element and create new config
    let config = {"color": $("#tutorial_phase_5_color").val().toString()}

    // Send new config to backend as state "tutorial_phase_5/config"
    API.save("tutorial_phase_5/config", config, __("tutorial_phase_5.script.save_config_failed"));
}

function update_state()
{
    // Get current state from state "tutorial_phase_5/state" after receiving
    // a change from the backend
    let state = API.get("tutorial_phase_5/state");

    // Update HTML element with current button value
    $("#tutorial_phase_5_button").val(state.button ? __("tutorial_phase_5.script.button_pressed") : __("tutorial_phase_5.script.button_released"));
}

export function init()
{
    // Attach the save_config function to the change event of the HTML
    // element to be able to send color changes to the backend.
    $("#tutorial_phase_5_color").on("change", save_config);
}

export function add_event_listeners(source: API.APIEventTarget)
{
    // Create event listener for state "tutorial_phase_5/config" to call the
    // update_config function if changes to that state are reported.
    source.addEventListener("tutorial_phase_5/config", update_config);

    // Create event listener for state "tutorial_phase_5/state" to call the
    // update_state function if changes to that state are reported.
    source.addEventListener("tutorial_phase_5/state", update_state);
}

export function update_sidebar_state(module_init: any)
{
    $("#sidebar-tutorial-phase-5").prop("hidden", !module_init.tutorial_phase_5);
}
