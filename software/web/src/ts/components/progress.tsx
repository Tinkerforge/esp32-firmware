/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

import { h, ComponentChildren } from "preact";

interface ProgressProps {
    class?: string;
    progress: number; // [0..1]
}

export function Progress(props: ProgressProps) {
    return (
        <div class={"form-progress form-control " + (props.class !== undefined ? props.class : "")} style="font-size: 1rem; padding: 0;">
            <div class="progress-bar progress-bar-no-transition"
                role="progressbar" style={"padding: 0; min-width: 2rem; width: " + (props.progress * 100) + "%"} aria-valuenow={props.progress * 100} aria-valuemin={0}
                aria-valuemax={100}>{Math.round(props.progress * 100) + "%"}</div>
        </div>
    )
}
