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

export function TutorialPhase5Navbar() {
    return <NavbarItem name="tutorial_phase_5" module="tutorial_phase_5" title={__("tutorial_phase_5.navbar.tutorial_phase_5")} symbol={<Box />} />;
}

interface TutorialPhase5State {
    color: string;
    button: boolean;
}

export class TutorialPhase5 extends Component<{}, TutorialPhase5State> {
    constructor() {
        super();

        this.state = {
            color: '#000000',
            button: null,
        } as any;

        // Create event listener for state "tutorial_phase_5/config" to
        // receive changes to that state
        util.addApiEventListener('tutorial_phase_5/config', () => {
            // Get current config from state "tutorial_phase_5/config" after
            // receiving a change from the backend
            let config = API.get("tutorial_phase_5/config");

            // Update HTML element with current color value
            this.setState({color: config.color});
        });

        // Create event listener for state "tutorial_phase_5/state" to
        // receive changes to that state
        util.addApiEventListener('tutorial_phase_5/state', () => {
            // Get current state from state "tutorial_phase_5/state" after
            // receiving a change from the backend
            let state = API.get("tutorial_phase_5/state");

            // Update HTML element with current button value
            this.setState({button: state.button});
        });
    }

    render() {
        return (
            <SubPage name="tutorial_phase_5">
                <PageHeader title={__("tutorial_phase_5.content.tutorial_phase_5")} />

                <FormRow label={__("tutorial_phase_5.content.color")}>
                    <input class="form-control" type="color" value={this.state.color} onChange={(event) => {
                        // Get current color value from the HTML element and create new config
                        let config = {color: (event.target as HTMLInputElement).value.toString()};

                        // Send new config to backend as state "tutorial_phase_5/config"
                        API.save("tutorial_phase_5/config", config, () => __("tutorial_phase_5.script.save_config_failed"));
                    }} />
                </FormRow>

                <FormRow label={__("tutorial_phase_5.content.button")}>
                    <input class="form-control" value={
                        this.state.button === null
                            ? __("tutorial_phase_5.script.button_unknown")
                            : this.state.button
                              ? __("tutorial_phase_5.script.button_pressed")
                              : __("tutorial_phase_5.script.button_released")
                    } readonly />
                </FormRow>
            </SubPage>);
    }
}

export function init() {
}
