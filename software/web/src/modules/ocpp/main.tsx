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

import { h, render, Component, Fragment, VNode} from "preact";
import { translate_unchecked, __ } from "../../ts/translation";

import { Switch } from "../../ts/components/switch";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { ConfigForm } from "../../ts/components/config_form";
import { ConfigComponent } from "../../ts/components/config_component";
import { Button } from "react-bootstrap";
import { CollapsedSection } from "src/ts/components/collapsed_section";
import { state } from "./api";
import { InputPassword } from "src/ts/components/input_password";

type OcppConfig = API.getType['ocpp/config']

interface OcppState {
    state: API.getType['ocpp/state'];
    configuration: API.getType['ocpp/configuration'];
}

export class Ocpp extends ConfigComponent<'ocpp/config', {}, OcppState> {
    constructor() {
        super('ocpp/config',
              __("ocpp.script.save_failed"),
              __("ocpp.script.reboot_content_changed"));

        util.eventTarget.addEventListener('ocpp/state', () => {
            this.setState({state: API.get('ocpp/state')});
        });

        util.eventTarget.addEventListener('ocpp/configuration', () => {
            this.setState({configuration: API.get('ocpp/configuration')});
        });
    }

    override async sendSave(t: "ocpp/config", cfg: OcppConfig) {
        await API.save_maybe('evse/ocpp_enabled', {enabled: this.state.enable}, __("evse.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    override async sendReset(t: "ocpp/config") {
        await API.save_maybe('evse/ocpp_enabled', {enabled: false}, __("evse.script.save_failed"));
        await super.sendReset(t);
    }

    override getIsModified(t: "ocpp/config"): boolean {
        let evse = API.get_maybe("evse/ocpp_enabled");
        if (evse != null)
            if (evse.enabled)
                return true;
        return super.getIsModified(t);
    }

    render(props: {}, state: OcppConfig & OcppState) {
        if (!state || !state.state)
            return (<></>);

        return (
            <>
                <ConfigForm id="ocpp_config_form" title={__("ocpp.content.ocpp")} isModified={this.isModified()} onSave={this.save} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("ocpp.content.enable_ocpp")}>
                        <Switch desc={__("ocpp.content.enable_ocpp_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}/>
                    </FormRow>
                    <FormRow label={__("ocpp.content.endpoint_url")}>
                        <InputText required
                                   maxLength={128}
                                   value={state.url}
                                   onValue={this.set("url")}/>
                    </FormRow>
                    <FormRow label={__("ocpp.content.identity")}>
                        <InputText required
                                   maxLength={64}
                                   value={state.identity}
                                   onValue={this.set("identity")}/>
                    </FormRow>
                    <FormRow label={__("ocpp.content.pass")}>
                        <InputPassword maxLength={64}
                                       value={state.pass}
                                       onValue={this.set("pass")}/>
                    </FormRow>
                    <FormRow label={__("ocpp.content.reset")} label_muted={__("ocpp.content.reset_muted")}>
                        <Button variant="danger" className="form-control" onClick={async () =>{
                            const modal = util.async_modal_ref.current;
                            if (!await modal.show({
                                    title: __("ocpp.content.reset_title"),
                                    body: __("ocpp.content.reset_title_text"),
                                    no_text: __("ocpp.content.abort_reset"),
                                    yes_text: __("ocpp.content.confirm_reset"),
                                    no_variant: "secondary",
                                    yes_variant: "danger"
                                }))
                                return;

                            API.call("ocpp/reset", null, __("ocpp.content.reset_failed"), __("ocpp.script.reboot_content_changed"));
                            }}>
                            {__("ocpp.content.reset")}
                        </Button>
                    </FormRow>

                    <CollapsedSection label={__("ocpp.content.debug")}>
                        <FormRow label={__("ocpp.content.charge_point_state")}>
                            <InputText value={translate_unchecked(`ocpp.content.charge_point_state_${state.state.charge_point_state}`)} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.charge_point_status")}>
                            <InputText value={translate_unchecked(`ocpp.content.status_${state.state.charge_point_status}`)} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.next_profile_eval")}>
                            <InputText value={new Date(state.state.next_profile_eval * 1000).toLocaleString()} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.connector_state")}>
                            <InputText value={translate_unchecked(`ocpp.content.connector_state_${state.state.connector_state}`)} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.connector_status")}>
                            <InputText value={translate_unchecked(`ocpp.content.status_${state.state.connector_status}`)} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.tag_id")}>
                            <InputText value={state.state.tag_id} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.parent_tag_id")}>
                            <InputText value={state.state.parent_tag_id} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.tag_expiry_date")}>
                            <InputText value={new Date(state.state.tag_expiry_date * 1000).toLocaleString()} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.tag_deadline")}>
                            <InputText value={state.state.tag_timeout} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.cable_deadline")}>
                            <InputText value={state.state.cable_timeout} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_id")}>
                            <InputText value={state.state.txn_id} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_confirmed_time")}>
                            <InputText value={new Date(state.state.txn_confirmed_time * 1000).toLocaleString()} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_start_time")}>
                            <InputText value={new Date(state.state.txn_start_time * 1000).toLocaleString()} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.current")}>
                            <InputText value={state.state.current / 1000.0 + " A"} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_with_invalid_id")}>
                            <InputText value={state.state.txn_with_invalid_id ? "true" : "false"} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.unavailable_requested")}>
                            <InputText value={state.state.unavailable_requested ? "true" : "false"} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.message_in_flight_type")}>
                            <InputText value={translate_unchecked(`ocpp.content.message_in_flight_type_${state.state.message_in_flight_type}`)} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.message_in_flight_id")}>
                            <InputText value={state.state.message_in_flight_id_high == 0 ? state.state.message_in_flight_id_low : "0x" + state.state.message_in_flight_id_high.toString(16) + state.state.message_in_flight_id_low.toString(16)} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.message_in_flight_len")}>
                            <InputText value={state.state.message_in_flight_len} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.message_deadline")}>
                            <InputText value={state.state.message_timeout} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_msg_retry_deadline")}>
                            <InputText value={state.state.txn_msg_retry_timeout} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.message_queue_depth")}>
                            <InputText value={state.state.message_queue_depth} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.status_queue_depth")}>
                            <InputText value={state.state.status_queue_depth} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_msg_queue_depth")}>
                            <InputText value={state.state.txn_msg_queue_depth} />
                        </FormRow>
                    </CollapsedSection>

                    <CollapsedSection label={__("ocpp.content.configuration")}>
                        {(Object.keys(state.configuration) as Array<keyof typeof state.configuration>).map((k, i) => (
                            <FormRow label={k}>
                                <InputText value={state.configuration[k]}
                                           onValue={(v) => this.setState({configuration: {...this.state.configuration, [k]: v}})}
                                           onfocusout={() => API.call("ocpp/change_configuration", {
                                                key: k,
                                                value: state.configuration[k]
                                           }, "lalala")} />
                            </FormRow>)
                        )}
                    </CollapsedSection>
                </ConfigForm>
            </>
        );
    }
}

render(<Ocpp/>, $('#ocpp')[0])

export function add_event_listeners(source: API.APIEventTarget) {}

export function init() {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-ocpp').prop('hidden', !module_init.ocpp);
}
