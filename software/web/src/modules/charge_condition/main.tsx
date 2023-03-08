/* esp32-firmware
 * Copyright (C) 2021 Erik Fleckstein <erik@tinkerforge.com>
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

import feather from "../../ts/feather";

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render, Fragment, Component } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";

import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { FormGroup } from "../../ts/components/form_group";
import { InputText } from "../../ts/components/input_text";
import { Button, Card, Collapse, ListGroup, Modal } from "react-bootstrap";
import { InputSelect } from "src/ts/components/input_select";
import { FormSeparator } from "src/ts/components/form_separator";
import { InputNumber } from "src/ts/components/input_number";
import { InputFloat } from "src/ts/components/input_float";
import { Switch } from "src/ts/components/switch";

type ChargeConditionConfig = API.getType["charge_condition/config"];

interface ChargeConditionState {
    state: API.getType["charge_condition/state"]
    meter_abs: number
    evse_uptime: number
}

export class ChargeCondition extends ConfigComponent<'charge_condition/config', {}, ChargeConditionState> {


    constructor() {
        super('charge_condition/config',
              __("charge_manager.script.save_failed"),
              __("charge_manager.script.reboot_content_changed"));

        this.state = {
            duration_limit: 0,
            energy_limit_kwh: 0,
            time_restriction_enabled: false,
            meter_abs: 0,
            evse_uptime: 0
        } as any;

        util.eventTarget.addEventListener("charge_condition/state", () => {
            this.setState({state: API.get("charge_condition/state")});
        });

        util.eventTarget.addEventListener("meter/values", () => {
            this.setState({meter_abs: API.get("meter/values").energy_abs});
        })

        util.eventTarget.addEventListener("evse/low_level_state", () => {
            this.setState({evse_uptime: API.get("evse/low_level_state").uptime});
        })
    }

    maxBitmap(): number {
        let num = 0;
        for (let i = 0; i < 24; i++)
        {
            let tmp = 1 << i;
            num = num | tmp;
        }
        return num;
    }

    hackToAllowSave() {
        document.getElementById("charge_condition_config_form").dispatchEvent(new Event('input'));
    }

    render(props: {}, state: ChargeConditionConfig & ChargeConditionState) {

        if (!state || !state.state)
            return (<></>);

        let has_meter = API.hasFeature("meter");

        let energy_settings = <FormRow label="Energie Einstellung">
                <InputFloat value={state.energy_limit_kwh}
                            onValue={(v) => this.setState({energy_limit_kwh: v})}
                            digits={3} min={0} max={100000} unit={"kwh"}/>
            </FormRow>;

        let energy_display = <FormRow label="Energy über">
                <InputFloat value={state.state.target_energy_kwh != 0 ? state.state.target_energy_kwh - (state.meter_abs * 1000) : 0}
                            digits={3} unit={"kwh"}/>
            </FormRow>

        //TODO: Investigate

        return (
            <>
                <ConfigForm title="Charge Condition"
                            id="charge_condition_config_form"
                            isModified={this.isModified()}
                            onSave={() => this.save()}
                            onReset={() => this.reset()}
                            onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label="Zeit Einstellung">
                        <InputSelect items={[
                            ["0", "Aus"],
                            ["1", "15 Min"],
                            ["2", "30 Min"],
                            ["3", "45 Min"],
                            ["4", "1 H"],
                            ["5", "2 H"],
                            ["6", "3 H"],
                            ["7", "4 H"],
                            ["8", "6 H"],
                            ["9", "8 H"],
                            ["10", "12 H"]
                        ]}
                        value={state.duration_limit}
                        onValue={(v) => this.setState({duration_limit: Number(v)})}/>
                    </FormRow>
                    {has_meter ? energy_settings : <></>}
                    <FormRow label="Zeit über">
                        <InputText value={state.state.target_timestamp_mil - state.evse_uptime > 0 ?
                                            util.format_timespan(Math.floor((state.state.target_timestamp_mil - state.evse_uptime) / 1000)) :
                                                util.format_timespan(0)}/>
                    </FormRow>
                    {has_meter ? energy_display : <></>}
                </ConfigForm>
            </>
        );
    }
}

render(<ChargeCondition/>, $('#charge_condition')[0]);

interface ChargeConditionOverrideState extends ChargeConditionState
{
    config: API.getType['charge_condition/live_config']
}

class ChargeConditionOverride extends Component<{}, ChargeConditionOverrideState>
{
    timeout_id: number;
    override_duration: number

    constructor()
    {
        super();

        this.timeout_id = null;
        this.override_duration = 0;



        util.eventTarget.addEventListener("charge_condition/live_config", () => {
            console.log("update live cfg");
            this.setState({config: API.get("charge_condition/live_config")});
        })

        util.eventTarget.addEventListener("charge_condition/state", () => {
            this.setState({state: API.get("charge_condition/state")});
        })

        util.eventTarget.addEventListener("meter/values", () => {
            this.setState({meter_abs: API.get("meter/values").energy_abs});
        })

        util.eventTarget.addEventListener("evse/low_level_state", () => {
            this.setState({evse_uptime: API.get("evse/low_level_state").uptime});
        })
    }

    async timeOutHandler(n: number)
    {
    }

    render(props: {}, s: ChargeConditionOverrideState)
    {
        let {state,
            config,
            evse_uptime,
            meter_abs} = s;

        if (!state || !config)
            return <></>;

        const has_meter = API.hasFeature("meter");

        const energy_override = <FormRow label="Energy override" labelColClasses="col-sm-4" contentColClasses="col-lg-8 col-xl-4">
                                <InputFloat value={config.energy_limit_kwh}
                                            onValue={(v) => {
                                                this.setState({config: {...config, energy_limit_kwh: v}});
                                                API.call("charge_condition/override_energy", {energy: v}, "Error");
                                            }}
                                            digits={3} min={0} max={100000} unit={"kwh"}/>
                            </FormRow>

        const energy_left = <FormRow label="Energy über" labelColClasses="col-sm-4" contentColClasses="col-lg-8 col-xl-4" hidden={config.energy_limit_kwh == 0}>
                                <InputFloat value={state.target_energy_kwh - (meter_abs * 1000) > 0 ? state.target_energy_kwh - (meter_abs * 1000) : 0}
                                            digits={3} unit={"kwh"}/>
                            </FormRow>

        return <>
                <FormRow label="Override" labelColClasses="col-sm-4" contentColClasses="col-lg-8 col-xl-4">
                    <InputSelect items={[
                            ["0", "Aus"],
                            ["1", "15 Min"],
                            ["2", "30 Min"],
                            ["3", "45 Min"],
                            ["4", "1 H"],
                            ["5", "2 H"],
                            ["6", "3 H"],
                            ["7", "4 H"],
                            ["8", "6 H"],
                            ["9", "8 H"],
                            ["10", "12 H"]
                        ]}
                        value={config.duration_limit}
                        onValue={(v) => {
                            this.setState({config: {...config, duration_limit: Number(v)}})
                            API.call("charge_condition/override_duration", {duration: Number(v)}, "Error");
                    }}/>
                </FormRow>
                <FormRow label="Zeit über" labelColClasses="col-sm-4" contentColClasses="col-lg-8 col-xl-4"
                            hidden={config.duration_limit === 0}>
                    <InputText value={state.target_timestamp_mil - evse_uptime > 0 ?
                                            util.format_timespan(Math.floor((state.target_timestamp_mil - evse_uptime) / 1000)) :
                                                util.format_timespan(0)}/>
                </FormRow>
                {has_meter ? <>{energy_override}{energy_left}</> : <></>}
            </>
    }
}

render(<ChargeConditionOverride/>, $('#charge_condition_override')[0]);

export function init()
{

}

export function add_event_listeners()
{

}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-charge_condition').prop('hidden', !module_init.charge_condition);
}
