/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

import { __ } from "../../ts/translation";
import { h, ComponentChildren, RefObject } from "preact";
import { NavbarGroup } from "../../ts/components/navbar_group";

export function EVSEGroupNavbar(props: {children: ComponentChildren, group_ref: RefObject<NavbarGroup>}) {
    return (
        <NavbarGroup ref={props.group_ref} title={__("evse_group.navbar.evse_group")} symbol={
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="currentColor" stroke="none" class="feather feather-type2"><path d="M23 10.846c0 6.022-4.925 10.904-11 10.904S1 16.868 1 10.846c0-1.506.308-2.94.864-4.244C2.143 5.95 2.88 4.75 2.88 4.75h18.243s.736 1.2 1.014 1.852c.556 1.304.864 2.738.864 4.244z" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/><circle cx="9" cy="8.5" r="1.5"/><circle cx="15" cy="8.5" r="1.5"/><circle cx="9" cy="16.75" r="2"/><circle cx="15" cy="16.75" r="2"/><circle cx="6" cy="12" r="2"/><circle cx="12" cy="12" r="2"/><circle cx="18" cy="12" r="2"/></svg>
            } hidden={false}>
            {props.children}
        </NavbarGroup>);
}

export function init() {
}
