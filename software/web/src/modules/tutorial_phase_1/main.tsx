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

import { h, Component } from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Box } from "react-feather";

export function TutorialPhase1Navbar() {
    return <NavbarItem name="tutorial_phase_1" module="tutorial_phase_1" title={__("tutorial_phase_1.navbar.tutorial_phase_1")} symbol={<Box />} />;
}

export class TutorialPhase1 extends Component {
    render() {
        return (
            <SubPage name="tutorial_phase_1">
                <PageHeader title={__("tutorial_phase_1.content.tutorial_phase_1")} />
            </SubPage>);
    }
}

export function init() {
}
