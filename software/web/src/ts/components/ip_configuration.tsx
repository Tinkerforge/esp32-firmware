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

import { h, Component, Fragment } from "preact";
import { JSXInternal } from "preact/src/jsx";
import { __ } from "../translation";
import { FormRow } from "./form_row";
import { InputIP } from "./input_ip";

import Collapse from 'react-bootstrap/Collapse';
import { InputSelect } from "./input_select";

export interface IPConfig {
    ip: string,
    gateway: string,
    subnet: string,
    dns?: string,
    dns2?: string,
}

interface IPConfigurationProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput" | "value" | "disabled"> {
    showDhcp?: boolean,
    showDns?: boolean,
    value: IPConfig
    onValue: (value: IPConfig) => void
    setValid: (valid: boolean) => void

    ip_label?: string
    gateway_label?: string
    subnet_label?: string
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
            }
        }

        this.props.setValid(!(gateway_out_of_subnet || subnet_captures_localhost))

        let inner = (<>
            <FormRow label={props.ip_label ? props.ip_label : __("component.ip_configuration.static_ip")}>
                <InputIP invalidFeedback={__("component.ip_configuration.static_ip_invalid")}
                         required={!props.showDhcp || !dhcp}
                         value={props.value.ip}
                         onValue={(v) => this.onUpdate("ip", v)}/>
            </FormRow>
            <FormRow label={props.gateway_label ? props.gateway_label : __("component.ip_configuration.gateway")}>
                <InputIP invalidFeedback={gateway_out_of_subnet ? __("component.ip_configuration.gateway_out_of_subnet") : __("component.ip_configuration.gateway_invalid")}
                         moreClasses={gateway_out_of_subnet ? ["is-invalid"] : [""]}
                         required={!props.showDhcp || !dhcp}
                         value={props.value.gateway}
                         onValue={(v) => this.onUpdate("gateway", v)}/>
            </FormRow>
            <FormRow label={props.subnet_label ? props.subnet_label : __("component.ip_configuration.subnet")}>
                <InputSelect classList={subnet_captures_localhost ? "is-invalid" : ""}
                        required={!props.showDhcp || !dhcp}
                        value={props.value.subnet}
                        onValue={(v) => this.onUpdate("subnet", v)}
                        placeholder={__("component.ip_configuration.subnet_placeholder")}
                        items={[
                            ["255.255.255.255", "255.255.255.255 (/32)"],
                            ["255.255.255.254", "255.255.255.254 (/31)"],
                            ["255.255.255.252", "255.255.255.252 (/30)"],
                            ["255.255.255.248", "255.255.255.248 (/29)"],
                            ["255.255.255.240", "255.255.255.240 (/28)"],
                            ["255.255.255.224", "255.255.255.224 (/27)"],
                            ["255.255.255.192", "255.255.255.192 (/26)"],
                            ["255.255.255.128", "255.255.255.128 (/25)"],

                            ["255.255.255.0", "255.255.255.0 (/24)"],
                            ["255.255.254.0", "255.255.254.0 (/23)"],
                            ["255.255.252.0", "255.255.252.0 (/22)"],
                            ["255.255.248.0", "255.255.248.0 (/21)"],
                            ["255.255.240.0", "255.255.240.0 (/20)"],
                            ["255.255.224.0", "255.255.224.0 (/19)"],
                            ["255.255.192.0", "255.255.192.0 (/18)"],
                            ["255.255.128.0", "255.255.128.0 (/17)"],

                            ["255.255.0.0", "255.255.0.0 (/16)"],
                            ["255.254.0.0", "255.254.0.0 (/15)"],
                            ["255.252.0.0", "255.252.0.0 (/14)"],
                            ["255.248.0.0", "255.248.0.0 (/13)"],
                            ["255.240.0.0", "255.240.0.0 (/12)"],
                            ["255.224.0.0", "255.224.0.0 (/11)"],
                            ["255.192.0.0", "255.192.0.0 (/10)"],
                            ["255.128.0.0", "255.128.0.0 (/9)"],

                            ["255.0.0.0", "255.0.0.0 (/8)"],
                            ["254.0.0.0", "254.0.0.0 (/7)"],
                            ["252.0.0.0", "252.0.0.0 (/6)"],
                            ["248.0.0.0", "248.0.0.0 (/5)"],
                            ["240.0.0.0", "240.0.0.0 (/4)"],
                            ["224.0.0.0", "224.0.0.0 (/3)"],
                            ["192.0.0.0", "192.0.0.0 (/2)"],
                            ["128.0.0.0", "128.0.0.0 (/1)"]
                        ]}
                    />
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
                    <InputSelect
                        value={dhcp ? "hide" : "show"}
                        onValue={(v) => {
                                if (v == "hide")
                                    props.onValue({
                                        ip: "0.0.0.0",
                                        gateway: "0.0.0.0",
                                        subnet: "0.0.0.0",
                                        dns: "",
                                        dns2: ""
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
                            }}
                        items={[
                            ["hide", __("component.ip_configuration.dhcp")],
                            ["show", __("component.ip_configuration.static")]
                        ]}
                        />
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
