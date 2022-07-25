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
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/page_header";

render(<PageHeader title={__("tutorial_phase_1.content.tutorial_phase_1")} />, $('#tutorial_phase_1_header')[0]);

export function init()
{
}

export function add_event_listeners(source: API.APIEventTarget)
{
}

export function update_sidebar_state(module_init: any)
{
    $("#sidebar-tutorial-phase-1").prop("hidden", !module_init.tutorial_phase_1);
}
