/* esp32-firmware
 * Copyright (C) 2022 Olaf Lüke <olaf@tinkerforge.com>
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
import { h, ComponentChildren } from "preact";
import { NavbarGroup } from "../../ts/components/navbar_group";
import { Sliders } from "react-feather";

export function EnergyManagerGroupNavbar(props: {children: ComponentChildren}) {
    return (
        <NavbarGroup name="energy_manager" title={__("energy_manager_group.navbar.energy_manager_group")} symbol={<Sliders />} hidden={false}>
            {props.children}
        </NavbarGroup>);
}

export function init() {
}
