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
import { parseIP, unparseIP } from "../util";

import Collapse from "react-bootstrap/Collapse";
import { InputSelect } from "./input_select";
import { InputSubnet } from "./input_subnet";

export interface IPConfig {
    ip: string,
    gateway: string,
    subnet: string,
    dns?: string,
    dns2?: string,
}

interface IPConfigurationProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput" | "value" | "disabled"> {
    showAnyAddress: boolean,
    showDhcp?: boolean,
    showDns?: boolean,
    value: IPConfig
    onValue: (value: IPConfig) => void
    setValid: (valid: boolean) => void

    ip_label?: string
    gateway_label?: string
    subnet_label?: string

    forbidNetwork?: {ip: number, subnet: number, name: string}[]
}

export class IPConfiguration extends Component<IPConfigurationProps, {}> {
    constructor() {
        super();
    }

    onUpdate<T extends keyof IPConfig>(k: T, v: IPConfig[T]) {
        this.props.value[k] = v;
        this.props.onValue(this.props.value);
    }

    render(props: IPConfigurationProps, state: Readonly<{}>) {
        let dhcp = props.value.ip == "0.0.0.0";
        let gateway_out_of_subnet = false;
        let ip_is_network_addr = false;
        let ip_is_broadcast_addr = false;
        let captured_subnet_name = "";
        let captured_subnet_ip = "";
        if (!dhcp && props.value.ip !== undefined) { //ip is undefined if we render before the web socket connection is established.
            let ip = parseIP(props.value.ip);
            let subnet = parseIP(props.value.subnet);
            let gateway = parseIP(props.value.gateway);

            if (!isNaN(ip) && !isNaN(subnet) && !isNaN(gateway) && subnet != 0){
                gateway_out_of_subnet = gateway != 0 && (ip & subnet) != (gateway & subnet);
                if (((subnet + 1) >>> 0) != 0xFFFFFFFF >>> 0)
                {
                    const network = (ip & subnet) >>> 0
                    const broadcast = ((ip & subnet) | ~subnet) >>> 0;
                    ip_is_network_addr = ip == network;
                    ip_is_broadcast_addr = ip == broadcast;
                }

                if (props.forbidNetwork) {
                    for (let net of props.forbidNetwork) {
                        let common_subnet = subnet & net.subnet;
                        if ((ip & common_subnet) == (net.ip & common_subnet))
                            captured_subnet_name = net.name;
                            captured_subnet_ip = unparseIP(net.ip);
                    }
                }
            }
        }


        this.props.setValid(!(gateway_out_of_subnet || captured_subnet_name != "" || ip_is_network_addr || ip_is_broadcast_addr))

        let ip_invalid_feedback = "";
        if (ip_is_broadcast_addr) {
            ip_invalid_feedback = __("component.ip_configuration.ip_is_broadcast");
        } else if (ip_is_network_addr) {
            ip_invalid_feedback = __("component.ip_configuration.ip_is_network");
        } else {
            ip_invalid_feedback = __("component.ip_configuration.static_ip_invalid");
        }

        let inner = (<>
            <FormRow label={props.ip_label ? props.ip_label : __("component.ip_configuration.static_ip")}>
                <InputIP invalidFeedback={ip_invalid_feedback}
                         moreClasses={ip_is_network_addr || ip_is_broadcast_addr ? ["is-invalid"] : [""]}
                         required={!props.showDhcp || !dhcp}
                         value={!props.showAnyAddress && props.value.ip == "0.0.0.0" ? "" : props.value.ip}
                         onValue={(v) => this.onUpdate("ip", !props.showAnyAddress && v == "" ? "0.0.0.0" : v)}/>
            </FormRow>
            <FormRow label={props.gateway_label ? props.gateway_label : __("component.ip_configuration.gateway")}>
                <InputIP invalidFeedback={gateway_out_of_subnet ? __("component.ip_configuration.gateway_out_of_subnet") : __("component.ip_configuration.gateway_invalid")}
                         moreClasses={gateway_out_of_subnet ? ["is-invalid"] : [""]}
                         required={!props.showDhcp || !dhcp}
                         value={!props.showAnyAddress && props.value.gateway == "0.0.0.0" ? "" : props.value.gateway}
                         onValue={(v) => this.onUpdate("gateway", !props.showAnyAddress && v == "" ? "0.0.0.0" : v)}/>
            </FormRow>
            <FormRow label={props.subnet_label ? props.subnet_label : __("component.ip_configuration.subnet")}>
                <InputSubnet classList={captured_subnet_name != "" ? "is-invalid" : ""}
                        required={!props.showDhcp || !dhcp}
                        value={props.value.subnet}
                        onValue={(v) => this.onUpdate("subnet", v)}
                        placeholder={__("component.ip_configuration.subnet_placeholder")}
                    />
                {captured_subnet_name != "" ? <div class="invalid-feedback">{__("component.ip_configuration.subnet_captures_prefix") + captured_subnet_name + " (" + captured_subnet_ip + ") " + __("component.ip_configuration.subnet_captures_suffix")}</div> : <></>}
            </FormRow>
        </>);

        if (props.showDns) {
            inner = (<>
                {inner}
                <FormRow label={__("component.ip_configuration.dns")} label_muted={__("component.ip_configuration.dns_muted")}>
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
