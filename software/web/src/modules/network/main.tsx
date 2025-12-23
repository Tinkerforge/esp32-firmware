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

//#include "module_available.inc"
//#include "../../options.inc"

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { h } from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputTextPatterned } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Collapse } from "react-bootstrap";
import { Settings } from "react-feather";
import { useEffect } from "preact/hooks";
import { TransportMode } from "../web_server/transport_mode.enum";

export function NetworkNavbar() {
    return <NavbarItem name="network" module="network" title={__("network.navbar.network")} symbol={<Settings />} />;
}

type NetworkConfig = API.getType["network/config"];
interface NetworkState {
//#if MODULE_ETHERNET_AVAILABLE
    ethernet: API.getType["ethernet/state"];
    ethernetConfig: API.getType["ethernet/config"];
//#endif

//#if MODULE_WIFI_AVAILABLE
    wifi: API.getType["wifi/state"];
    wifiConfig: API.getType["wifi/sta_config"];
    apConfig: API.getType["wifi/ap_config"];
//#endif

//#if MODULE_WIREGUARD_AVAILABLE
    wireguardConfig: API.getType["wireguard/config"];
//#endif

//#if MODULE_REMOTE_ACCESS_AVAILABLE
    remoteAccessConfig: API.getType["remote_access/config"];
//#endif
}

export class Network extends ConfigComponent<'network/config', {}, NetworkState> {
    constructor() {
        super('network/config',
              () => __("network.script.save_failed"),
              () => __("network.script.reboot_content_changed"));

//#if MODULE_ETHERNET_AVAILABLE
        util.addApiEventListener('ethernet/state', () => {
            this.setState({ ethernet: API.get('ethernet/state') });
        });
        util.addApiEventListener('ethernet/config', () => {
            this.setState({ ethernetConfig: API.get('ethernet/config') });
        });
//#endif

//#if MODULE_WIFI_AVAILABLE
        util.addApiEventListener('wifi/state', () => {
            this.setState({ wifi: API.get('wifi/state') });
        });
        util.addApiEventListener('wifi/sta_config', () => {
            this.setState({ wifiConfig: API.get('wifi/sta_config') });
        });
        util.addApiEventListener('wifi/ap_config', () => {
            this.setState({ apConfig: API.get('wifi/ap_config') });
        });
//#endif

//#if MODULE_WIREGUARD_AVAILABLE
        util.addApiEventListener('wireguard/config', () => {
            this.setState({ wireguardConfig: API.get('wireguard/config') });
        });
//#endif


//#if MODULE_REMOTE_ACCESS_AVAILABLE
        util.addApiEventListener('remote_access/config', () => {
            this.setState({ remoteAccessConfig: API.get('remote_access/config') });
        });
//#endif
    }

