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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { h, Component } from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";
import { FormRow } from "../../ts/components/form_row";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Box } from "react-feather";

export function TutorialPhase3Navbar() {
    return <NavbarItem name="tutorial_phase_3" module="tutorial_phase_3" title={__("tutorial_phase_3.navbar.tutorial_phase_3")} symbol={<Box />} />;
}

interface TutorialPhase3State {
    color: string;
}

export class TutorialPhase3 extends Component<{}, TutorialPhase3State> {
    constructor() {
        super();

        this.state = {
            color: '#000000',
        } as any;

        // Create event listener for state "tutorial_phase_3/config" to
        // receive changes to that state
        util.addApiEventListener('tutorial_phase_3/config', () => {
            // Get current config from state "tutorial_phase_3/config" after
            // receiving a change from the backend
            let config = API.get("tutorial_phase_3/config");

            // Update HTML element with current color value
            this.setState({color: config.color});
        });
    }

    render() {
        return (
            <SubPage name="tutorial_phase_3">
                <PageHeader title={__("tutorial_phase_3.content.tutorial_phase_3")} />

                <FormRow label={__("tutorial_phase_3.content.color")}>
                    <input class="form-control" type="color" value={this.state.color} onChange={(event) => {
                        // Get current color value from the HTML element and create new config
                        let config = {color: (event.target as HTMLInputElement).value.toString()};

                        // Send new config to backend as state "tutorial_phase_3/config"
                        API.save("tutorial_phase_3/config", config, __("tutorial_phase_3.script.save_config_failed"));
                    }} />
                </FormRow>
            </SubPage>);
    }
}

export function init() {
}
