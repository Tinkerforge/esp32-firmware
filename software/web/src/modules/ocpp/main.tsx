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

import { h, render, Component, Fragment } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";

import { Switch } from "../../ts/components/switch";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { ConfigForm } from "../../ts/components/config_form";
import { ConfigComponent } from "../../ts/components/config_component";
import { Button } from "react-bootstrap";
import { CollapsedSection } from "src/ts/components/collapsed_section";
import { InputPassword } from "src/ts/components/input_password";
import { IndicatorGroup } from "src/ts/components/indicator_group";
import { SubPage } from "src/ts/components/sub_page";
import { InputSelect } from "src/ts/components/input_select";

type OcppConfig = API.getType["ocpp/config"];

interface OcppState {
    state: API.getType['ocpp/state'];
    configuration: API.getType['ocpp/configuration'];
}

export class Ocpp extends ConfigComponent<'ocpp/config', {}, OcppState> {
    constructor() {
        super('ocpp/config',
              __("ocpp.script.save_failed"),
              __("ocpp.script.reboot_content_changed"));

        util.addApiEventListener('ocpp/state', () => {
            this.setState({state: API.get('ocpp/state')});
        });

        util.addApiEventListener('ocpp/configuration', () => {
            this.setState({configuration: API.get('ocpp/configuration')});
        });
    }