    getEqualSubnets() {
        const connectedSubnets: {
            network: number;
            subnet: string;
            name: string;
            href: string
            dhcp?: boolean;
        }[] = [];

//#if MODULE_ETHERNET_AVAILABLE
        const ethernetIP = util.parseIP(this.state.ethernet.ip);
        if (ethernetIP !== 0) {
            const ethernetSubnet = util.parseIP(this.state.ethernet.subnet);
            const ethernetNetwork = ethernetIP & ethernetSubnet;
            const subnetString = `/${util.countBits(ethernetSubnet)}`;
            const dhcp = API.get_unchecked("ethernet/config").ip === "0.0.0.0";
            connectedSubnets.push({network: ethernetNetwork, name: translate_unchecked("ethernet.navbar.ethernet"), subnet: subnetString, href: "#ethernet", dhcp});
        }
//#endif

//#if MODULE_WIFI_AVAILABLE
        const wifiIP = util.parseIP(this.state.wifi.sta_ip);
        if (wifiIP !== 0) {
            const wifiSubnet = util.parseIP(this.state.wifi.sta_subnet);
            const wifiNetwork = wifiIP & wifiSubnet;
            const subnetString = `/${util.countBits(wifiSubnet)}`;
            const dhcp = API.get("wifi/sta_config").ip === "0.0.0.0";
            connectedSubnets.push({network: wifiNetwork, name: __("wifi.navbar.wifi_sta"), subnet: subnetString, href: "#wifi_sta", dhcp});
        }

        if (this.state.apConfig.enable_ap && !this.state.apConfig.ap_fallback_only) {
            const apIP = util.parseIP(this.state.apConfig.ip);
            const apSubnet = util.parseIP(this.state.apConfig.subnet);
            const apNetwork = apIP & apSubnet;
            const subnetString = `/${util.countBits(apSubnet)}`;
            connectedSubnets.push({network: apNetwork, name: __("wifi.navbar.wifi_ap"), subnet: subnetString, href: "#wifi_ap"});
        }
//#endif

//#if MODULE_WIREGUARD_AVAILABLE
        if (this.state.wireguardConfig.enable) {
            const wireguardIP = util.parseIP(this.state.wireguardConfig.internal_ip);
            const wireguardSubnet = util.parseIP(this.state.wireguardConfig.internal_subnet);
            const wireguardNetwork = wireguardIP & wireguardSubnet;
            const subnetString = `/${util.countBits(wireguardSubnet)}`;
            connectedSubnets.push({network: wireguardNetwork, name: __("wireguard.navbar.wireguard"), subnet: subnetString, href: "#wireguard"});
        }
//#endif

//#if MODULE_REMOTE_ACCESS_AVAILABLE
        if (this.state.remoteAccessConfig.enable) {
            for (let i = 0; i < this.state.remoteAccessConfig.users.length * 5; i++) {
                const remoteAccessNetwork = util.parseIP(`10.123.${i}.0`);
                connectedSubnets.push({network: remoteAccessNetwork, name: __("remote_access.navbar.remote_access"), subnet: "/24", href: "#remote_access"});
            }
            const remoteAccessNetwork = util.parseIP(`10.123.123.0`);
            connectedSubnets.push({network: remoteAccessNetwork, name: __("remote_access.navbar.remote_access"), subnet: "/24", href: "#remote_access"});
        }
//#endif

        const equalSubnets = connectedSubnets.filter((subnet) => {
            return connectedSubnets.findIndex(v => v.network === subnet.network && v.name !== subnet.name) !== -1;
        });

        const conflictSubnets: [{
            network: string,
            name: string,
            href: string,
            dhcp?: boolean,
        },
        {
            network: string,
            name: string,
            href: string,
            dhcp?: boolean,
        }][] = [];
        equalSubnets.map((subnet, idx) => {
            const conflict = equalSubnets.slice(idx + 1).filter(v => v.network === subnet.network);
            for (const c of conflict) {
                conflictSubnets.push([{
                    network: `${util.unparseIP(subnet.network)}${subnet.subnet}`,
                    name: subnet.name,
                    href: subnet.href,
                    dhcp: subnet.dhcp,
                },
            {
                    network: `${util.unparseIP(c.network)}${c.subnet}`,
                    name: c.name,
                    href: c.href,
                    dhcp: c.dhcp,
            }]);
            }
        })

        return conflictSubnets;
    }

    checkDNS() {
        const interfacesMissingDNS: string[] = [];
//#if MODULE_ETHERNET_AVAILABLE
        if (this.state.ethernetConfig.ip !== "0.0.0.0" &&
            this.state.ethernetConfig.dns === "0.0.0.0" &&
            this.state.ethernetConfig.dns2 === "0.0.0.0" &&
            this.state.ethernetConfig.enable_ethernet) {
                interfacesMissingDNS.push(translate_unchecked("ethernet.navbar.ethernet"));
        }
//#endif

//#if MODULE_WIFI_AVAILABLE
        if (this.state.wifiConfig.ip !== "0.0.0.0" &&
            this.state.wifiConfig.dns === "0.0.0.0" &&
            this.state.wifiConfig.dns2 === "0.0.0.0" &&
            this.state.wifiConfig.enable_sta) {
                interfacesMissingDNS.push(translate_unchecked("wifi.navbar.wifi_sta"));
        }
//#endif

        return interfacesMissingDNS;
    }

