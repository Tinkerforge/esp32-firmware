/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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
import { h, createRef, Fragment, Component, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { InputText } from "../../ts/components/input_text";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Activity } from "react-feather";
import { InputNumber } from "../../ts/components/input_number";
import { FormSeparator } from "../../ts/components/form_separator";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { UplotData, UplotWrapperB, UplotPath } from "../../ts/components/uplot_wrapper_2nd";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { InputFile } from "../../ts/components/input_file";
import { Button } from "react-bootstrap";

export function ISO15118Navbar() {
    return <NavbarItem name="iso15118" module="iso15118" title="ISO 15118" symbol={<Activity />} />;
}

type ISO15118Config = API.getType["iso15118/config"];

export class ISO15118 extends ConfigComponent<'iso15118/config', {}, {pib_downloading: boolean}> {
    uplot_loader_ref        = createRef();
    uplot_wrapper_ref       = createRef();

    constructor() {
        super('iso15118/config',
              () => __("iso15118.script.save_failed"),
              undefined,
              {pib_downloading: false} as any);

        util.addApiEventListener("iso15118/state_slac", () => {
            // Update chart every time new price data comes in
            this.update_uplot();
        });
    }

    async pib_download() {
        this.setState({pib_downloading: true});
        try {
            const blob = await util.download("/iso15118/pib_read", true, 15 * 1000);
            const buffer = await blob.arrayBuffer();
            util.downloadToFile(new Uint8Array(buffer), "qca7000.pib", "application/octet-stream");
        } catch (e) {
            util.add_alert("pib_download_failed", "danger",
                () => __("iso15118.script.pib_download_failed"),
                () => e.toString());
        } finally {
            this.setState({pib_downloading: false});
        }
    }

    async pib_poll_write_status() {
        // Poll write status until complete or error
        for (let i = 0; i < 300; i++) { // 300 * 500ms = 150s max
            await new Promise(resolve => setTimeout(resolve, 500));
            try {
                const blob = await util.download("/iso15118/pib_write_status", true, 5000);
                const text = await blob.text();
                if (text === "complete") {
                    util.add_alert("pib_upload_success", "success",
                        () => __("iso15118.script.pib_upload_success"),
                        () => "");
                    return;
                }
                if (text.startsWith("error")) {
                    util.add_alert("pib_write_failed", "danger",
                        () => __("iso15118.script.pib_write_failed"),
                        () => text);
                    return;
                }
                if (text === "idle") {
                    // No operation in progress (unexpected), stop polling
                    return;
                }
                // Still in_progress, continue polling
            } catch (e) {
                util.add_alert("pib_write_failed", "danger",
                    () => __("iso15118.script.pib_write_failed"),
                    () => e.toString());
                return;
            }
        }
        util.add_alert("pib_write_failed", "danger",
            () => __("iso15118.script.pib_write_failed"),
            () => "Timed out waiting for write to complete");
    }

    update_uplot() {
        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        const state_slac = API.get('iso15118/state_slac');

        const data: UplotData = {
            keys: [null, 'index'],
            names: [null, __("iso15118.content.attenuation")],
            values: [[], []],
            stacked: [null, true],
            paths: [null, UplotPath.Bar],
            // Only enable the electricity price by default.
            // The chart with only electricity price is the most useful in most cases.
            default_visibilty: [null, true],
            lines_vertical: []
        };

        for (let i = 0; i < state_slac.attenuation_profile.length; i++) {
            data.values[0].push(i);
            data.values[1].push(state_slac.attenuation_profile[i]);
        }
        data.values[0].push(state_slac.attenuation_profile.length);
        data.values[1].push(0);

        // Show loader or data depending on the availability of data
        this.uplot_loader_ref.current.set_data(data && data.keys.length > 1);
        this.uplot_wrapper_ref.current.set_data(data);
    }

