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

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { h, Component, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputTextPatterned } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Settings } from "react-feather";
import { StatusSection } from "ts/components/status_section";
import { IndicatorGroup } from "ts/components/indicator_group";

export function NetworkNavbar() {
    return <NavbarItem name="network" module="network" title={__("network.navbar.network")} symbol={<Settings />} />;
}

type NetworkConfig = API.getType["network/config"];

export class Network extends ConfigComponent<'network/config', {status_ref: RefObject<NetworkStatus> }> {
    constructor() {
        super('network/config',
              () => __("network.script.save_failed"),
              () => __("network.script.reboot_content_changed"));
    }

    render(props: {}, state: Readonly<NetworkConfig>) {
        if (!util.render_allowed())
            return <SubPage name="network" />;

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

                    <FormRow label={__("network.content.web_server_port")}>
                        <InputNumber required
                                     min={1}
                                     max={65535}
                                     value={state.web_server_port}
                                     onValue={this.set("web_server_port")}/>
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

interface NetworkStatusState {
    ethernet: API.getType["ethernet/state"];
    wifi: API.getType["wifi/state"];
    wireguardConfig: API.getType["wireguard/config"];
    apConfig: API.getType["wifi/ap_config"];
    remoteAccessConfig: API.getType["remote_access/config"];
}

export class NetworkStatus extends Component<{}, NetworkStatusState> {
    constructor() {
        super();

        util.addApiEventListener('ethernet/state', () => {
            this.setState({ ethernet: API.get('ethernet/state') });
        });
        util.addApiEventListener('wifi/state', () => {
            this.setState({ wifi: API.get('wifi/state') });
        });
        util.addApiEventListener('wireguard/config', () => {
            this.setState({ wireguardConfig: API.get('wireguard/config') });
        });
        util.addApiEventListener('wifi/ap_config', () => {
            this.setState({ apConfig: API.get('wifi/ap_config') });
        });
        util.addApiEventListener('remote_access/config', () => {
            this.setState({ remoteAccessConfig: API.get('remote_access/config') });
        });
    }

    getEqualSubnets(state: Readonly<NetworkStatusState>) {
        const connectedSubnets: {
            network: number;
            name: string;
        }[] = [];

        const wifiIP = util.parseIP(state.wifi.sta_ip);
        if (wifiIP !== 0) {
            const wifiSubnet = util.parseIP(state.wifi.sta_subnet);
            const wifiNetwork = wifiIP & wifiSubnet;
            connectedSubnets.push({network: wifiNetwork, name: __("network.status.sta")});
        }


        const ethernetIP = util.parseIP(state.ethernet.ip);
        if (ethernetIP !== 0) {
            const ethernetSubnet = util.parseIP(state.ethernet.subnet);
            const ethernetNetwork = ethernetIP & ethernetSubnet;
            connectedSubnets.push({network: ethernetNetwork, name: __("network.status.ethernet")});
        }

        if (state.apConfig.enable_ap && !state.apConfig.ap_fallback_only) {
            const apIP = util.parseIP(state.apConfig.ip);
            const apSubnet = util.parseIP(state.apConfig.subnet);
            const apNetwork = apIP & apSubnet;
            connectedSubnets.push({network: apNetwork, name: __("network.status.ap")});
        }

        if (state.wireguardConfig.enable) {
            const wireguardIP = util.parseIP(state.wireguardConfig.internal_ip);
            const wireguardSubnet = util.parseIP(state.wireguardConfig.internal_subnet);
            const wireguardNetwork = wireguardIP & wireguardSubnet;
            connectedSubnets.push({network: wireguardNetwork, name: __("network.status.wireguard")});
        }

        if (state.remoteAccessConfig.enable) {
            for (let i = 0; i < state.remoteAccessConfig.users.length * 5; i++) {
                const remoteAccessNetwork = util.parseIP(`10.123.${i}.0`);
                connectedSubnets.push({network: remoteAccessNetwork, name: __("network.status.remote_access")});
            }
            const remoteAccessNetwork = util.parseIP(`10.123.123.0`);
            connectedSubnets.push({network: remoteAccessNetwork, name: __("network.status.remote_access")});
        }

        const equalSubnets = connectedSubnets.filter((subnet) => {
            return connectedSubnets.findIndex(v => v.network === subnet.network && v.name !== subnet.name) !== -1;
        });

        return equalSubnets;
    }

    render(props: {}, state: Readonly<NetworkStatusState>) {
        if (!util.render_allowed()) {
            return <StatusSection name="network" />;
        }
        const equalSubnets = this.getEqualSubnets(state);
        const wifiAndEthernetActive = util.parseIP(state.wifi.sta_ip) !== 0 && util.parseIP(state.ethernet.ip) !== 0;
        const indicatorValue = equalSubnets.length > 0 ? 2 : wifiAndEthernetActive ? 1 : 0;

        return <StatusSection name="network">
            <FormRow label={__("network.content.network")} help={__("network.status.status_help")(equalSubnets)}>
                <IndicatorGroup
                    style="width: 100%"
                    class="flex-wrap"
                    value={indicatorValue}
                    items={[
                        ["success", "OK"],
                        ["warning", __("network.status.sta_and_ethernet")],
                        ["danger", __("network.status.subnet_conflict")],
                    ]} />
            </FormRow>
        </StatusSection>;
    }
}

export function init() {
}
