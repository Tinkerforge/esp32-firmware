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
import { h, Fragment, Component, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import { InputText } from "../../ts/components/input_text";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Activity } from "react-feather";
import { InputNumber } from "../../ts/components/input_number";
import { FormSeparator } from "../../ts/components/form_separator";

export function ISO15118Navbar() {
    return <NavbarItem name="iso15118" module="iso15118" title="ISO15118" symbol={<Activity />} />;
}

type ISO15118Config = API.getType["iso15118/config"];

export class ISO15118 extends ConfigComponent<'iso15118/config', {}> {
    constructor() {
        super('iso15118/config',
              () => __("iso15118.script.save_failed"),
              () => __("iso15118.script.reboot_content_changed"));
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
                <ConfigForm id="iso15118_config_form" title="ISO15118" isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
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
                        <InputText value={state_din70121.ev_maximum_current_limit_val + " * 10^" + state_din70121.ev_maximum_current_limit_mul}/>
                    </FormRow>
                    <FormRow label="EV Maximum Power Limit">
                        <InputText value={state_din70121.ev_maximum_power_limit_val + " * 10^" + state_din70121.ev_maximum_power_limit_mul}/>
                    </FormRow>
                    <FormRow label="EV Maximum Power Limit Is Used">
                        <InputText value={state_din70121.ev_maximum_power_limit_is_used ? "Yes" : "No"}/>
                    </FormRow>
                    <FormRow label="EV Maximum Voltage Limit">
                        <InputText value={state_din70121.ev_maximum_voltage_limit_val + " * 10^" + state_din70121.ev_maximum_voltage_limit_mul}/>
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

                    <FormSeparator heading="ISO 15118-20"/>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