    render(props: {}, state: Readonly<NetworkConfig>) {
        if (!util.render_allowed())
            return <SubPage name="network" />;

        const conflictSubnets = this.getEqualSubnets();
        useEffect(() => {
            if (conflictSubnets.length > 0) {
                util.add_status_alert(
                    "conflict_subnets",
                    "danger",
                    () => __("network.status.subnet_conflict"),
                    () => __("network.status.subnet_text")(conflictSubnets)
                );
            } else {
                util.remove_status_alert("conflict_subnets");
            }
        }, [JSON.stringify(conflictSubnets)]);

        const interfacesMissingDNS = this.checkDNS();
        useEffect(() => {
            if (interfacesMissingDNS.length > 0) {
                util.add_status_alert(
                    "dns_not_configured",
                    "warning",
                    () => __("network.status.dns_not_configured"),
                    () => __("network.status.dns_not_configured_text")(interfacesMissingDNS)
            );
            } else {
                util.remove_status_alert("dns_not_configured");
            }
        }, [JSON.stringify(interfacesMissingDNS)]);

        const cert_state = API.get_unchecked('certs/state');
        const certs = cert_state == null ? [] : cert_state.certs.map((c: any) => [c.id.toString(), c.name] as [string, string]);

        let show_transport_mode;
//#if OPTIONS_WEB_SERVER_HTTPS_ENABLED
//#if OPTIONS_WEB_SERVER_HTTP_ENABLED
        // Both enabled
        show_transport_mode = true;
//#else
        // HTTPS on, HTTP off
        show_transport_mode = state.transport_mode != TransportMode.Secure;
//#endif
//#else
//#if OPTIONS_WEB_SERVER_HTTP_ENABLED
        // HTTPS off, HTTP on
        show_transport_mode = state.transport_mode != TransportMode.Insecure;
//#else
        // HTTPS off, HTTP off (can't happen)
        show_transport_mode = false;
//#endif
//#endif

        return (
            <SubPage name="network">
                <ConfigForm id="network_config_form"
                            title={__("network.content.network")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("network.content.hostname")}>
                        <InputTextPatterned maxLength={32}
                                   pattern="[a-zA-Z0-9\-]*"
                                   required
                                   value={state.hostname}
                                   onValue={this.set("hostname")}
                                   invalidFeedback={__("network.content.hostname_invalid")}
                                   />
                    </FormRow>

                    <FormRow label={__("network.content.enable_mdns")}>
                        <Switch desc={__("network.content.enable_mdns_desc")}
                                checked={state.enable_mdns}
                                onClick={this.toggle('enable_mdns')}/>
                    </FormRow>

                    {show_transport_mode ?
                        <FormRow label={__("network.content.transport_mode")}>
                            <InputSelect
                                items={[
                                    [TransportMode.Insecure.toString(),          __("network.content.insecure")],
                                    [TransportMode.Secure.toString(),            __("network.content.secure")],
                                    [TransportMode.InsecureAndSecure.toString(), __("network.content.insecure_and_secure")],
                                ]}
                                value={state.transport_mode}
                                onValue={(v) => this.setState({transport_mode: parseInt(v)})}
                            />
                        </FormRow>
                    : undefined}

                    <Collapse in={state.transport_mode != TransportMode.Secure}>
                        <div>
                            <FormRow label={__("network.content.web_server_port")}>
                                <InputNumber
                                    required={state.transport_mode != TransportMode.Secure}
                                    min={1}
                                    max={65535}
                                    value={state.web_server_port}
                                    onValue={this.set("web_server_port")}
                                />
                            </FormRow>
                        </div>
                    </Collapse>

                    <Collapse in={state.transport_mode != TransportMode.Insecure}>
                        <div>
                            <FormRow label={__("network.content.web_server_port_secure")}>
                                <InputNumber
                                    required={state.transport_mode != TransportMode.Insecure}
                                    min={1}
                                    max={65535}
                                    value={state.web_server_port_secure}
                                    onValue={this.set("web_server_port_secure")}
                                />
                            </FormRow>

                            <FormRow label={__("network.content.cert")}>
                                <InputSelect
                                    disabled={cert_state == null}
                                    required={state.transport_mode != TransportMode.Insecure}
                                    items={[["-1", __("network.content.no_cert")] as [string, string]].concat(certs)}
                                    value={state.cert_id}
                                    onValue={(v) => {
                                        const cid = parseInt(v);
                                        this.setState({cert_id: cid});
                                        if (cid == -1) {this.setState({key_id: -1});}
                                    }}
                                />
                            </FormRow>

                            <FormRow label={__("network.content.key")}>
                                <InputSelect
                                    disabled={cert_state == null || state.cert_id == -1}
                                    required={state.transport_mode != TransportMode.Insecure && state.cert_id != -1}
                                    items={state.cert_id == -1 ? [["-1", __("network.content.no_key")] as [string, string]] : certs}
                                    placeholder={__("select")}
                                    value={state.key_id}
                                    onValue={(v) => this.setState({key_id: parseInt(v)})}
                                />
                            </FormRow>
                        </div>
                    </Collapse>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function pre_init() {
}

export function init() {
}
