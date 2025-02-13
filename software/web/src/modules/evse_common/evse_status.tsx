/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

//#include "module_available.inc"

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { __ } from "../../ts/translation";
import { h, Fragment, Component } from "preact";
import { Button, Dropdown } from "react-bootstrap";
import { FormRow } from "../../ts/components/form_row";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { InputFloat } from "../../ts/components/input_float";
import { InputText } from "../../ts/components/input_text";
import { StatusSection } from "../../ts/components/status_section";
import { useId } from "preact/hooks";

interface EVSEStatusState {
    hidden: boolean;
    state: API.getType["evse/state"];
    auto_start: API.getType["evse/auto_start_charging"];
    slots: Readonly<API.getType["evse/slots"]>;
    configured_current: number;
    is_evse_v2: boolean;
}

export class EVSEStatus extends Component<{}, EVSEStatusState> {
    timeout: number = undefined;

    constructor() {
        super();

        this.state = {
            hidden: true,
        } as any;

        util.addApiEventListener("info/modules", () => {
            this.setState({hidden: !API.hasModule("evse_v2") && !API.hasModule("evse")});
        });

        util.addApiEventListener('evse/state', () => {
            this.setState({state: API.get('evse/state')})
        });

        util.addApiEventListener('evse/auto_start_charging', () => {
            this.setState({auto_start: API.get('evse/auto_start_charging')})
        });

        util.addApiEventListener('evse/slots', () => {
            let slots = API.get('evse/slots');
            this.setState({slots: slots, configured_current: Math.min(slots[0].max_current, slots[1].max_current, slots[5].max_current)});
        });

        util.addApiEventListener("evse/hardware_configuration", () => {
            this.setState({is_evse_v2: API.get("evse/hardware_configuration").evse_version >= 20});
        });
    }

    update_evse_slots() {
        let slots = this.state.slots;

        let real_maximum = 32000;
        for (let i = 0; i < slots.length; ++i) {
            let s = slots[i];
            if (s.active)
                real_maximum = Math.min(real_maximum, s.max_current);
        }

        if (real_maximum == 32000)
            return util.toLocaleFixed(real_maximum / 1000.0, 3) + " A";

        let rm_string = (real_maximum == 0) ? __("evse.script.acc_blocked") : (util.toLocaleFixed(real_maximum / 1000.0, 3)  + " A");

        let status_string = rm_string + " " + __("evse.script.by") + " ";

        let status_list = [];
        for (let i = 0; i < slots.length; ++i) {
            let s = slots[i];
            if (!s.active || s.max_current != real_maximum)
                continue;

            status_list.push(__("evse.script.slot")(i, this.state.is_evse_v2));
        }

        status_string += status_list.join(", ");

        return status_string;
    }

    timeoutSave(current: number, theoretical_max: number) {
        if (current === theoretical_max)
            current = 32000;

        if (current == this.state.slots[5].max_current)
            return;

        API.save('evse/global_current', {"current": current}, () => __("evse.script.set_charging_current_failed"));
    }

//#if MODULE_NFC_AVAILABLE
    get_nfc_tag_list() {
        // We only care about tags that are mapped to a user
        const users = API.get("users/config").users;
        const tags = API.get("nfc/config").authorized_tags.filter(t => t.user_id > 0);

        let result = [];

        for (let u of users) {
            const users_tags = tags.filter(t => t.user_id == u.id);
            let user_has_multiple_tags = users_tags.length > 1;

            result.push(...users_tags.map(
                t =>
                    <Dropdown.Item as="button"
                                   className="py-2"
                                   onClick={() => {API.call("nfc/inject_tag_start",
                                                            {tag_type: t.tag_type, tag_id: t.tag_id},
                                                            () => __("evse.script.start_charging_failed"));
                                                   API.call("evse/start_charging", {}, () => __("evse.script.start_charging_failed"));}
                                   }>
                        {u.display_name + (user_has_multiple_tags ? " (" + t.tag_id +")" : "")}
                    </Dropdown.Item>
                ))
        }
        return result;
    }
//#endif

    render(props: {}, state: EVSEStatusState) {
        if (!util.render_allowed() || !API.hasFeature("evse") || state.hidden)
            return <StatusSection name="evse" />;

        let theoretical_max = Math.min(state.slots[0].max_current, state.slots[1].max_current);

//#if MODULE_NFC_AVAILABLE
        let nfc_start_button = <Dropdown className="flex-grow-1 mr-2">
                <Dropdown.Toggle variant="primary"
                                 id={useId()}
                                 className="form-control rounded-right"
                                 disabled={!(state.state.iec61851_state == 1 && (state.slots[4].max_current == 0 || state.slots[6].max_current == 0))}>
                    {__("evse.status.start_charging")}
                </Dropdown.Toggle>
                <Dropdown.Menu alignRight>
                    <Dropdown.Header>{__("evse.status.start_charge_for_user")}</Dropdown.Header>
                    {this.get_nfc_tag_list()}
                </Dropdown.Menu>
            </Dropdown>
//#endif

        let evse_start_button = <Button
                className="form-control mr-2 rounded-right"
                disabled={!(state.state.iec61851_state == 1 && state.slots[4].max_current == 0)}
                onClick={() =>  API.call('evse/start_charging', {}, () => __("evse.script.start_charging_failed"))}>
                {__("evse.status.start_charging")}
            </Button>

        return <StatusSection name="evse">
                <FormRow label={__("evse.status.evse")}>
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
                        ]}
                    />
                </FormRow>

                <FormRow label={__("evse.status.charge_control")}>
                        <div class="input-group">
{/*#if MODULE_NFC_AVAILABLE*/}
                        {state.slots[6].active ? nfc_start_button : evse_start_button}
{/*#else*/}
                        {evse_start_button}
{/*#endif*/}
                        <Button
                            className="form-control rounded-left"
                            disabled={!(state.state.charger_state == 2 || state.state.charger_state == 3 || (state.state.iec61851_state == 1 && state.slots[4].max_current != 0))}
                            onClick={() => {
{/*#if MODULE_NFC_AVAILABLE*/}
{/*#if MODULE_CHARGE_TRACKER_AVAILABLE*/}
                                let cc = API.get("charge_tracker/current_charge")
                                if (cc.authorization_type == 2 || cc.authorization_type == 3)
                                    API.call('nfc/inject_tag_stop',
                                             cc.authorization_info,
                                             () => __("evse.script.stop_charging_failed"));
{/*#endif*/}
{/*#endif*/}
                                API.call('evse/stop_charging', {}, () => __("evse.script.stop_charging_failed"));
                            }}>
                            {__("evse.status.stop_charging")}
                        </Button>
                    </div>
                </FormRow>
                <FormRow label={__("evse.status.configured_charging_current")}>
                        <InputFloat min={6000} max={theoretical_max} digits={3} unit="A"
                            value={state.configured_current}
                            onValue={(v) => {
                                window.clearTimeout(this.timeout);

                                this.timeout = window.setTimeout(() => this.timeoutSave(v, theoretical_max), 1000);
                                this.setState({configured_current: v})
                            }}
                            showMinMax/>
                </FormRow>
                <FormRow label={__("evse.status.allowed_charging_current")}>
                        <InputText value={this.update_evse_slots()} />
                </FormRow>
            </StatusSection>;
    }
}