    override async sendSave(t: "ocpp/config", cfg: OcppConfig) {
        await API.save_unchecked('evse/ocpp_enabled', {enabled: this.state.enable}, __("evse.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    override async sendReset(t: "ocpp/config") {
        await API.save_unchecked('evse/ocpp_enabled', {enabled: false}, __("evse.script.save_failed"));
        await super.sendReset(t);
    }

    override getIsModified(t: "ocpp/config"): boolean {
        let evse = API.get_unchecked("evse/ocpp_enabled");
        if (evse != null && evse.enabled)
            return true;
        return super.getIsModified(t);
    }

    render(props: {}, state: OcppConfig & OcppState) {
        if (!util.render_allowed())
            return <></>

        let ocpp_debug = API.hasFeature("ocpp_debug");
        let msg_in_flight = state.state.message_in_flight_id_high != 0 || state.state.message_in_flight_id_low != 0;

        return (
            <SubPage>
                <ConfigForm id="ocpp_config_form" title={__("ocpp.content.ocpp")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("ocpp.content.enable_ocpp")}>
                        <Switch desc={__("ocpp.content.enable_ocpp_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}/>
                    </FormRow>
                    <FormRow label={__("ocpp.content.endpoint_url")}>
                        <InputText required
                                   maxLength={128}
                                   value={state.url}
                                   onValue={this.set("url")}
                                   pattern="wss?:\/\/.*[^\/]"
                                   invalidFeedback={__("ocpp.content.endpoint_url_invalid")}/>
                    </FormRow>
                    <FormRow label={__("ocpp.content.tls_cert")}>
                        <InputSelect items={[
                                ["-1", __("ocpp.content.use_cert_bundle")],
                            ].concat(API.get('certs/state').certs.map(c => [c.id.toString(), c.name])) as [string, string][]
                            }
                            value={state.cert_id}
                            onValue={(v) => this.setState({cert_id: parseInt(v)})}
                            disabled={!state.url.startsWith("wss://")}
                            required={state.url.startsWith("wss://")}
                        />
                    </FormRow>
                    <FormRow label={__("ocpp.content.identity")}>
                        <InputText required
                                   maxLength={64}
                                   value={state.identity}
                                   onValue={this.set("identity")}/>
                    </FormRow>
                    <FormRow label={__("ocpp.content.enable_auth")}>
                        <Switch desc={__("ocpp.content.enable_auth_desc")}
                                checked={state.enable_auth}
                                onClick={this.toggle('enable_auth')}/>
                    </FormRow>
                    <FormRow label={__("ocpp.content.pass")}>
                        <InputPassword maxLength={64}
                                       value={state.pass}
                                       onValue={s => {
                                           this.setState({pass: s});
                                           if (s != null && s.length > 0)
                                               this.setState({enable_auth: true});
                                       }}/>
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
                            <InputText value={util.timestamp_sec_to_date(state.state.next_profile_eval)} />
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
                            <InputText value={util.timestamp_sec_to_date(state.state.tag_expiry_date, __("ocpp.content.no_tag_seen"))} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.tag_timeout")}>
                            <InputText value={util.format_timespan_ms(state.state.tag_timeout, {replace_u32max_with: __("ocpp.content.not_waiting_for_tag")})} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.cable_timeout")}>
                            <InputText value={util.format_timespan_ms(state.state.cable_timeout, {replace_u32max_with: __("ocpp.content.not_waiting_for_cable")})} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_id")}>
                            <InputText value={state.state.txn_id == 0x7FFFFFFF ?  __("ocpp.content.no_transaction_running") : state.state.txn_id} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_confirmed_time")}>
                            <InputText value={util.timestamp_sec_to_date(state.state.txn_confirmed_time, __("ocpp.content.no_transaction_running"))} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_start_time")}>
                            <InputText value={util.timestamp_sec_to_date(state.state.txn_start_time, __("ocpp.content.no_transaction_running"))} />
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
                            <InputText value={!msg_in_flight ? __("ocpp.content.no_message_in_flight") : translate_unchecked(`ocpp.content.message_in_flight_type_${state.state.message_in_flight_type}`)} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.message_in_flight_id")}>
                            <InputText value={!msg_in_flight ?
                                                __("ocpp.content.no_message_in_flight") :
                                                (state.state.message_in_flight_id_high == 0 ?
                                                    state.state.message_in_flight_id_low :
                                                    ("0x" + state.state.message_in_flight_id_high.toString(16) + state.state.message_in_flight_id_low.toString(16)))} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.message_in_flight_len")}>
                            <InputText value={!msg_in_flight ? __("ocpp.content.no_message_in_flight") : state.state.message_in_flight_len} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.message_timeout")}>
                            <InputText value={util.format_timespan_ms(state.state.message_timeout, {replace_u32max_with: __("ocpp.content.no_message_in_flight")})} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_msg_retry_timeout")}>
                            <InputText value={util.format_timespan_ms(state.state.txn_msg_retry_timeout, {replace_u32max_with: __("ocpp.content.no_message_in_flight")})} />
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
                        <FormRow label={__("ocpp.content.is_connected")}>
                            <InputText value={__("ocpp.content.connection_state_since")(state.state.connected, util.timestamp_sec_to_date(state.state.connected_change_time, __("ocpp.content.never_connected_since_reboot")))} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.last_ping_sent")}>
                            <InputText value={util.format_timespan_ms(state.state.last_ping_sent, {replace_u32max_with: __("ocpp.content.no_ping_sent")})} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.pong_timeout")}>
                            <InputText value={util.format_timespan_ms(state.state.pong_timeout, {replace_u32max_with: __("ocpp.content.no_ping_sent")})} />
                        </FormRow>
                    </CollapsedSection>

                    <CollapsedSection label={__("ocpp.content.configuration")}>
                        {(Object.keys(state.configuration) as Array<keyof typeof state.configuration>).map((k, i) => (
                            ocpp_debug ?
                                <FormRow label={k.replace(/([a-z])([A-Z])/g, "$1\u00AD$2")}>
                                    <InputText value={state.configuration[k]}
                                            onValue={(v) => this.setState({configuration: {...this.state.configuration, [k]: v}})}
                                            onfocusout={() => API.call("ocpp/change_configuration", {
                                                    key: k,
                                                    value: state.configuration[k]
                                            }, "lalala")} />
                                </FormRow> :
                                <FormRow label={k.replace(/([a-z])([A-Z])/g, "$1\u00AD$2")}>
                                    <InputText value={state.configuration[k]} />
                                </FormRow>)
                        )}
                    </CollapsedSection>
                </ConfigForm>
            </SubPage>
        );
    }
}

render(<Ocpp />, $("#ocpp")[0]);

interface OcppStatusState {
    state: API.getType["ocpp/state"];
    config: API.getType["ocpp/config"];
}

export class OcppStatus extends Component<{}, OcppStatusState>
{
    constructor()
    {
        super();

        util.addApiEventListener('ocpp/state', () => {
            this.setState({state: API.get('ocpp/state')})
        });

        util.addApiEventListener('ocpp/config', () => {
            this.setState({config: API.get('ocpp/config')})
        });
    }

    getConnectionState() {
        // TODO: we need some mechanism to get access to backend module magic numbers/defines/enums.
        switch(this.state.state.charge_point_state) {
            case 0:
            case 1:
            case 3:
                return 0;
            case 2:
                return this.state.state.connected ? 1 : 0;
            case 4:
            case 5:
            case 6:
            default:
                return 2;

        }
    }

    getStatusPrefix() {
        let ocpp = this.state.state;
        switch (ocpp.charge_point_state) {
            case 0:
            case 1:
            case 3:
            case 4:
            case 5:
            case 7:
            case 8:
                return translate_unchecked(`ocpp.content.charge_point_state_${ocpp.charge_point_state}`);
        }

        // normal operation
        if (ocpp.charge_point_state == 2) {
            // no txn running
            if (ocpp.connector_state != 8)
                return translate_unchecked(`ocpp.content.connector_state_${ocpp.connector_state}`);

            if (ocpp.txn_with_invalid_id)
                return __("ocpp.content.txn_with_invalid_id");

            // txn running
            return translate_unchecked(`ocpp.content.status_${ocpp.connector_status}`);
        }
    }

    getStatusLine() {
        let result = this.getStatusPrefix();

        if (this.state.state.tag_timeout != 0xFFFFFFFF)
            result = __("ocpp.status.waiting_for_tag") + util.format_timespan_ms(this.state.state.tag_timeout)
        if (this.state.state.cable_timeout != 0xFFFFFFFF)
            result = __("ocpp.status.waiting_for_cable") + util.format_timespan_ms(this.state.state.cable_timeout)

        return result;
    }

    render(props: {}, state: OcppStatusState)
    {
        if (!util.render_allowed() || !state.config.enable)
            return <></>;

        return <>
                <FormRow label={__("ocpp.status.ocpp")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={this.getConnectionState()}
                        items={[
                            ["primary", __("ocpp.status.connecting")],
                            ["success", __("ocpp.status.connected")],
                            ["danger", __("ocpp.status.error")]
                        ]}/>
                </FormRow>

                <FormRow label={__("ocpp.status.status")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                        <InputText value={this.getStatusLine()} />
                </FormRow>

            </>;
    }
}

render(<OcppStatus />, $("#status-ocpp")[0]);

export function add_event_listeners(source: API.APIEventTarget) {}

export function init() {}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-ocpp").prop("hidden", !module_init.ocpp);
}
