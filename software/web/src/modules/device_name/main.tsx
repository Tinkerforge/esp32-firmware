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

import $ from "../../ts/jq";

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { __ } from "../../ts/translation";

import { h, render, Fragment, } from "preact";

import { InputText } from "../../ts/components/input_text";
import { Button } from "react-bootstrap";
import { Save } from "react-feather";
import { ConfigComponent } from "src/ts/components/config_component";


export class DeviceName extends ConfigComponent<'info/display_name'> {
    constructor() {
        super('info/display_name', __("device_name.script.config_failed"));
    }

    render(props: {}, state: Readonly<API.getType['info/display_name']>) {
        if (!util.render_allowed() || !API.hasModule("device_name"))
            return <></>

        document.title = API.get("info/display_name").display_name + " - " + __("main.title");

        return (
            <div class="col-12 col-xl-8">
                <div class="row pt-3 border-bottom tab-header-shadow">
                    <h1 class="col-4 col-xl-6 page-header pb-2">{__("device_name.status.status")}</h1>
                    <form class="col-8 col-xl-6" onSubmit={(e: Event) => {
                        e.preventDefault();
                        e.stopPropagation();
                        if (!(e.target as HTMLFormElement).checkValidity())
                            return;

                        API.save("info/display_name", state, __("device_name.script.config_failed"));
                    }}>
                        <InputText maxLength={32} value={state.display_name} onValue={(v) => this.setState({display_name: v})} required>
                            {state.display_name == API.get('info/display_name').display_name ? <></> :
                                <div class="input-group-append">
                                    <Button className="form-control rounded-right" type="submit"><Save/></Button>
                                </div>
                            }
                        </InputText>
                    </form>
                </div>
            </div>
        )
    }
}

render(<DeviceName/>, $('#status-device_name')[0]);

export function add_event_listeners(source: API.APIEventTarget) {
}

export function init() {
}

export function update_sidebar_state(module_init: any) {

}
