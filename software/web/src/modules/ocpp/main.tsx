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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { h, Component, Fragment, RefObject } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { ConfigForm } from "../../ts/components/config_form";
import { ConfigComponent } from "../../ts/components/config_component";
import { Button } from "react-bootstrap";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { InputPassword } from "../../ts/components/input_password";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { SubPage } from "../../ts/components/sub_page";
import { InputSelect } from "../../ts/components/input_select";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";

export function OcppNavbar() {
    return (
        <NavbarItem name="ocpp" module="ocpp" title={__("ocpp.navbar.ocpp")} symbol={
            <svg height="24" width="24" stroke="currentColor" fill="currentColor" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" style="stroke-width:34.036;"><path d="M192.5488 275.1058c-13.6006 0-26.9936-1.0551-39.8068-3.1362-12.3047-1.9984-24.3433-4.986-35.7814-8.88-21.6536-7.3714-41.5725-18.0794-59.2034-31.8264-16.4378-12.8168-30.649-28.064-42.239-45.3182-5.513-8.2072-10.4153-16.8686-14.5708-25.7436-4.0748-8.7024-7.4743-17.7006-10.1043-26.7445-2.6464-9.1006-4.3186-18.3851-4.9701-27.5958-.6458-9.1308-.2907-18.2117 1.0554-26.9905 1.3513-8.812 3.7044-17.3389 6.9938-25.3439 3.342-8.1331 7.6638-15.7569 12.8451-22.6595 4.933-6.5719 10.6853-12.5392 17.097-17.7362 6.6207-5.3663 14.027-9.9767 22.013-13.703 17.0246-7.9438 36.7265-11.9717 58.5585-11.9717 1.3972 0 2.7637.016 4.1772.0487 12.0177.2783 24.9814 1.5164 38.5311 3.68 13.3235 2.1276 27.2083 5.1483 41.2688 8.9783 14.0181 3.8184 28.1831 8.4336 42.1015 13.7173 14.0759 5.3435 27.881 11.3642 41.0318 17.8949 13.4728 6.6906 26.2783 13.9272 38.0608 21.5088 12.196 7.8477 23.3624 16.1095 33.189 24.556 10.2444 8.8055 19.1326 17.8998 26.4178 27.0303 7.6264 9.5582 13.6004 19.2896 17.756 28.924 2.5169 5.835 4.2659 11.7344 5.1985 17.5343.9042 5.6233 1.0621 11.284.4695 16.8248-1.1394 10.6526-5.0555 21.1296-11.6395 31.1398-7.2698 11.053-17.7132 21.467-31.0401 30.9524-12.8257 9.1288-28.0076 17.1855-45.1242 23.9465-16.887 6.6703-35.1079 11.866-54.1566 15.4425-19.334 3.6303-38.8914 5.471-58.1286 5.471z" style="stroke-width:34.036;" transform="translate(1.7265 4.153) scale(.05426)"/><path d="M104.4358-12.0437c-21.7584 0-41.3892 4.012-58.347 11.9247C38.1396 3.59 30.7682 8.1788 24.179 13.5195c-6.3799 5.1711-12.1035 11.1087-17.012 17.6478C2.0114 38.036-2.2893 45.6228-5.6152 53.7167c-3.2744 7.9684-5.6168 16.4569-6.962 25.2296-1.3406 8.7423-1.6942 17.7858-1.051 26.8795.649 9.1754 2.315 18.4249 4.9515 27.4915 2.6228 9.0192 6.0131 17.993 10.077 26.672 4.1446 8.852 9.0342 17.4909 14.533 25.677 11.5605 17.21 25.7355 32.4185 42.1314 45.2026 17.5868 13.7126 37.4565 24.394 59.057 31.7474 11.412 3.885 23.4233 6.8658 35.7006 8.8598 12.7867 2.0767 26.1527 3.1297 39.7266 3.1297 19.2064 0 38.7326-1.8378 58.0364-5.4622 19.0175-3.5708 37.2077-8.7576 54.0651-15.4163 17.079-6.746 32.2252-14.7834 45.018-23.8887 13.2762-9.4495 23.6766-19.8188 30.9123-30.8199 6.5399-9.943 10.4292-20.3454 11.56-30.9183.588-5.4966.4312-11.1127-.466-16.692-.926-5.7591-2.6634-11.6187-5.164-17.4159-4.138-9.5935-10.089-19.2866-17.6877-28.81-7.2657-9.1062-16.132-18.1778-26.3528-26.9631-9.8092-8.4315-20.957-16.6793-33.1337-24.5146-11.767-7.5717-24.5563-14.799-38.0126-21.4815-13.1362-6.5234-26.9263-12.5375-40.9869-17.8752C216.4336 9.07 202.2841 4.4599 188.2815.6457c-14.0433-3.8253-27.9104-6.8422-41.2162-8.9669-13.5275-2.16-26.4687-3.3962-38.4639-3.674a178.4206 178.4206 0 0 0-4.1656-.0485m0-1c1.3885 0 2.7768.0161 4.1888.0488 97.612 2.2605 244.5038 67.0692 278.8043 146.5914s-83.216 142.0093-194.8801 142.0093S11.344 205.7467-9.6371 133.5965C-30.3146 62.49 9.8332-13.0437 104.4358-13.0437Z" style="stroke-width:34.036;" transform="translate(1.7265 4.153) scale(.05426)"/></g><path d="M3304.846-1220.153c-87.7 98.242 24.527 222.824 23.83 222.877-113.552-29.289-119.295-125.434-119.295-125.434s-16.281-101.54 95.465-97.443z" style="stroke:none" transform="translate(-171.3407 71.1169) scale(.05426)"/></svg>
        } />);
}

