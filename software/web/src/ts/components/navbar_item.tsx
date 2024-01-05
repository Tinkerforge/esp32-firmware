/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

interface NavbarItemProps {
    name: string;
    title: string;
    symbol: h.JSX.Element;
    hidden?: boolean; // default: true
}

export function NavbarItem(props: NavbarItemProps) {
    return (
        <li class="nav-item">
            <a id={`sidebar-${props.name}`} class="nav-link" data-toggle="tab" role="tab" aria-controls={props.name} aria-selected="true" href={`#${props.name}`} hidden={props.hidden === undefined ? true : props.hidden}>
                {props.symbol}
                <span style="margin-left: 8px;">{props.title}</span>
            </a>
        </li>
    );
}
