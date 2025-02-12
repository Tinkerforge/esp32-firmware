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

import { h } from "preact";

export function DeviceInfo() {
    if (!util.render_allowed())
        return <div></div>

    let i = API.get("info/name");
    let display_name = API.get("info/display_name").display_name
    let dev_name_changed = i.name != display_name;
    let [version, timestamp] = API.get("info/version").firmware.split("+")

    return <div class="pt-2 mx-3 text-muted text-center" style="border-top: 1px rgba(0,0,0,.1) solid;">
        <small>
        <strong>{display_name}</strong><br/>
        {i.display_type}<br/>
        <strong>{version}</strong>{"+" + timestamp}{dev_name_changed ? (" | " + i.uid) : ""}
        </small>
    </div>;
}
