/* esp32-firmware
 * Copyright (C) 2022 Erik Fleckstein <erik@tinkerforge.com>
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

import { h, Component, Context, Fragment, createRef, RefObject } from "preact";
import {useContext} from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";
import { __ } from "../translation";
import { FormRow } from "./form_row";
import { InputIP } from "./input_ip";

import Collapse from 'react-bootstrap/Collapse';

export interface IPConfig {
    ip: string,
    gateway: string,
    subnet: string,
    dns?: string,
    dns2?: string,
}

interface IPConfigurationProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput" | "value" | "disabled"> {
    showDhcp: boolean,
    showDns: boolean,
    value: IPConfig
    onValue: (value: IPConfig) => void
    setValid: (valid: boolean) => void
}


export class IPConfiguration extends Component<IPConfigurationProps, {}> {
    constructor() {
        super();
    }

    onUpdate<T extends keyof IPConfig>(k: T, v: IPConfig[T]) {
        this.props.value[k] = v;
        this.props.onValue(this.props.value);
    }

    parseIP(ip: string) {
        return ip.split(".").map((x, i, _) => parseInt(x, 10) * (1 << (8 * (3 - i)))).reduce((a, b) => a+b);
    }

    render(props: IPConfigurationProps, state: Readonly<{}>) {
        let dhcp = props.value.ip == "0.0.0.0";
        let gateway_out_of_subnet = false;
        let subnet_captures_localhost = false;
        if (!dhcp && props.value.ip !== undefined) { //ip is undefined if we render before the web socket connection is established.
            let ip = this.parseIP(props.value.ip);
            let subnet = this.parseIP(props.value.subnet);
            let gateway = this.parseIP(props.value.gateway);

            if (!isNaN(ip) && !isNaN(subnet) && !isNaN(gateway)){
                gateway_out_of_subnet = gateway != 0 && (ip & subnet) != (gateway & subnet);
                subnet_captures_localhost = (ip & subnet) == (0x7F000001 & subnet);
                console.log(ip, subnet, gateway, gateway_out_of_subnet, subnet_captures_localhost);
            }
        }

        this.props.setValid(!(gateway_out_of_subnet || subnet_captures_localhost))

        let inner = (<>
            <FormRow label={__("component.ip_configuration.static_ip")}>
                <InputIP invalidFeedback={__("component.ip_configuration.static_ip_invalid")}
                         required={!props.showDhcp || !dhcp}
                         value={props.value.ip}
                         onValue={(v) => this.onUpdate("ip", v)}/>
            </FormRow>
            <FormRow label={__("component.ip_configuration.gateway")}>
                <InputIP invalidFeedback={gateway_out_of_subnet ? __("component.ip_configuration.gateway_out_of_subnet") : __("component.ip_configuration.gateway_invalid")}
                         moreClasses={gateway_out_of_subnet ? ["is-invalid"] : [""]}
                         required={!props.showDhcp || !dhcp}
                         value={props.value.gateway}
                         onValue={(v) => this.onUpdate("gateway", v)}/>
            </FormRow>
            <FormRow label={__("component.ip_configuration.subnet")}>
                <select class={"custom-select" + (subnet_captures_localhost ? " is-invalid" : "")}
                        required={!props.showDhcp || !dhcp}
                        value={props.value.subnet}
                        onChange={(e) => this.onUpdate("subnet", (e.target as HTMLSelectElement).value)}>
                    <option value="" disabled selected>{__("component.ip_configuration.subnet_placeholder")}</option>
                    <option value="255.255.255.255">255.255.255.255 (/32)</option>
                    <option value="255.255.255.254">255.255.255.254 (/31)</option>
                    <option value="255.255.255.252">255.255.255.252 (/30)</option>
                    <option value="255.255.255.248">255.255.255.248 (/29)</option>
                    <option value="255.255.255.240">255.255.255.240 (/28)</option>
                    <option value="255.255.255.224">255.255.255.224 (/27)</option>
                    <option value="255.255.255.192">255.255.255.192 (/26)</option>
                    <option value="255.255.255.128">255.255.255.128 (/25)</option>

                    <option value="255.255.255.0">255.255.255.0 (/24)</option>
                    <option value="255.255.254.0">255.255.254.0 (/23)</option>
                    <option value="255.255.252.0">255.255.252.0 (/22)</option>
                    <option value="255.255.248.0">255.255.248.0 (/21)</option>
                    <option value="255.255.240.0">255.255.240.0 (/20)</option>
                    <option value="255.255.224.0">255.255.224.0 (/19)</option>
                    <option value="255.255.192.0">255.255.192.0 (/18)</option>
                    <option value="255.255.128.0">255.255.128.0 (/17)</option>

                    <option value="255.255.0.0">255.255.0.0 (/16)</option>
                    <option value="255.254.0.0">255.254.0.0 (/15)</option>
                    <option value="255.252.0.0">255.252.0.0 (/14)</option>
                    <option value="255.248.0.0">255.248.0.0 (/13)</option>
                    <option value="255.240.0.0">255.240.0.0 (/12)</option>
                    <option value="255.224.0.0">255.224.0.0 (/11)</option>
                    <option value="255.192.0.0">255.192.0.0 (/10)</option>
                    <option value="255.128.0.0">255.128.0.0 (/9)</option>

                    <option value="255.0.0.0">255.0.0.0 (/8)</option>
                    <option value="254.0.0.0">254.0.0.0 (/7)</option>
                    <option value="252.0.0.0">252.0.0.0 (/6)</option>
                    <option value="248.0.0.0">248.0.0.0 (/5)</option>
                    <option value="240.0.0.0">240.0.0.0 (/4)</option>
                    <option value="224.0.0.0">224.0.0.0 (/3)</option>
                    <option value="192.0.0.0">192.0.0.0 (/2)</option>
                    <option value="128.0.0.0">128.0.0.0 (/1)</option>
                </select>
                <div class="invalid-feedback" dangerouslySetInnerHTML={{__html:__("component.ip_configuration.subnet_captures_localhost")}}></div>
            </FormRow>

        </>);

        if (props.showDns) {
            inner = (<>
                {inner}
                <FormRow label={__("component.ip_configuration.dns")}>
                    <InputIP invalidFeedback={__("component.ip_configuration.dns_invalid")}
                             value={props.value.dns}
                             onValue={(v) => this.onUpdate("dns", v)}/>
                </FormRow>
                <FormRow label={__("component.ip_configuration.dns2")} label_muted={__("component.ip_configuration.dns2_muted")}>
                    <InputIP invalidFeedback={__("component.ip_configuration.dns2_invalid")}
                             value={props.value.dns2}
                             onValue={(v) => this.onUpdate("dns2", v)}/>
                </FormRow>
            </>)
        }

        if (!props.showDhcp)
            return inner;

        return (
            <>
                <FormRow label={__("component.ip_configuration.ip_configuration")}>
                    <select class="custom-select" value={dhcp ? "hide" : "show"} onChange={(e) => {
                        if ((e.target as HTMLSelectElement).value == "hide")
                            props.onValue({
                                ip: "0.0.0.0",
                                gateway: "0.0.0.0",
                                subnet: "0.0.0.0",
                                dns: null,
                                dns2: null
                            });
                        else {
                            props.onValue({
                                ip: "",
                                gateway: "",
                                subnet: "",
                                dns: "",
                                dns2: ""
                            });
                        }
                    }}>
                        <option value="hide" selected data-i18n="component.ip_configuration.dhcp"></option>
                        <option value="show" data-i18n="component.ip_configuration.static"></option>
                    </select>
                </FormRow>

                <Collapse in={!dhcp}>
                    <div>
                        {inner}
                    </div>
                </Collapse>
            </>
        );
    }
}
