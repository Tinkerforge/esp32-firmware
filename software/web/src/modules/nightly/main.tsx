/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

import { underconstruction_gif } from "./underconstruction_gif.embedded";

export function init() {
    let parent = document.querySelector('nav.navbar > a');
    if (parent == null)
        return;

    let target = document.createElement("img");
    (target.style as any) = "position: fixed; top:10px; left:0px; z-index: 10000; pointer-events: none;";
    target.src = underconstruction_gif;

    parent.after(target);
}
