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
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { InputText } from "../../ts/components/input_text";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Activity } from "react-feather";
import { InputNumber } from "../../ts/components/input_number";
import { FormSeparator } from "../../ts/components/form_separator";
import { UplotData, UplotWrapperB, UplotPath } from "../../ts/components/uplot_wrapper_2nd";
import { UplotLoader } from "../../ts/components/uplot_loader";

export function ISO15118Navbar() {
    return <NavbarItem name="iso15118" module="iso15118" title="ISO15118" symbol={<Activity />} />;
}

type ISO15118Config = API.getType["iso15118/config"];

export class ISO15118 extends ConfigComponent<'iso15118/config', {}> {
    uplot_loader_ref        = createRef();
    uplot_wrapper_ref       = createRef();

    constructor() {
        super('iso15118/config',
              () => __("iso15118.script.save_failed"));

        util.addApiEventListener("iso15118/state_slac", () => {
            // Update chart every time new price data comes in
            this.update_uplot();
        });
    }

    update_uplot() {
        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        const state_slac = API.get('iso15118/state_slac');

        const data: UplotData = {
            keys: [null, 'index'],
            names: [null, 'Attenuation'],
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

        const sdp_state_names: {[key: number]: string} = {
            0: "Idle",
            1: "Listening",
            2: "Discovery Completed",
        };

        const common_state_names: {[key: number]: string} = {
            0: "Idle",
            1: "Protocol Negotiated",
        };

        const slac_state_names: {[key: number]: string} = {
            0: "Modem Reset",
            1: "Modem Initialization",
            2: "CM Set Key Request",
            3: "Wait For CM Set Key Confirmation",
            4: "CM Qualcomm Get Sw Request",
            5: "Wait For CM Qualcomm Get Sw Response",
            6: "CM Qualcomm Link Status Request",
            7: "Wait For CM Qualcomm Link Status Response",
            8: "CM Qualcomm Op Attr Request",
            9: "Wait For CM Qualcomm Op Attr Response",
            10: "Wait For Slac Param Request",
            11: "Wait For Start Atten Char Indication",
            12: "Wait For MNBC Sound",
            13: "Wait For Atten Char",
            14: "Wait For Slac Match",
            15: "Wait For SDP",
            16: "Link Detected",
        };

        const din70121_state_names: {[key: number]: string} = {
            0: "Idle",
            1: "Bitstream Received",
            2: "Session Setup",
            3: "Service Discovery",
            4: "Service Payment Selection",
            5: "Contract Authentication",
            6: "Charge Parameter Discovery",
            7: "Session Stop",
            8: "Cable Check",
        };

        const iso2_state_names: {[key: number]: string} = {
            0: "Idle",
            1: "Bitstream Received",
            2: "Session Setup",
            3: "Service Discovery",
            4: "Payment Service Selection",
            5: "Authorization",
            6: "Charge Parameter Discovery",
            7: "Power Delivery",
            8: "Charging Status",
            9: "Session Stop",
            10: "Cable Check",
        };

        const iso20_state_names: {[key: number]: string} = {
            0: "Idle",
            1: "Bitstream Received",
            2: "Session Setup",
            3: "Authorization Setup",
            4: "Authorization",
            5: "Service Discovery",
            6: "Service Detail",
            7: "Service Selection",
            8: "Schedule Exchange",
            9: "Power Delivery Start",
            10: "Power Delivery Stop",
            11: "Session Stop Terminate",
            12: "Session Stop Pause",
            13: "AC Charge Parameter Discovery",
            14: "AC Charge Loop",
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
                return "TLS";
            } else if (value == 0x10) {
                return "No transport layer security";
            }  else if (value < 0) {
                return "-";
            } else {
                return "Unknown";
            }
        };

        const transport_protocol_to_string = (value: number) => {
            if (value == 0x00) {
                return "TCP";
            } else if (value == 0x10) {
                return "UDP";
            } else if (value < 0) {
                return "-";
            } else {
                return "Unknown";
            }
        };

        return (
            <SubPage name="iso15118">
                <ConfigForm id="iso15118_config_form" title="ISO 15118" isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormSeparator heading={__("iso15118.content.configuration")}/>
                    <FormRow label={__("iso15118.content.autocharge")}>
                        <Switch desc={__("iso15118.content.autocharge_desc")}
                                checked={state.autocharge}
                                onClick={this.toggle('autocharge')}
                        />
                    </FormRow>
                    <FormRow label={__("iso15118.content.read_soc")}>
                        <Switch desc={__("iso15118.content.read_soc_desc")}
                                checked={state.read_soc}
                                onClick={this.toggle('read_soc')}
                        />
                    </FormRow>
                    <FormRow label={__("iso15118.content.charge_via_iso15118")}>
                        <Switch desc={__("iso15118.content.charge_via_iso15118_desc")}
                                checked={state.charge_via_iso15118}
                                onClick={this.toggle('charge_via_iso15118')}
                        />
                    </FormRow>
                    <FormRow label={__("iso15118.content.min_charge_current")}>
                        <InputNumber
                            unit="mA"
                            value={state.min_charge_current}
                            onValue={this.set('min_charge_current')}
                            min={1000}
                            max={22000}
                        />
                    </FormRow>
                    <FormSeparator heading="Signal Level Attenuation Characterisation (SLAC)"/>
                    <FormRow label="State">
                        <InputText value={slac_state_names[state_slac.state] ?? state_slac.state.toString()}/>
                    </FormRow>
                    <FormRow label="Modem Found">
                        <InputText value={state_slac.modem_found ? "Yes" : "No" }/>
                    </FormRow>
                    <FormRow label="Modem Initialization Tries">
                        <InputNumber value={state_slac.modem_initialization_tries}/>
                    </FormRow>
                    <FormRow label="Atten Char Indication Tries">
                        <InputNumber value={state_slac.atten_char_indication_tries}/>
                    </FormRow>
                    <FormRow label="Received Sounds">
                        <InputNumber value={state_slac.received_sounds}/>
                    </FormRow>
                    <FormRow label="Received AAG Lists">
                        <InputNumber value={state_slac.received_aag_lists}/>
                    </FormRow>
                    <FormRow label="Attenuation Profile">
                        <div style="position: relative;">
                            <UplotLoader
                                ref={this.uplot_loader_ref}
                                show={true}
                                marker_class={'h4'}
                                no_data={"No Data"}
                                loading={"Loading"}>
                                <UplotWrapperB
                                    ref={this.uplot_wrapper_ref}
                                    class="attenuation-profile-chart"
                                    sub_page="iso15118"
                                    color_cache_group="iso15118.default"
                                    show={true}
                                    on_mount={() => this.update_uplot()}
                                    legend_time_label={"Index"}
                                    legend_time_with_minutes={false}
                                    legend_show={false}
                                    aspect_ratio={3}
                                    x_format={null}
                                    x_padding_factor={0}
                                    x_include_date={false}
                                    y_min={0}
                                    y_label={"Attenuation (dB)"}
                                    y_unit="dB"
                                    y_digits={2}
                                    only_show_visible={true}
                                    padding={[null, null, null, null]}
                                />
                            </UplotLoader>
                        </div>
                    </FormRow>
                    <FormRow>
                        <InputText value={state_slac.attenuation_profile.slice(0,  16).toString()}/>
                        <InputText value={state_slac.attenuation_profile.slice(16, 32).toString()}/>
                        <InputText value={state_slac.attenuation_profile.slice(32, 48).toString()}/>
                        <InputText value={state_slac.attenuation_profile.slice(48, 58).toString()}/>
                    </FormRow>
                    <FormRow label="NMK">
                        <InputText value={array8_to_hexstring(state_slac.nmk, '')}/>
                    </FormRow>
                    <FormRow label="NID">
                        <InputText value={array8_to_hexstring(state_slac.nid, '')}/>
                    </FormRow>
                    <FormRow label="EVSE MAC">
                        <InputText value={array8_to_hexstring(state_slac.evse_mac, ':')}/>
                    </FormRow>
                    <FormRow label="EVSE MAC Modem">
                        <InputText value={array8_to_hexstring(state_slac.evse_mac_modem, ':')}/>
                    </FormRow>
                    <FormRow label="PEV MAC">
                        <InputText value={array8_to_hexstring(state_slac.pev_mac, ':')}/>
                    </FormRow>
                    <FormRow label="PEV MAC Modem">
                        <InputText value={array8_to_hexstring(state_slac.pev_mac_modem, ':')}/>
                    </FormRow>
                    <FormRow label="PEV Run ID">
                        <InputText value={array8_to_hexstring(state_slac.pev_run_id, '')}/>
                    </FormRow>

                    <FormSeparator heading="SECC Discovery Protocol (SDP)"/>
                    <FormRow label="State">
                        <InputText value={sdp_state_names[state_sdp.state] ?? state_sdp.state.toString()}/>
                    </FormRow>
                    <FormRow label="EVSE IP Address">
                        <InputText value={array32_to_ipv6string(state_sdp.evse_ip_address)}/>
                    </FormRow>
                    <FormRow label="EVSE Port">
                        <InputNumber value={state_sdp.evse_port}/>
                    </FormRow>
                    <FormRow label="EVSE Security">
                        <InputText value={security_value_to_string(state_sdp.evse_security)}/>
                    </FormRow>
                    <FormRow label="EVSE Transport Protocol">
                        <InputText value={transport_protocol_to_string(state_sdp.evse_tranport_protocol)}/>
                    </FormRow>
                    <FormRow label="EV Security">
                        <InputText value={security_value_to_string(state_sdp.ev_security)}/>
                    </FormRow>
                    <FormRow label="EV Transport Protocol">
                        <InputText value={transport_protocol_to_string(state_sdp.ev_tranport_protocol)}/>
                    </FormRow>

                    <FormSeparator heading="Common"/>
                    <FormRow label="State">
                        <InputText value={common_state_names[state_common.state] ?? state_common.state.toString()}/>
                    </FormRow>
                    <FormRow label="Supported Protocols">
                        <InputText value={state_common.supported_protocols.join(' and ')}/>
                    </FormRow>
                    <FormRow label="Protocol">
                        <InputText value={state_common.protocol}/>
                    </FormRow>

                    <FormSeparator heading="DIN SPEC 70121"/>
                    <FormRow label="State">
                        <InputText value={din70121_state_names[state_din70121.state] ?? state_din70121.state.toString()}/>
                    </FormRow>
                    <FormRow label="Session ID">
                        <InputText value={array8_to_hexstring(state_din70121.session_id, '')}/>
                    </FormRow>
                    <FormRow label="EVCC ID">
                        <InputText value={array8_to_hexstring(state_din70121.evcc_id, '')}/>
                    </FormRow>
                    <FormRow label="State Of Charge">
                        <InputNumber value={state_din70121.soc}/>
                    </FormRow>
                    <FormRow label="EV Ready">
                        <InputNumber value={state_din70121.ev_ready}/>
                    </FormRow>
                    <FormRow label="EV Cabin Conditioning">
                        <InputNumber value={state_din70121.ev_cabin_conditioning}/>
                    </FormRow>
                    <FormRow label="EV Cabin Conditioning Is Used">
                        <InputText value={state_din70121.ev_cabin_conditioning_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="EV Ress Conditioning">
                        <InputNumber value={state_din70121.ev_ress_conditioning}/>
                    </FormRow>
                    <FormRow label="EV Ress Conditioning Is Used">
                        <InputText value={state_din70121.ev_ress_conditioning_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="EV Error Code">
                        <InputNumber value={state_din70121.ev_error_code}/>
                    </FormRow>
                    <FormRow label="EV Maximum Current Limit">
                        <InputText value={state_din70121.ev_max_current_limit_val + " * 10^" + state_din70121.ev_max_current_limit_mul}/>
                    </FormRow>
                    <FormRow label="EV Maximum Power Limit">
                        <InputText value={state_din70121.ev_max_power_limit_val + " * 10^" + state_din70121.ev_max_power_limit_mul}/>
                    </FormRow>
                    <FormRow label="EV Maximum Power Limit Is Used">
                        <InputText value={state_din70121.ev_max_power_limit_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="EV Maximum Voltage Limit">
                        <InputText value={state_din70121.ev_max_voltage_limit_val + " * 10^" + state_din70121.ev_max_voltage_limit_mul}/>
                    </FormRow>
                    <FormRow label="EV Energy Capacity">
                        <InputText value={state_din70121.ev_energy_capacity_val + " * 10^" + state_din70121.ev_energy_capacity_mul}/>
                    </FormRow>
                    <FormRow label="EV Energy Capacity Is Used">
                        <InputText value={state_din70121.ev_energy_capacity_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="EV Energy Request">
                        <InputText value={state_din70121.ev_energy_request_val + " * 10^" + state_din70121.ev_energy_request_mul}/>
                    </FormRow>
                    <FormRow label="EV Energy Request Is Used">
                        <InputText value={state_din70121.ev_energy_request_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Full SOC">
                        <InputNumber value={state_din70121.full_soc}/>
                    </FormRow>
                    <FormRow label="Full SOC Is Used">
                        <InputText value={state_din70121.full_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Bulk SOC">
                        <InputNumber value={state_din70121.bulk_soc}/>
                    </FormRow>
                    <FormRow label="Bulk SOC Is Used">
                        <InputText value={state_din70121.bulk_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>

                    <FormSeparator heading="ISO 15118-2"/>
                    <FormRow label="State">
                        <InputText value={iso2_state_names[state_iso2.state] ?? state_iso2.state.toString()}/>
                    </FormRow>
                    <FormRow label="Session ID">
                        <InputText value={array8_to_hexstring(state_iso2.session_id, '')}/>
                    </FormRow>
                    <FormRow label="EVCC ID">
                        <InputText value={array8_to_hexstring(state_iso2.evcc_id, '')}/>
                    </FormRow>
                    <FormRow label="Max Entries SA Schedule Tuple">
                        <InputNumber value={state_iso2.max_entries_sa_schedule_tuple}/>
                    </FormRow>
                    <FormRow label="Requested Energy Transfer Mode">
                        <InputText value={state_iso2.requested_energy_transfer_mode}/>
                    </FormRow>
                    <FormRow label="AC Is Used">
                        <InputText value={state_iso2.ac.is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="AC Departure Time">
                        <InputText value={state_iso2.ac.departure_time}/>
                    </FormRow>
                    <FormRow label="AC Departure Time Is Used">
                        <InputText value={state_iso2.ac.departure_time_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="AC EAmount">
                        <InputText value={state_iso2.ac.eamount_val + " * 10^" + state_iso2.ac.eamount_mul}/>
                    </FormRow>
                    <FormRow label="AC EV Max Voltage">
                        <InputText value={state_iso2.ac.ev_max_voltage_val + " * 10^" + state_iso2.ac.ev_max_voltage_mul}/>
                    </FormRow>
                    <FormRow label="AC EV Min Current">
                        <InputText value={state_iso2.ac.ev_min_current_val + " * 10^" + state_iso2.ac.ev_min_current_mul}/>
                    </FormRow>
                    <FormRow label="DC Is Used">
                        <InputText value={state_iso2.dc.is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="DC Departure Time">
                        <InputText value={state_iso2.dc.departure_time}/>
                    </FormRow>
                    <FormRow label="DC Departure Time Is Used">
                        <InputText value={state_iso2.dc.departure_time_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="DC SOC">
                        <InputText value={state_iso2.dc.soc}/>
                    </FormRow>
                    <FormRow label="DC EV Ready">
                        <InputText value={state_iso2.dc.ev_ready}/>
                    </FormRow>
                    <FormRow label="DC EV Error Code">
                        <InputText value={state_iso2.dc.ev_error_code}/>
                    </FormRow>
                    <FormRow label="DC EV Max Current Limit">
                        <InputText value={state_iso2.dc.ev_max_current_limit_val + " * 10^" + state_iso2.dc.ev_max_current_limit_mul}/>
                    </FormRow>
                    <FormRow label="DC EV Max Power Limit">
                        <InputText value={state_iso2.dc.ev_max_power_limit_val + " * 10^" + state_iso2.dc.ev_max_power_limit_mul}/>
                    </FormRow>
                    <FormRow label="DC EV Max Power Limit Is Used">
                        <InputText value={state_iso2.dc.ev_max_power_limit_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="DC EV Max Voltage Limit">
                        <InputText value={state_iso2.dc.ev_max_voltage_limit_val + " * 10^" + state_iso2.dc.ev_max_voltage_limit_mul}/>
                    </FormRow>
                    <FormRow label="DC EV Energy Capacity">
                        <InputText value={state_iso2.dc.ev_energy_capacity_val + " * 10^" + state_iso2.dc.ev_energy_capacity_mul}/>
                    </FormRow>
                    <FormRow label="DC EV Energy Capacity Is Used">
                        <InputText value={state_iso2.dc.ev_energy_capacity_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="DC EV Energy Request">
                        <InputText value={state_iso2.dc.ev_energy_request_val + " * 10^" + state_iso2.dc.ev_energy_request_mul}/>
                    </FormRow>
                    <FormRow label="DC EV Energy Request Is Used">
                        <InputText value={state_iso2.dc.ev_energy_request_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="DC Full SOC">
                        <InputText value={state_iso2.dc.full_soc}/>
                    </FormRow>
                    <FormRow label="DC Full SOC Is Used">
                        <InputText value={state_iso2.dc.full_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="DC Bulk SOC">
                        <InputText value={state_iso2.dc.bulk_soc}/>
                    </FormRow>
                    <FormRow label="DC Bulk SOC Is Used">
                        <InputText value={state_iso2.dc.bulk_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>

                    <FormSeparator heading="ISO 15118-20"/>
                    <FormRow label="State">
                        <InputText value={iso20_state_names[state_iso20.state] ?? state_iso20.state.toString()}/>
                    </FormRow>
                    <FormRow label="Session ID">
                        <InputText value={state_iso20.session_id || "-"}/>
                    </FormRow>
                    <FormRow label="EVCC ID">
                        <InputText value={state_iso20.evcc_id || "-"}/>
                    </FormRow>

                    <FormSeparator heading="ISO 15118-20 Display Parameters"/>
                    <FormRow label="Present SOC">
                        <InputText value={state_iso20.present_soc_is_used ? state_iso20.present_soc + "%" : "-"}/>
                    </FormRow>
                    <FormRow label="Present SOC Is Used">
                        <InputText value={state_iso20.present_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Minimum SOC">
                        <InputText value={state_iso20.minimum_soc_is_used ? state_iso20.minimum_soc + "%" : "-"}/>
                    </FormRow>
                    <FormRow label="Minimum SOC Is Used">
                        <InputText value={state_iso20.minimum_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Target SOC">
                        <InputText value={state_iso20.target_soc_is_used ? state_iso20.target_soc + "%" : "-"}/>
                    </FormRow>
                    <FormRow label="Target SOC Is Used">
                        <InputText value={state_iso20.target_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Maximum SOC">
                        <InputText value={state_iso20.maximum_soc_is_used ? state_iso20.maximum_soc + "%" : "-"}/>
                    </FormRow>
                    <FormRow label="Maximum SOC Is Used">
                        <InputText value={state_iso20.maximum_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Remaining Time To Min SOC">
                        <InputText value={state_iso20.remaining_time_to_minimum_soc_is_used ? state_iso20.remaining_time_to_minimum_soc + " s" : "-"}/>
                    </FormRow>
                    <FormRow label="Remaining Time To Min SOC Is Used">
                        <InputText value={state_iso20.remaining_time_to_minimum_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Remaining Time To Target SOC">
                        <InputText value={state_iso20.remaining_time_to_target_soc_is_used ? state_iso20.remaining_time_to_target_soc + " s" : "-"}/>
                    </FormRow>
                    <FormRow label="Remaining Time To Target SOC Is Used">
                        <InputText value={state_iso20.remaining_time_to_target_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Remaining Time To Max SOC">
                        <InputText value={state_iso20.remaining_time_to_maximum_soc_is_used ? state_iso20.remaining_time_to_maximum_soc + " s" : "-"}/>
                    </FormRow>
                    <FormRow label="Remaining Time To Max SOC Is Used">
                        <InputText value={state_iso20.remaining_time_to_maximum_soc_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Charging Complete">
                        <InputText value={state_iso20.charging_complete_is_used ? (state_iso20.charging_complete ? "Yes" : "No") : "-"}/>
                    </FormRow>
                    <FormRow label="Charging Complete Is Used">
                        <InputText value={state_iso20.charging_complete_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Battery Energy Capacity">
                        <InputText value={state_iso20.battery_energy_capacity_is_used ? state_iso20.battery_energy_capacity_val + " * 10^" + state_iso20.battery_energy_capacity_exp + " Wh" : "-"}/>
                    </FormRow>
                    <FormRow label="Battery Energy Capacity Is Used">
                        <InputText value={state_iso20.battery_energy_capacity_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="Inlet Hot">
                        <InputText value={state_iso20.inlet_hot_is_used ? (state_iso20.inlet_hot ? "Yes" : "No") : "-"}/>
                    </FormRow>
                    <FormRow label="Inlet Hot Is Used">
                        <InputText value={state_iso20.inlet_hot_is_used ? "Yes" : "No"}/>
                    </FormRow>

                    <FormSeparator heading="ISO 15118-20 Power"/>
                    <FormRow label="EV Present Active Power">
                        <InputText value={state_iso20.ev_present_active_power_val + " * 10^" + state_iso20.ev_present_active_power_exp + " W"}/>
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function pre_init() {
}

export function init() {
}