    render(props: {}, state: Readonly<ISO15118Config>) {
        if (!util.render_allowed())
            return <SubPage name="iso15118" />;

        const state_slac     = API.get('iso15118/state_slac');
        const state_sdp      = API.get('iso15118/state_sdp');
        const state_common   = API.get('iso15118/state_common');
        const state_din70121 = API.get('iso15118/state_din70121');
        const state_iso2     = API.get('iso15118/state_iso2');
        const state_iso20    = API.get('iso15118/state_iso20');
        const seen_macs      = API.get('ev/seen_macs');

        const sdp_state_names: {[key: number]: string} = {
            0: __("iso15118.content.sdp_state_idle"),
            1: __("iso15118.content.sdp_state_listening"),
            2: __("iso15118.content.sdp_state_discovery_completed"),
        };

        const common_state_names: {[key: number]: string} = {
            0: __("iso15118.content.common_state_idle"),
            1: __("iso15118.content.common_state_protocol_negotiated"),
        };

        const encryption_names: {[key: number]: string} = {
            0: __("iso15118.content.encryption_unencrypted"),
            1: __("iso15118.content.encryption_tls_1_2"),
            2: __("iso15118.content.encryption_tls_1_3"),
        };

        const slac_state_names: {[key: number]: string} = {
            0: __("iso15118.content.slac_state_modem_reset"),
            1: __("iso15118.content.slac_state_modem_initialization"),
            2: __("iso15118.content.slac_state_cm_set_key_request"),
            3: __("iso15118.content.slac_state_wait_for_cm_set_key_confirmation"),
            4: __("iso15118.content.slac_state_cm_qualcomm_get_sw_request"),
            5: __("iso15118.content.slac_state_wait_for_cm_qualcomm_get_sw_response"),
            6: __("iso15118.content.slac_state_cm_qualcomm_link_status_request"),
            7: __("iso15118.content.slac_state_wait_for_cm_qualcomm_link_status_response"),
            8: __("iso15118.content.slac_state_cm_qualcomm_op_attr_request"),
            9: __("iso15118.content.slac_state_wait_for_cm_qualcomm_op_attr_response"),
            10: __("iso15118.content.slac_state_wait_for_slac_param_request"),
            11: __("iso15118.content.slac_state_wait_for_start_atten_char_indication"),
            12: __("iso15118.content.slac_state_wait_for_mnbc_sound"),
            13: __("iso15118.content.slac_state_wait_for_atten_char"),
            14: __("iso15118.content.slac_state_wait_for_slac_match"),
            15: __("iso15118.content.slac_state_wait_for_sdp"),
            16: __("iso15118.content.slac_state_link_detected"),
            17: __("iso15118.content.slac_state_modem_disabled"),
            18: __("iso15118.content.slac_state_slac_init_ef"),
            19: __("iso15118.content.slac_state_slac_init_failed"),
        };

        const din70121_state_names: {[key: number]: string} = {
            0: __("iso15118.content.din70121_state_idle"),
            1: __("iso15118.content.din70121_state_bitstream_received"),
            2: __("iso15118.content.din70121_state_session_setup"),
            3: __("iso15118.content.din70121_state_service_discovery"),
            4: __("iso15118.content.din70121_state_service_payment_selection"),
            5: __("iso15118.content.din70121_state_contract_authentication"),
            6: __("iso15118.content.din70121_state_charge_parameter_discovery"),
            7: __("iso15118.content.din70121_state_session_stop"),
            8: __("iso15118.content.din70121_state_cable_check"),
            9: __("iso15118.content.din70121_state_power_delivery"),
            10: __("iso15118.content.din70121_state_pre_charge"),
            11: __("iso15118.content.din70121_state_current_demand"),
        };

        const iso2_state_names: {[key: number]: string} = {
            0: __("iso15118.content.iso2_state_idle"),
            1: __("iso15118.content.iso2_state_bitstream_received"),
            2: __("iso15118.content.iso2_state_session_setup"),
            3: __("iso15118.content.iso2_state_service_discovery"),
            4: __("iso15118.content.iso2_state_payment_service_selection"),
            5: __("iso15118.content.iso2_state_authorization"),
            6: __("iso15118.content.iso2_state_charge_parameter_discovery"),
            7: __("iso15118.content.iso2_state_power_delivery"),
            8: __("iso15118.content.iso2_state_charging_status"),
            9: __("iso15118.content.iso2_state_session_stop"),
            10: __("iso15118.content.iso2_state_cable_check"),
        };

        const iso20_state_names: {[key: number]: string} = {
            0: __("iso15118.content.iso20_state_idle"),
            1: __("iso15118.content.iso20_state_bitstream_received"),
            2: __("iso15118.content.iso20_state_session_setup"),
            3: __("iso15118.content.iso20_state_authorization_setup"),
            4: __("iso15118.content.iso20_state_authorization"),
            5: __("iso15118.content.iso20_state_service_discovery"),
            6: __("iso15118.content.iso20_state_service_detail"),
            7: __("iso15118.content.iso20_state_service_selection"),
            8: __("iso15118.content.iso20_state_schedule_exchange"),
            9: __("iso15118.content.iso20_state_power_delivery_start"),
            10: __("iso15118.content.iso20_state_power_delivery_stop"),
            11: __("iso15118.content.iso20_state_session_stop_terminate"),
            12: __("iso15118.content.iso20_state_session_stop_pause"),
            13: __("iso15118.content.iso20_state_ac_charge_parameter_discovery"),
            14: __("iso15118.content.iso20_state_ac_charge_loop"),
        };

        const array8_to_hexstring = (array8: number[], separator: string) => {
            return array8.map(value => ('0' + value.toString(16).toUpperCase()).slice(-2)).join(separator);
        };

        const array32_to_ipv6string = (array32: number[]) => {
            const array16: number[] = [];
            for (let i = 0; i < 4; i++) {
                const val1 = array32[i] & 0xffff
                array16.push((val1 >> 8) | ((val1 & 0xFF) << 8)); // Lower 16 bits
                const val2 = (array32[i] >>> 16) & 0xffff;
                array16.push((val2 >> 8) | ((val2 & 0xFF) << 8)); // Upper 16 bits
            }
            return array16.map(value => ('000' + value.toString(16).toUpperCase()).slice(-4)).join(':');
        };

        const security_value_to_string = (value: number) => {
            if (value == 0x00) {
                return __("iso15118.content.security_tls");
            } else if (value == 0x10) {
                return __("iso15118.content.security_none");
            }  else if (value < 0) {
                return "-";
            } else {
                return __("iso15118.content.unknown");
            }
        };

        const transport_protocol_to_string = (value: number) => {
            if (value == 0x00) {
                return __("iso15118.content.transport_tcp");
            } else if (value == 0x10) {
                return __("iso15118.content.transport_udp");
            } else if (value < 0) {
                return "-";
            } else {
                return __("iso15118.content.unknown");
            }
        };

        const uptime = API.get("info/keep_alive").uptime;

        return (
            <SubPage name="iso15118" title="ISO 15118">
                <SubPage.Status>
                    <FormRow label={__("iso15118.content.slac_state")} label_muted={__("iso15118.content.slac_state_muted")}>
                        <InputText value={slac_state_names[state_slac.state] ?? state_slac.state.toString()}/>
                    </FormRow>
                    <FormRow label={__("iso15118.content.protocol_state")} label_muted={
                            state_din70121.state !== 0 ? "DIN 70121" :
                            state_iso2.state !== 0 ? "ISO 15118-2" :
                            state_iso20.state !== 0 ? "ISO 15118-20" :
                            undefined
                        }>
                        <InputText value={
                            state_din70121.state !== 0 ? (din70121_state_names[state_din70121.state] ?? state_din70121.state.toString()) :
                            state_iso2.state !== 0 ? (iso2_state_names[state_iso2.state] ?? state_iso2.state.toString()) :
                            state_iso20.state !== 0 ? (iso20_state_names[state_iso20.state] ?? state_iso20.state.toString()) :
                            "-"
                        }/>
                    </FormRow>

                    {seen_macs.length > 0 &&
                        <FormRow label={__("iso15118.content.seen_macs")}>
                            <div class="row gx-2 gy-1">
                                {seen_macs
                                    .slice()
                                    .sort((a, b) => b.seen_at - a.seen_at)
                                    .map((entry, i) =>
                                        <Fragment key={i}>
                                            <div class="col-6">
                                                <InputText value={entry.mac}/>
                                            </div>
                                            <div class="col-6">
                                                <InputText value={util.format_timespan_ms(uptime - entry.seen_at)}/>
                                            </div>
                                        </Fragment>
                                    )
                                }
                            </div>
                        </FormRow>
                    }

                    <CollapsedSection>
                        <FormSeparator heading={__("iso15118.content.slac_section")} first/>
                        <FormRow label={__("iso15118.content.state")}>
                            <InputText value={slac_state_names[state_slac.state] ?? state_slac.state.toString()}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.modem_found")}>
                            <InputText value={state_slac.modem_found ? __("iso15118.content.yes") : __("iso15118.content.no") }/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.modem_initialization_tries")}>
                            <InputNumber value={state_slac.modem_initialization_tries}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.atten_char_indication_tries")}>
                            <InputNumber value={state_slac.atten_char_indication_tries}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.received_sounds")}>
                            <InputNumber value={state_slac.received_sounds}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.received_aag_lists")}>
                            <InputNumber value={state_slac.received_aag_lists}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.attenuation_profile")}>
                            <div style="position: relative;">
                                <UplotLoader
                                    ref={this.uplot_loader_ref}
                                    show
                                    marker_class="h4"
                                    no_data={__("iso15118.content.no_data")}
                                    loading={__("iso15118.content.loading")}>
                                    <UplotWrapperB
                                        ref={this.uplot_wrapper_ref}
                                        class="attenuation-profile-chart"
                                        sub_page="iso15118"
                                        color_cache_group="iso15118.default"
                                        show
                                        on_mount={() => this.update_uplot()}
                                        legend_time_label={__("iso15118.content.index")}
                                        legend_time_with_minutes={false}
                                        legend_show={false}
                                        aspect_ratio={3}
                                        x_format={null}
                                        x_padding_factor={0}
                                        x_include_date={false}
                                        y_min={0}
                                        y_label={__("iso15118.content.attenuation_db")}
                                        y_unit="dB"
                                        y_digits={2}
                                        padding={[null, null, null, null]}
                                    />
                                </UplotLoader>
                            </div>
                        </FormRow>
                        <FormRow>
                            <div class="row gx-2 gy-1">
                                <div class="col-12"><InputText value={state_slac.attenuation_profile.slice(0,  16).toString()}/></div>
                                <div class="col-12"><InputText value={state_slac.attenuation_profile.slice(16, 32).toString()}/></div>
                                <div class="col-12"><InputText value={state_slac.attenuation_profile.slice(32, 48).toString()}/></div>
                                <div class="col-12"><InputText value={state_slac.attenuation_profile.slice(48, 58).toString()}/></div>
                            </div>
                        </FormRow>
                        <FormRow label={__("iso15118.content.nmk")}>
                            <InputText value={array8_to_hexstring(state_slac.nmk, '')}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.nid")}>
                            <InputText value={array8_to_hexstring(state_slac.nid, '')}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.evse_mac")}>
                            <InputText value={array8_to_hexstring(state_slac.evse_mac, ':')}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.evse_mac_modem")}>
                            <InputText value={array8_to_hexstring(state_slac.evse_mac_modem, ':')}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.pev_mac")}>
                            <InputText value={array8_to_hexstring(state_slac.pev_mac, ':')}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.pev_mac_modem")}>
                            <InputText value={array8_to_hexstring(state_slac.pev_mac_modem, ':')}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.pev_run_id")}>
                            <InputText value={array8_to_hexstring(state_slac.pev_run_id, '')}/>
                        </FormRow>

                        <FormSeparator heading={__("iso15118.content.sdp_section")}/>
                        <FormRow label={__("iso15118.content.state")}>
                            <InputText value={sdp_state_names[state_sdp.state] ?? state_sdp.state.toString()}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.evse_ip_address")}>
                            <InputText value={array32_to_ipv6string(state_sdp.evse_ip_address)}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.evse_port")}>
                            <InputNumber value={state_sdp.evse_port}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.evse_security")}>
                            <InputText value={security_value_to_string(state_sdp.evse_security)}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.evse_transport_protocol")}>
                            <InputText value={transport_protocol_to_string(state_sdp.evse_tranport_protocol)}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.ev_security")}>
                            <InputText value={security_value_to_string(state_sdp.ev_security)}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.ev_transport_protocol")}>
                            <InputText value={transport_protocol_to_string(state_sdp.ev_tranport_protocol)}/>
                        </FormRow>

                        <FormSeparator heading={__("iso15118.content.din_iso_section")}/>
                        <FormRow label={__("iso15118.content.state")}>
                            <InputText value={common_state_names[state_common.state] ?? state_common.state.toString()}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.supported_protocols")}>
                            <InputText value={state_common.supported_protocols.join(__("iso15118.content.protocol_separator"))}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.protocol")}>
                            <InputText value={state_common.protocol}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.encryption")}>
                            <InputText value={encryption_names[state_common.encryption] ?? state_common.encryption.toString()}/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.protocol_state")}>
                            <InputText value={
                                state_din70121.state !== 0 ? (din70121_state_names[state_din70121.state] ?? state_din70121.state.toString()) :
                                state_iso2.state !== 0 ? (iso2_state_names[state_iso2.state] ?? state_iso2.state.toString()) :
                                state_iso20.state !== 0 ? (iso20_state_names[state_iso20.state] ?? state_iso20.state.toString()) :
                                "-"
                            }/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.session_id")}>
                            <InputText value={
                                state_din70121.state !== 0 ? array8_to_hexstring(state_din70121.session_id, '') :
                                state_iso2.state !== 0 ? array8_to_hexstring(state_iso2.session_id, '') :
                                state_iso20.state !== 0 ? (state_iso20.session_id || "-") :
                                "-"
                            }/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.evcc_id")}>
                            <InputText value={
                                state_din70121.state !== 0 ? array8_to_hexstring(state_din70121.evcc_id, '') :
                                state_iso2.state !== 0 ? array8_to_hexstring(state_iso2.evcc_id, '') :
                                state_iso20.state !== 0 ? (state_iso20.evcc_id || "-") :
                                "-"
                            }/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.state_of_charge")}>
                            <InputText value={
                                state_din70121.state !== 0 ? state_din70121.soc + "%" :
                                state_iso2.state !== 0 ? (state_iso2.soc < 0 ? "-" : state_iso2.soc + "%") :
                                state_iso20.state !== 0 ? (state_iso20.soc < 0 ? "-" : state_iso20.soc + "%") :
                                "-"
                            }/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.energy_capacity")}>
                            <InputText value={
                                state_iso2.state !== 0 ? (state_iso2.energy_capacity < 0 ? "-" : state_iso2.energy_capacity + " Wh") :
                                state_iso20.state !== 0 ? (state_iso20.energy_capacity < 0 ? "-" : state_iso20.energy_capacity + " Wh") :
                                "-"
                            }/>
                        </FormRow>
                        <FormRow label={__("iso15118.content.active_power")}>
                            <InputText value={
                                state_iso20.state !== 0 ? (state_iso20.active_power < 0 ? "-" : state_iso20.active_power + " W") :
                                "-"
                            }/>
                        </FormRow>
                    </CollapsedSection>
                </SubPage.Status>

                <SubPage.Config
                    id="iso15118_config_form"
                    isDirty={this.isDirty()}
                    onSave={this.save}
                    onDirtyChange={this.setDirty}>
                    <FormRow label={__("iso15118.content.autocharge")} help={__("iso15118.content.autocharge_help")}>
                        <Switch desc={__("iso15118.content.autocharge_desc")}
                                checked={state.autocharge}
                                onClick={this.toggle('autocharge')}
                        />
                    </FormRow>
                    <FormRow label={__("iso15118.content.read_soc")} help={__("iso15118.content.read_soc_help")}>
                        <Switch desc={__("iso15118.content.read_soc_desc")}
                                checked={API.hasFeature("meter") && state.read_soc}
                                onClick={this.toggle('read_soc')}
                                disabled={!API.hasFeature("meter")}
                        />
                    </FormRow>
                    <FormRow label={__("iso15118.content.charge_via_iso15118")} help={__("iso15118.content.charge_via_iso15118_help")}>
                        <Switch desc={__("iso15118.content.charge_via_iso15118_desc")}
                                checked={false}
                                onClick={this.toggle('charge_via_iso15118')}
                                disabled={true}
                        />
                    </FormRow>
                    <FormRow label={__("iso15118.content.min_charge_current")} help={__("iso15118.content.min_charge_current_help")}>
                        <InputNumber
                            unit="mA"
                            value={state.min_charge_current}
                            onValue={this.set('min_charge_current')}
                            min={1000}
                            max={22000}
                            disabled={true}
                        />
                    </FormRow>
                    <CollapsedSection heading={__("iso15118.content.advanced_settings")}>
                        <FormRow label={__("iso15118.content.pib_download")} label_muted={__("iso15118.content.pib_download_desc")}>
                            <Button variant="primary" className="w-100"
                                disabled={this.state.pib_downloading}
                                onClick={() => this.pib_download()}>
                                {this.state.pib_downloading ? "..." : __("iso15118.content.pib_download")}
                            </Button>
                        </FormRow>
                        <FormRow label={__("iso15118.content.pib_upload")} label_muted={__("iso15118.content.pib_upload_desc")}>
                            <InputFile
                                url="/iso15118/pib_write"
                                upload={__("iso15118.content.pib_upload_button")}
                                accept=".pib,.bin"
                                timeout_ms={30 * 1000}
                                contentType="application/octet-stream"
                                onUploadSuccess={() => this.pib_poll_write_status()}
                                onUploadError={(error) => {
                                    util.add_alert("pib_upload_failed", "danger",
                                        () => __("iso15118.script.pib_upload_failed"),
                                        () => error.toString());
                                }}
                            />
                        </FormRow>
                    </CollapsedSection>
                </SubPage.Config>
            </SubPage>
        );
    }
}

export function pre_init() {
}

export function init() {
}
