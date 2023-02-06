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

import { h, render, Fragment } from "preact";
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
import { DateTimePicker} from "src/ts/components/datetime_picker"
import { Switch } from "src/ts/components/switch";

type ChargeConditionConfig = API.getType["charge_condition/config"];

interface ChargeConditionState {
    state: API.getType["charge_condition/state"]
}

export class ChargeCondition extends ConfigComponent<'charge_condition/config', {}, ChargeConditionState> {


    constructor() {
        super('charge_condition/config',
              __("charge_manager.script.save_failed"),
              __("charge_manager.script.reboot_content_changed"));

        util.eventTarget.addEventListener("charge_condition/state", () => {
            this.setState({state: API.get("charge_condition/state")});
        });
    }

    handle_change(idx: number, e: boolean[])
    {
        const times = this.state.allowed_times;
        times.splice(idx, 1, e)
        this.setState({allowed_times: [...times]});
    }

    render(props: {}, state: ChargeConditionConfig & ChargeConditionState) {
        if (!state || !state.state)
            return (<></>);
        let has_meter = API.hasFeature("meter");

        let energy_settings = <FormRow label="Energie Einstellung">
                <InputFloat value={state.energy_limit_kwh} onValue={(v) => this.setState({energy_limit_kwh: v})} digits={3} min={0} max={100000} unit={"kwh"}/>
            </FormRow>;

        let energy_display = <FormRow label="Energy über">
                <InputFloat value={state.state.energy_left_kwh} digits={3} unit={"kwh"}/>
            </FormRow>
        console.log(state.allowed_times);
        return (
            <>
                <ConfigForm title="Charge Condition" id="charge_condition_config_form" isModified={this.isModified()} onSave={() => this.save()} onReset={() => this.reset()} onDirtyChange={(d) => this.ignore_updates = d}>
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
                        <InputText value={util.format_timespan(state.state.duration_left_sec)}/>
                    </FormRow>
                    {has_meter ? energy_display : <></>}
                    <FormSeparator/>
                    <FormGroup label="Activate time">
                        <Switch onClick={() => this.setState({time_restriction_enabled: !state.time_restriction_enabled})} desc="lalala" checked={state.time_restriction_enabled}/>
                    </FormGroup>
                    <DateTimePicker onValue={(v) => this.setState({allowed_times: v})} value={state.allowed_times}/>
                </ConfigForm>
            </>
        );
    }
}

render(<ChargeCondition/>, $('#charge_condition')[0]);


export function init()
{

}

export function add_event_listeners()
{

}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-charge_condition').prop('hidden', !module_init.charge_condition);
}
