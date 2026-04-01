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

import {h, Component, Fragment} from "preact";
import {JSX} from 'preact';
import {__} from "../translation";
import {FormRow} from "./form_row";
import {InputIP} from "./input_ip";
import {parseIP, range, unparseIP, IPV6_ADDRESS_PATTERN, parseCIDR, hasCIDR} from "../util";
import * as API from "../api";

import Collapse from "react-bootstrap/Collapse";
import {InputSelect} from "./input_select";
import {InputSubnet} from "./input_subnet";

export interface IPConfig {
    ip: string;
    gateway?: string;
    subnet?: string;
    dns?: string;
    dns2?: string;
}

interface IPConfigurationProps extends Omit<JSX.InputHTMLAttributes<HTMLInputElement>, "class" | "id" | "type" | "onInput" | "value" | "disabled"> {
    showAnyAddress: boolean,
    showDhcp?: boolean,
    showDns?: boolean,
    hideSubnet?: boolean
    hideGateway?: boolean
    hideDns2?: boolean
    value: IPConfig
    onValue: (value: IPConfig) => void
    setValid: (valid: boolean) => void

    ipv6?: boolean
    allowIpVersionSwitch?: boolean
    onIpVersionChange?: (ipv6: boolean) => void

    ip_label?: string
    gateway_label?: string
    subnet_label?: string

    min_subnet_prefix?: number
    max_subnet_prefix?: number

    forbidNetwork?: { ip: number, subnet: number, name: string }[]
}

interface IPConfigurationState {
    ipv6Mode: boolean;
    cidrDetected: boolean;
}

export class IPConfiguration extends Component<IPConfigurationProps, IPConfigurationState> {

    ip_value = "";

    constructor(props: IPConfigurationProps) {
        super(props);
        this.state = {
            ipv6Mode: !!props.ipv6,
            cidrDetected: hasCIDR(props.value.ip || "")
        };
    }

    onUpdate<T extends keyof IPConfig>(k: T, v: IPConfig[T]) {
        this.props.value[k] = v;
        this.props.onValue(this.props.value);
    }

    onIpUpdate(v: string) {
        // Check if CIDR notation is present
        const cidrResult = parseCIDR(v);
        if (cidrResult) {
            // CIDR detected - extract IP and subnet
            this.setState({cidrDetected: true});
            this.props.value.ip = cidrResult.ip;
            this.props.value.subnet = cidrResult.prefix;
            this.props.onValue(this.props.value);
        } else {
            // No CIDR - just update IP
            this.setState({cidrDetected: false});
            this.props.value.ip = v;
            this.props.onValue(this.props.value);
        }
        this.ip_value = v;
    }

    addRemoteAccessNetworks(networks: { ip: number, subnet: number, name: string }[]) {
        const remoteAccessNetworks = range(0, API.get_unchecked("remote_access/config").users.length * 5).map((i) => {
            return {
                ip: parseIP(`10.123.${i}.2`),
                subnet: parseIP("255.255.255.0"),
                name: __("component.ip_configuration.remote_access")
            };
        });
        remoteAccessNetworks.push({
            ip: parseIP("10.123.123.2"),
            subnet: parseIP("255.255.255.0"),
            name: __("component.ip_configuration.remote_access"),
        });
        return networks.concat(remoteAccessNetworks);
    }