type OcppConfig = API.getType["ocpp/config"];

interface OcppState {
    state: API.getType['ocpp/state'];
    configuration: API.getType['ocpp/configuration'];
}

export class Ocpp extends ConfigComponent<'ocpp/config', {status_ref?: RefObject<OcppStatus>}, OcppState> {
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
            return <SubPage name="ocpp" />;

        // This is not an official feature.
        let ocpp_debug = API.hasFeature_unchecked("ocpp_debug");
        let msg_in_flight = state.state.message_in_flight_id_high != 0 || state.state.message_in_flight_id_low != 0;

        return (
            <SubPage name="ocpp">
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
                        <FormRow label={__("ocpp.content.last_rejected_tag_reason")}>
                            <InputText value={__("ocpp.content.last_rejected_tag")(state.state.last_rejected_tag, state.state.last_rejected_tag_reason)} />
                        </FormRow>
                        <FormRow label={__("ocpp.content.txn_id")}>
                            <InputText value={state.state.txn_id == 0x7FFFFFFF ?  __("ocpp.content.no_transaction_running") : state.state.txn_id} />
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

interface OcppStatusState {
    state: API.getType["ocpp/state"];
    config: API.getType["ocpp/config"];
}

export class OcppStatus extends Component<{}, OcppStatusState> {
    constructor() {
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
        switch (this.state.state.charge_point_state) {
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
        let result = ""

        if (this.state.state.last_rejected_tag != "")
            result += __("ocpp.content.last_rejected_tag")(this.state.state.last_rejected_tag, this.state.state.last_rejected_tag_reason) + ". "

        if (this.state.state.tag_timeout != 0xFFFFFFFF)
            result += __("ocpp.status.waiting_for_tag") + util.format_timespan_ms(this.state.state.tag_timeout) + ". "
        if (this.state.state.cable_timeout != 0xFFFFFFFF)
            result += __("ocpp.status.waiting_for_cable") + util.format_timespan_ms(this.state.state.cable_timeout) + ". "

        result += this.getStatusPrefix();

        return result;
    }

    render(props: {}, state: OcppStatusState) {
        if (!util.render_allowed() || !state.config.enable)
            return <StatusSection name="ocpp" />;

        return <StatusSection name="ocpp">
                <FormRow label={__("ocpp.status.ocpp")}>
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

                <FormRow label={__("ocpp.status.status")}>
                        <InputText value={this.getStatusLine()} />
                </FormRow>
            </StatusSection>;
    }
}

export function init() {
}
