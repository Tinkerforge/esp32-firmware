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
    electricity_price: number
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

        util.addApiEventListener("charge_tracker/config", () => {
            this.setState({electricity_price: API.get("charge_tracker/config").electricity_price});
        })
    }

    render(props: {}, s: ChargeConditionOverrideState)
    {
        let {state,
            config_in_use,
            config,
            evse_uptime,
            meter_abs,
            electricity_price} = s;

        if (!state || !config_in_use || !config)
            return <></>;

        const has_meter = API.hasFeature("meter");

        const get_energy_left = () => {
            let ret = "";
            if (config_in_use.energy_limit_wh == 0)
                ret = __("charge_condition.content.unlimited")
            else {
                let energy = state.target_energy_kwh - meter_abs;
                if (state.start_energy_kwh == null)
                    energy = config_in_use.energy_limit_wh / 1000.0;
                else if (energy < 0)
                    energy = 0;

                ret = util.toLocaleFixed(energy, 3) + " kWh";
            }

            return ret;
        }

        const get_duration_left = () => {
            let ret = "";
            if (config_in_use.duration_limit == 0)
                ret = __("charge_condition.content.unlimited")
            else {
                let duration: number;
                duration = state.target_timestamp_ms;
                if (state.start_timestamp_ms != 0)
                    duration = Math.floor((state.target_timestamp_ms - evse_uptime));
                if (state.target_timestamp_ms - evse_uptime < 0 && state.start_timestamp_ms != 0)
                    duration = 0;

                ret = util.format_timespan(Math.floor(duration / 1000));
            }

            return ret;
        }

        let duration_items: [string, string][] = [
            ["disabled", get_duration_left()],
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
        ];

        if (config_in_use.duration_limit != 0)
            duration_items[0][1] = duration_items[config_in_use.duration_limit + 1][1] + " | " + get_duration_left() + __("charge_condition.content.left");

        let energy_items: [string, string][] = [
            ["disabled", get_energy_left()],
            ["0", __("charge_condition.content.unlimited")],
            ["5000", util.toLocaleFixed(5, 0) + " kWh"],
            ["10000", util.toLocaleFixed(10, 0) + " kWh"],
            ["15000", util.toLocaleFixed(15, 0) + " kWh"],
            ["20000", util.toLocaleFixed(20, 0) + " kWh"],
            ["25000", util.toLocaleFixed(25, 0) + " kWh"],
            ["30000", util.toLocaleFixed(30, 0) + " kWh"],
            ["40000", util.toLocaleFixed(40, 0) + " kWh"],
            ["50000", util.toLocaleFixed(50, 0) + " kWh"],
            ["60000", util.toLocaleFixed(60, 0) + " kWh"],
            ["70000", util.toLocaleFixed(70, 0) + " kWh"],
            ["80000", util.toLocaleFixed(80, 0) + " kWh"],
            ["90000", util.toLocaleFixed(90, 0) + " kWh"],
            ["100000", util.toLocaleFixed(100, 0) + " kWh"]
        ];

        if (electricity_price > 0) {
            for(let i = 2; i < energy_items.length; ++i) {
                energy_items[i][1] += ` (~ ${util.toLocaleFixed(electricity_price / 10000 * parseFloat(energy_items[i][0]) / 1000, 2)} €)`
            }
        }

        if (config_in_use.energy_limit_wh != 0)
            energy_items[0][1] = util.toLocaleFixed(config_in_use.energy_limit_wh / 1000, 0) + " kWh" + " | " + get_energy_left() + __("charge_condition.content.left");

        let conf_idx = energy_items.findIndex(x => x[0] == config.energy_limit_wh.toString())
        if (conf_idx == -1)
            energy_items = [energy_items[0],
                            [config.energy_limit_wh.toString(),
                             util.toLocaleFixed(config.energy_limit_wh / 1000.0, 3)
                             + " kWh "
                             + (electricity_price > 0 ? ` (~ ${util.toLocaleFixed(electricity_price / 10000 * config.energy_limit_wh / 1000, 2)} €)` : "")
                             + __("charge_condition.content.configured")],
                            ...energy_items.slice(1)];
        else
            energy_items[conf_idx][1] += " " + __("charge_condition.content.configured");

        if (config.energy_limit_wh != config_in_use.energy_limit_wh) {
            let conf_idx = energy_items.findIndex(x => x[0] == config_in_use.energy_limit_wh.toString())
            if (conf_idx == -1)
                console.log("ups");
            else
                energy_items[conf_idx][1] += " " + __("charge_condition.content.overridden");
        }


        duration_items[config.duration_limit + 1][1] += " " + __("charge_condition.content.configured");
        if (config.duration_limit != config_in_use.duration_limit)
            duration_items[config_in_use.duration_limit + 1][1] += " " + __("charge_condition.content.overridden");

        return <>
                <FormRow label={__("charge_condition.content.override_duration")} labelColClasses="col-sm-4" contentColClasses="col-lg-8 col-xl-4">
                    <InputSelect items={duration_items}
                        value={"disabled"}
                        onValue={(v) => {
                            this.setState({config_in_use: {...config_in_use, duration_limit: Number(v)}})
                            API.call("charge_condition/override_duration", {duration: Number(v)}, __("charge_condition.script.override_failed"));
                    }}/>
                </FormRow>
                {has_meter ? <>{
                    <FormRow label={__("charge_condition.content.override_energy")} labelColClasses="col-sm-4" contentColClasses="col-lg-8 col-xl-4">
                        <InputSelect items={energy_items}
                            value={"disabled"}
                            onValue={(v) => {
                                this.setState({config_in_use: {...config_in_use, energy_limit_wh: Number(v)}})
                                API.call("charge_condition/override_energy", {energy_wh: Number(v)}, __("charge_condition.script.override_failed"));
                        }}/>
                    </FormRow>
                }</> : <></>}
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