    render(props: IPConfigurationProps, state: Readonly<{}>) {
        const ipv6Mode = this.state.ipv6Mode;
        const cidrDetected = this.state.cidrDetected;

        let dhcp = props.value.ip == "0.0.0.0";
        if (ipv6Mode) {
            dhcp = props.value.ip == "::";
        }
        if (!dhcp && props.value.ip !== undefined && props.value.ip !== "") {
            this.ip_value = props.value.ip;
        } else if (dhcp && this.ip_value !== "") {
            this.ip_value = "";
        }
        let gateway_out_of_subnet = false;
        let ip_is_network_addr = false;
        let ip_is_broadcast_addr = false;
        let captured_subnet_name = "";
        let captured_subnet_ip = "";
        let ipv6_ip_invalid = false;
        let ipv6_gateway_invalid = false;
        if (!dhcp && props.value.ip !== undefined && !ipv6Mode) { //IPv4 validation only
            let ip = parseIP(props.value.ip);
            let subnet = parseIP(props.value.subnet);
            let gateway = parseIP(props.value.gateway);

            if (!isNaN(ip) && !isNaN(subnet) && !isNaN(gateway) && subnet != 0) {
                gateway_out_of_subnet = gateway != 0 && (ip & subnet) != (gateway & subnet);
                if (((subnet + 1) >>> 0) != 0xFFFFFFFF >>> 0) {
                    const network = (ip & subnet) >>> 0
                    const broadcast = ((ip & subnet) | ~subnet) >>> 0;
                    ip_is_network_addr = ip == network;
                    ip_is_broadcast_addr = ip == broadcast;
                }

                if (props.forbidNetwork) {
                    let forbidNetwork = props.forbidNetwork;
                    if (API.hasModule("remote_access") && API.get_unchecked("remote_access/config").enable) {
                        forbidNetwork = this.addRemoteAccessNetworks(forbidNetwork);
                    }

                    for (let net of forbidNetwork) {
                        let common_subnet = subnet & net.subnet;
                        if ((ip & common_subnet) == (net.ip & common_subnet)) {
                            captured_subnet_name = net.name;
                            captured_subnet_ip = unparseIP(net.ip);
                        }
                    }
                }
            }
        }
        if (!dhcp && props.value.ip !== undefined && ipv6Mode) {
            const ipv6Regex = new RegExp(`^${IPV6_ADDRESS_PATTERN}$`);
            ipv6_ip_invalid = !ipv6Regex.test(props.value.ip);
            if (props.value.gateway !== undefined && props.value.gateway.length > 0) {
                ipv6_gateway_invalid = !ipv6Regex.test(props.value.gateway);
            }
        }


        this.props.setValid(!(gateway_out_of_subnet || captured_subnet_name != "" || ip_is_network_addr || ip_is_broadcast_addr || ipv6_ip_invalid || ipv6_gateway_invalid))

        let ip_invalid_feedback = "";
        if (ipv6Mode && ipv6_ip_invalid) {
            ip_invalid_feedback = __("component.ip_configuration.static_ipv6_invalid");
        } else if (ip_is_broadcast_addr) {
            ip_invalid_feedback = __("component.ip_configuration.ip_is_broadcast");
        } else if (ip_is_network_addr) {
            ip_invalid_feedback = __("component.ip_configuration.ip_is_network");
        } else {
            ip_invalid_feedback = __("component.ip_configuration.static_ip_invalid");
        }


        let inner = (<>
            <FormRow label={props.ip_label ? props.ip_label : __("component.ip_configuration.static_ip")}>
                <InputIP invalidFeedback={ip_invalid_feedback}
                         moreClasses={ip_is_network_addr || ip_is_broadcast_addr || ipv6_ip_invalid ? ["is-invalid"] : [""]}
                         required={!props.showDhcp || !dhcp}
                         value={this.ip_value}
                         onValue={(v) => {
                             this.onIpUpdate(!props.showAnyAddress && v == "" ? (ipv6Mode ? "::" : "0.0.0.0") : v);
                         }}
                         ipVersion={ipv6Mode ? "v6" : "v4"}
                         allowCidr={true}/>
            </FormRow>

        </>);
        if(!props.hideGateway) {
            inner = (<>
                {inner}
                <FormRow label={props.gateway_label ? props.gateway_label : __("component.ip_configuration.gateway")}>
                    <InputIP
                        invalidFeedback={gateway_out_of_subnet ? __("component.ip_configuration.gateway_out_of_subnet") : __("component.ip_configuration.gateway_invalid")}
                        moreClasses={(gateway_out_of_subnet || ipv6_gateway_invalid) ? ["is-invalid"] : [""]}
                        required={!props.showDhcp || !dhcp}
                        value={!props.showAnyAddress && (props.value.gateway == "0.0.0.0" || props.value.gateway == "::") ? "" : props.value.gateway}
                        onValue={(v) => this.onUpdate("gateway", !props.showAnyAddress && v == "" ? (ipv6Mode ? "::" : "0.0.0.0") : v)}
                        ipVersion={ipv6Mode ? "v6" : "v4"}/>
                </FormRow>
                </>)
        }
        if(!props.hideSubnet) {
            inner = (<>
                {inner}
                <FormRow label={props.subnet_label ? props.subnet_label : __("component.ip_configuration.subnet")}>
                    <InputSubnet className={captured_subnet_name != "" ? "is-invalid" : ""}
                            required={!props.showDhcp || !dhcp}
                            value={props.value.subnet}
                            onValue={(v) => this.onUpdate("subnet", v)}
                            minPrefixLength={props.min_subnet_prefix}
                            maxPrefixLength={props.max_subnet_prefix}
                            placeholder={__("component.ip_configuration.subnet_placeholder")}
                            ipVersion={ipv6Mode ? "v6" : "v4"}
                            disabled={cidrDetected}
                        />
                    {captured_subnet_name != "" ? <div class="invalid-feedback">{__("component.ip_configuration.subnet_captures_prefix") + captured_subnet_name + " (" + captured_subnet_ip + ") " + __("component.ip_configuration.subnet_captures_suffix")}</div> : <></>}
                </FormRow>
            </>)
        }

        if (props.showDns) {
            inner = (<>
                {inner}
                <FormRow label={__("component.ip_configuration.dns")}>
                    <InputIP invalidFeedback={__("component.ip_configuration.dns_invalid")}
                             value={props.value.dns}
                             onValue={(v) => this.onUpdate("dns", v)}
                             ipVersion={ipv6Mode ? "v6" : "v4"}/>
                </FormRow>
            </>)
                if (!props.hideDns2) {
                    inner = (<>
                        {inner}
                        <FormRow label={__("component.ip_configuration.dns2")}
                                 label_muted={__("component.ip_configuration.dns2_muted")}>
                            <InputIP invalidFeedback={__("component.ip_configuration.dns2_invalid")}
                                     value={props.value.dns2}
                                     onValue={(v) => this.onUpdate("dns2", v)}
                                     ipVersion={ipv6Mode ? "v6" : "v4"}/>
                        </FormRow>
                    </>)
                }
        }

        if (!props.showDhcp)
            return inner;

        const allowSwitch = props.allowIpVersionSwitch === true;

        return (
            <>
                <FormRow label={__("component.ip_configuration.ip_configuration")}>
                    <div class="d-flex gap-2 align-items-center">
                        <InputSelect
                            value={dhcp ? "hide" : "show"}
                            onValue={(v) => {
                                if (v == "hide")
                                    props.onValue({
                                        ip: ipv6Mode ? "::" : "0.0.0.0",
                                        gateway: ipv6Mode ? "::" : "0.0.0.0",
                                        subnet: ipv6Mode ? "::" : "0.0.0.0",
                                        dns: ipv6Mode ? "::" : "0.0.0.0",
                                        dns2: ipv6Mode ? "::" : "0.0.0.0"
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
                                ["hide", ipv6Mode ?  __("component.ip_configuration.dhcpv6"): __("component.ip_configuration.dhcp")],
                                ["show", __("component.ip_configuration.static")]
                            ]}
                        />
                        {allowSwitch ? <InputSelect
                            value={ipv6Mode ? "v6" : "v4"}
                            onValue={(v) => {
                                const newIpv6 = v === "v6";
                                if (props.onIpVersionChange) {
                                    props.onIpVersionChange(newIpv6);
                                }
                                this.setState({ipv6Mode: newIpv6});
                            }}
                            items={[
                                ["v4", "IPv4"],
                                ["v6", "IPv6"]
                            ]}
                        /> : <></>}
                    </div>
                </FormRow>

                <Collapse in={!dhcp}>
                    <div>{inner}</div>
                </Collapse>
            </>
        );
    }
}
