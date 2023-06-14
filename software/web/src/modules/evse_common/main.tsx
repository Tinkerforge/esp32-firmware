/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

import $ from "jquery"

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, Component, Fragment, render } from "preact"
import { IndicatorGroup } from "src/ts/components/indicator_group";
import { __, translate_unchecked } from "../../ts/translation"
import { FormRow } from "src/ts/components/form_row";
import { Switch } from "src/ts/components/switch";
import { Button } from "react-bootstrap";
import { InputFloat } from "src/ts/components/input_float";
import { InputText } from "src/ts/components/input_text";

interface EVSEStatusState {
    state: API.getType['evse/state']
    auto_start: API.getType['evse/auto_start_charging']
    slots: Readonly<API.getType['evse/slots']>
    configured_current: number
}

export class EVSEStatus extends Component<{}, EVSEStatusState>
{
    timeout: number;
    constructor()
    {
        super();

        this.timeout = null;

        util.addApiEventListener('evse/state', () => {
            this.setState({state: API.get('evse/state')})
        });

        util.addApiEventListener('evse/auto_start_charging', () => {
            this.setState({auto_start: API.get('evse/auto_start_charging')})
        })

        util.addApiEventListener('evse/slots', () => {
            let slots = API.get('evse/slots');
            this.setState({slots: slots, configured_current: Math.min(slots[0].max_current, slots[1].max_current, slots[5].max_current)});
        })
    }

    update_evse_slots() {
        let slots = this.state.slots;

        let real_maximum = 32000;
        for(let i = 0; i < slots.length; ++i) {
            let s = slots[i];
            if (s.active)
                real_maximum = Math.min(real_maximum, s.max_current);
        }

        if (real_maximum == 32000)
            return util.toLocaleFixed(real_maximum / 1000.0, 3) + " A";

        let status_string = util.toLocaleFixed(real_maximum / 1000.0, 3) + " A " + __("evse.script.by") + " ";

        let status_list = [];
        for(let i = 0; i < slots.length; ++i) {
            let s = slots[i];
            if (!s.active || s.max_current != real_maximum)
                continue;

            status_list.push(translate_unchecked(`evse.script.slot_${i}`));
        }

        status_string += status_list.join(", ");

        return status_string;
    }


    timeoutSave(current: number, theoretical_max: number)
    {
        if (current === theoretical_max)
            current = 32000;

        if (current == this.state.slots[5].max_current)
            return;

        API.save('evse/global_current', {"current": current}, __("evse.script.set_charging_current_failed"));
    }

    render(props: {}, state: EVSEStatusState)
    {
        if (!util.render_allowed() || !API.hasFeature("evse"))
            return <></>;

        let theoretical_max = Math.min(state.slots[0].max_current, state.slots[1].max_current);

        return <>
                <FormRow label={__("evse.status.evse")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={state.state.charger_state}
                        items={[
                            ["primary", __("evse.status.not_connected")],
                            ["warning", __("evse.status.waiting_for_charge_release")],
                            ["info",    __("evse.status.ready_to_charge")],
                            ["success", __("evse.status.charging")],
                            ["danger",  __("evse.status.error")]
                        ]}/>
                </FormRow>

                <FormRow label={__("evse.status.charge_control")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                        <div class="input-group">
                        <Button
                            className="form-control mr-2 rounded-right"
                            disabled={!(state.state.iec61851_state == 1 && state.slots[4].max_current == 0)}
                            onClick={() =>  API.call('evse/start_charging', {}, __("evse.script.start_charging_failed"))}>
                            {__("evse.status.start_charging")}
                        </Button>
                        <Button
                            className="form-control rounded-left"
                            disabled={!(state.state.charger_state == 2 || state.state.charger_state == 3 || (state.state.iec61851_state == 1 && state.slots[4].max_current != 0))}
                            onClick={() => API.call('evse/stop_charging', {}, __("evse.script.stop_charging_failed"))}>
                            {__("evse.status.stop_charging")}
                        </Button>
                        </div>
                </FormRow>
                <FormRow label={__("evse.status.configured_charging_current")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4 input-group">
                        <InputFloat min={6000} max={theoretical_max} digits={3} unit="A"
                            value={state.configured_current}
                            onValue={(v) => {
                                window.clearTimeout(this.timeout);

                                this.timeout = window.setTimeout(() => this.timeoutSave(v, theoretical_max), 1000);
                                this.setState({configured_current: v})
                            }}
                            showMinMax/>
                </FormRow>
                <FormRow label={__("evse.status.allowed_charging_current")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                        <InputText value={this.update_evse_slots()} />
                </FormRow>
            </>;
    }
}

render(<EVSEStatus/>, $('#status-evse')[0]);

export function init(){}
export function add_event_listeners(){}
export function update_sidebar_state(module_init: any) {}
