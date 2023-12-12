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
import { h, render, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputFloat } from "../../ts/components/input_float";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { OutputFloat } from "../../ts/components/output_float";
import { SubPage } from "../../ts/components/sub_page";

type Co2AmpelConfig = API.getType["co2ampel/config"];

export class Co2Ampel extends ConfigComponent<"co2ampel/config"> {
    constructor() {
        super('co2ampel/config',
              __("co2ampel.script.save_failed"),
              __("co2ampel.script.reboot_content_changed"));
    }

    render(props: {}, state: Readonly<Co2AmpelConfig>) {
        if (!util.render_allowed())
            return <></>

        return (
            <SubPage>
                <ConfigForm id="co2ampel_config_form" title={__("co2ampel.content.co2ampel")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("co2ampel.content.temperature_offset")} label_muted={__("co2ampel.content.temperature_offset_muted")}>
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
            </SubPage>
        );
    }
}

render(<Co2Ampel />, $("#co2ampel")[0]);

interface Co2AmpelStatusState {
    state: API.getType["co2ampel/state"];
}

export class Co2AmpelStatus extends Component<{}, Co2AmpelStatusState>
{
    constructor()
    {
        super();

        util.addApiEventListener('co2ampel/state', () => {
            this.setState({state: API.get('co2ampel/state')})
        });
    }

    render(props: {}, state: Co2AmpelStatusState)
    {
        if (!util.render_allowed())
            return <></>;

        return <>
                <FormRow label={__("co2ampel.status.led_state")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={state.state.led}
                        items={[
                            ["secondary", __("co2ampel.status.led_off")],
                            ["success", __("co2ampel.status.led_green")],
                            ["warning", __("co2ampel.status.led_yellow")],
                            ["danger", __("co2ampel.status.led_red")],
                        ]}
                    />
                </FormRow>

                <FormRow label={__("co2ampel.status.co2")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <OutputFloat value={state.state.co2} digits={0} scale={0} unit="ppm"/>
                </FormRow>

                <FormRow label={__("co2ampel.status.temperature")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <OutputFloat value={state.state.temperature} digits={2} scale={2} unit="°C"/>
                </FormRow>

                <FormRow label={__("co2ampel.status.humidity")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <OutputFloat value={state.state.humidity} digits={2} scale={2} unit="% RH"/>
                </FormRow>
            </>;
    }
}

render(<Co2AmpelStatus />, $("#status-co2ampel")[0]);

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-co2ampel").prop("hidden", !module_init.co2ampel);
}
