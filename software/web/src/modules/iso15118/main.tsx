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
import { InputSelect } from "../../ts/components/input_select";
import { InputText } from "../../ts/components/input_text";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Activity } from "react-feather";
import { InputNumber } from "../../ts/components/input_number";
import { FormSeparator } from "../../ts/components/form_separator";
import { UplotData, UplotWrapperB, UplotPath } from "../../ts/components/uplot_wrapper_2nd";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { ChargeType } from "./chargetype.enum";

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
                    <FormSeparator heading="Configuration"/>
                    <FormRow label="ISO 15118 enabled">
                        <Switch desc="ISO 15118 allows to read the State of Charge and identify an EV for Plug and Charge"
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <FormRow label="Charge Type" label_muted="This is for testing different possible modes, will probably be changed/removed in release version">
                        <InputSelect
                            items={[
                                ["0", "DC Read SoC Once"],
                                ["1", "DC Read SoC In Loop"],
                                ["2", "AC Charging"]
                            ]}
                            value={state.charge_type}
                            onValue={(v) => this.setState({charge_type: parseInt(v)})}
                        />
                    </FormRow>
                    <FormSeparator heading="Signal Level Attenuation Characterisation (SLAC)"/>
                    <FormRow label="State">
                        <InputNumber value={state_slac.state}/>
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
                    <FormRow label="">
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
                        <InputNumber value={state_sdp.state}/>
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
                        <InputNumber value={state_common.state}/>
                    </FormRow>
                    <FormRow label="Supported Protocols">
                        <InputText value={state_common.supported_protocols.join(' and ')}/>
                    </FormRow>
                    <FormRow label="Protocol">
                        <InputText value={state_common.protocol}/>
                    </FormRow>

                    <FormSeparator heading="DIN SPEC 70121"/>
                    <FormRow label="State">
                        <InputNumber value={state_din70121.state}/>
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
                        <InputNumber value={state_iso2.state}/>
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
                        <InputNumber value={state_iso20.state}/>
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
