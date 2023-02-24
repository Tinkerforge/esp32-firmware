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

import * as API from "../../ts/api";
import * as util from "../../ts/util";

import { h, render, Fragment } from "preact";
import { __ } from "../../ts/translation";

import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputFloat } from "../../ts/components/input_float";

type Co2Config = API.getType['co2/config'];

export class Co2 extends ConfigComponent<'co2/config'> {
    constructor() {
        super('co2/config',
              __("co2.script.save_failed"),
              __("co2.script.reboot_content_changed"));
    }

    render(props: {}, state: Readonly<Co2Config>) {
        if (!util.allow_render)
            return <></>

        return (
            <>
                <ConfigForm id="co2_config_form" title={__("co2.content.co2")} onSave={() => this.save()} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("co2.content.temperature_offset")} label_muted={__("co2.content.temperature_offset_muted")}>
                        <InputFloat
                            unit="°C"
                            value={state.temperature_offset}
                            onValue={this.set("temperature_offset")}
                            digits={2}
                            min={-4000}
                            max={12000}
                            />
                    </FormRow>
                </ConfigForm>
            </>
        );
    }
}

render(<Co2/>, $('#co2ampel')[0])

function update_co2_state() {
    let state = API.default_updater('co2/state', ['temperature', 'humidity'], false);
    util.setNumericInput('co2_state_temperature', state.temperature / 100, 2);
    util.setNumericInput('co2_state_humidity', state.humidity / 100, 2);
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('co2/state', update_co2_state);
}

export function init() {
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-co2ampel').prop('hidden', !module_init.co2ampel);
}
