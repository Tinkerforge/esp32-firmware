/* esp32-firmware
 * Copyright (C) 2025 Erik Fleckstein <erik@tinkerforge.com>
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

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, Fragment } from "preact";

export function DeviceInfo() {
    if (!util.render_allowed())
        return <div></div>

    // Can't use ifdef here: This is not a module.
    // Moving this into the device_name module would also not work
    // because then app.tsx.template would have to check for the device_name module.

    let name_and_type = <strong>Unknown name and firmware type:<br/>Device name module missing!<br/></strong>
    let uid = ""

    let i = API.get_unchecked("info/name");
    if (i !== null) {
        let display_name = API.get_unchecked("info/display_name")?.display_name
        name_and_type = <><strong>{display_name}</strong><br/>{i.display_type}<br/></>
        if (i.name != display_name)
            uid = " | " + i.uid;
    }
    let [version, timestamp] = API.get("info/version").firmware.split("+")

    return <div class="pt-2 mx-3 text-muted text-center" style="border-top: 1px rgba(0,0,0,.1) solid;">
        <small>
        {name_and_type}
        <strong>{version}</strong>{"+" + timestamp}{uid}
        </small>
    </div>;
}
