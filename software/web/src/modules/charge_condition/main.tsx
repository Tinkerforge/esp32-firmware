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

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render, Fragment, Component } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";

import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { Button, InputGroup } from "react-bootstrap";
import { InputSelect } from "src/ts/components/input_select";
import { InputFloat } from "src/ts/components/input_float";


interface ChargeConditionState {
    state: API.getType["charge_condition/state"]
    meter_abs: number
    evse_uptime: number
}

interface ChargeConditionOverrideState extends ChargeConditionState
{
    config_in_use: API.getType['charge_condition/live_config']
    config: API.getType['charge_condition/config']
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



        util.addApiEventListener("charge_condition/live_config", () => {
            this.setState({config_in_use: API.get("charge_condition/live_config")});
        })

        util.addApiEventListener("charge_condition/config", () => {
            this.setState({config: API.get("charge_condition/config")})
        })

        util.addApiEventListener("charge_condition/state", () => {
            this.setState({state: API.get("charge_condition/state")});
        })

        util.addApiEventListener("meter/values", () => {
            this.setState({meter_abs: API.get("meter/values").energy_abs});
        })

        util.addApiEventListener("evse/low_level_state", () => {
            this.setState({evse_uptime: API.get("evse/low_level_state").uptime});
        })
    }

    render(props: {}, s: ChargeConditionOverrideState)
    {
        let {state,
            config_in_use,
            config,
            evse_uptime,
            meter_abs} = s;

        if (!state || !config_in_use || !config)
            return <></>;

        const has_meter = API.hasFeature("meter");

        const get_energy_left = () => {
            let energy = state.target_energy_kwh / 1000 - meter_abs;
            if (state.start_energy_kwh == 0)
                energy = config_in_use.energy_limit_kwh / 1000;
            else if (energy < 0)
                energy = 0;

            let ret = util.toLocaleFixed(energy, 3);

            if (config_in_use.energy_limit_kwh != config.energy_limit_kwh)
                ret += " " + __("charge_condition.content.overridden");

            return ret;
        }

        const get_duration_left = () => {
            if (config_in_use.duration_limit == 0 && config_in_use.duration_limit != config.duration_limit)
                return __("charge_condition.content.unlimited") + " " + __("charge_condition.content.overridden");

            let duration: number;
            duration = state.target_timestamp_mil / 1000;
            if (state.start_timestamp_mil != 0)
                duration = Math.floor((state.target_timestamp_mil - evse_uptime) / 1000);
            if (state.target_timestamp_mil - evse_uptime < 0 && state.start_timestamp_mil != 0)
                duration = 0;

            let ret = util.format_timespan(duration);

            if (config.duration_limit != config_in_use.duration_limit)
                ret += " " +__("charge_condition.content.overridden");

            return ret;
        }


        const energy_override = <FormRow label={__("charge_condition.content.override_energy")} labelColClasses="col-sm-4" contentColClasses="col-lg-8 col-xl-4">
                                    <InputFloat value={config_in_use.energy_limit_kwh}
                                                onValue={(v) => {
                                                    this.setState({config_in_use: {...config_in_use, energy_limit_kwh: v}});
                                                    API.call("charge_condition/override_energy", {energy: v}, __("charge_condition.script.override_failed"));
                                                }}
                                                digits={3} min={0} max={100000} unit={"kwh"}/>
                            </FormRow>

        const energy_left = <FormRow label={__("charge_condition.content.energy_left")} labelColClasses="col-sm-4" contentColClasses="col-lg-8 col-xl-4"
                                         hidden={config_in_use.energy_limit_kwh == 0 && config_in_use.energy_limit_kwh == config.energy_limit_kwh}>
                                <InputGroup>
                                    <InputText value={get_energy_left()}/>
                                    <InputGroup.Append>
                                        <div class="form-control input-group-text">
                                            kwh
                                        </div>
                                    </InputGroup.Append>
                                    <InputGroup.Append>
                                        <Button onClick={() => {
                                            this.setState({config_in_use: {...config_in_use, energy_limit_kwh: config.energy_limit_kwh}});
                                            API.call("charge_condition/override_energy", {energy: config.energy_limit_kwh}, __("charge_condition.script.override_failed"));
                                        }}
                                        variant="primary"
                                        hidden={config_in_use.energy_limit_kwh == config.energy_limit_kwh}>
                                            {__("charge_condition.content.reset")}
                                        </Button>
                                    </InputGroup.Append>
                                </InputGroup>
                            </FormRow>

        return <>
                <FormRow label={__("charge_condition.content.override_duration")} labelColClasses="col-sm-4" contentColClasses="col-lg-8 col-xl-4">
                    <InputSelect items={[
                            ["0", __("charge_condition.content.unlimited")],
                            ["1", __("charge_condition.content.min15")],
                            ["2", __("charge_condition.content.min30")],
                            ["3", __("charge_condition.content.min45")],
                            ["4", __("charge_condition.content.h1")],
                            ["5", __("charge_condition.content.h2")],
                            ["6", __("charge_condition.content.h3")],
                            ["7", __("charge_condition.content.h4")],
                            ["8", __("charge_condition.content.h6")],
                            ["9", __("charge_condition.content.h8")],
                            ["10", __("charge_condition.content.h12")]
                        ]}
                        value={config_in_use.duration_limit}
                        onValue={(v) => {
                            this.setState({config_in_use: {...config_in_use, duration_limit: Number(v)}})
                            API.call("charge_condition/override_duration", {duration: Number(v)}, __("charge_condition.script.override_failed"));
                    }}/>
                </FormRow>
                <FormRow label={__("charge_condition.content.time_left")} labelColClasses="col-sm-4" contentColClasses="col-lg-8 col-xl-4"
                            hidden={config_in_use.duration_limit === 0 && config_in_use.duration_limit == config.duration_limit}>
                    <InputGroup>
                        <InputText value={get_duration_left()}/>
                        <InputGroup.Append>
                            <Button onClick={() => {
                                        this.setState({config_in_use: {...config_in_use, duration_limit: config.duration_limit}})
                                        API.call("charge_condition/override_duration", {duration: config.duration_limit}, __("charge_condition.script.override_failed"));
                                    }}
                                    className="form-control"
                                    variant="primary"
                                    hidden={config.duration_limit == config_in_use.duration_limit}>
                                {__("charge_condition.content.reset")}
                            </Button>
                        </InputGroup.Append>
                    </InputGroup>
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
