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

import $ from "../../ts/jq";

import * as util from "../../ts/util";
import * as API from "../../ts/api";


import { h, render, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";

import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { InputPassword } from "../../ts/components/input_password";
import { Switch } from "../../ts/components/switch";
import { IPConfiguration } from "src/ts/components/ip_configuration";
import { Slash } from "react-feather";
import { InputIP } from "src/ts/components/input_ip";
import { IndicatorGroup } from "src/ts/components/indicator_group";

type WireGuardConfig = API.getType['wireguard/config'];

export class WireGuard extends ConfigComponent<'wireguard/config'> {
    ipconfig_valid: boolean = true;
    constructor() {
        super('wireguard/config',
              __("wireguard.script.save_failed"),
              __("wireguard.script.reboot_content_changed"));
    }

    render(props: {}, state: Readonly<WireGuardConfig>) {
        if (!util.render_allowed())
            return <></>

        return (
            <>
                <ConfigForm id="wireguard_config_form" title={__("wireguard.content.wireguard")} isModified={this.isModified()} onSave={() => this.save()} onReset={() => this.reset()} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("wireguard.content.enable_wireguard")}>
                        <Switch desc={__("wireguard.content.enable_wireguard_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}/>
                    </FormRow>

                    <FormRow label={__("wireguard.content.default_if")}>
                        <Switch desc={__("wireguard.content.default_if_desc")}
                                checked={state.make_default_interface}
                                onClick={this.toggle('make_default_interface')}/>
                    </FormRow>

                    <IPConfiguration
                        onValue={(v) => this.setState({internal_ip: v.ip, internal_subnet: v.subnet, internal_gateway: v.gateway})}
                        value={{ip: state.internal_ip, subnet: state.internal_subnet, gateway: state.internal_gateway}}
                        setValid={(v) => this.ipconfig_valid = v}

                        ip_label={__("wireguard.content.internal_ip")}
                        subnet_label={__("wireguard.content.internal_subnet")}
                        gateway_label={__("wireguard.content.internal_gateway")}
                        />

                    <FormRow label={__("wireguard.content.remote_host")}>
                        <InputText required
                                   maxLength={64}
                                   value={state.remote_host}
                                   onValue={this.set("remote_host")}/>
                    </FormRow>

                    <FormRow label={__("wireguard.content.port")}>
                        <InputNumber required
                                     min={1}
                                     max={65535}
                                     value={state.remote_port}
                                     onValue={this.set("remote_port")}/>
                    </FormRow>

                    <FormRow label={__("wireguard.content.private_key")}>
                        <InputPassword maxLength={44}
                                       value={state.private_key}
                                       onValue={this.set("private_key")}
                                       />
                    </FormRow>

                    <FormRow label={__("wireguard.content.remote_public_key")}>
                        <InputPassword maxLength={44}
                                       value={state.remote_public_key}
                                       onValue={this.set("remote_public_key")}
                                       />
                    </FormRow>

                    <FormRow label={__("wireguard.content.preshared_key")} label_muted={__("wireguard.content.preshared_key_muted")}>
                        <InputPassword maxLength={44}
                                       value={state.preshared_key}
                                       onValue={this.set("preshared_key")}
                                       clearSymbol={<Slash/>}
                                       clearPlaceholder={__("wireguard.script.preshared_key_unused")}
                                       allowAPIClear
                                       />
                    </FormRow>

                    <FormRow label={__("wireguard.content.allowed_ip")}>
                        <InputIP invalidFeedback={__("wireguard.content.allowed_ip_invalid")}
                            required
                            value={state.allowed_ip}
                            onValue={this.set("allowed_ip")}/>
                    </FormRow>

                    <FormRow label={__("wireguard.content.allowed_subnet")}>
                        <InputIP invalidFeedback={__("wireguard.content.allowed_subnet_invalid")}
                            required
                            value={state.allowed_subnet}
                            onValue={this.set("allowed_subnet")}/>
                    </FormRow>
                </ConfigForm>
            </>
        )
    }
}

render(<WireGuard/>, $('#wireguard')[0])


interface WireGuardStatusState {
    state: API.getType['wireguard/state']
    config: API.getType['wireguard/config'];
}

export class WireGuardStatus extends Component<{}, WireGuardStatusState>
{
    constructor()
    {
        super();

        util.addApiEventListener('wireguard/state', () => {
            this.setState({state: API.get('wireguard/state')})
        });

        util.addApiEventListener('wireguard/config', () => {
            this.setState({config: API.get('wireguard/config')})
        });
    }

    render(props: {}, state: WireGuardStatusState)
    {
        if (!util.render_allowed() || !state.config.enable)
            return <></>;

        return <>
                <FormRow label={__("wireguard.status.connection")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={state.state.state}
                        items={[
                            ["primary", __("wireguard.status.not_configured")],
                            ["warning", __("wireguard.status.waiting_for_timesync")],
                            ["warning", __("wireguard.status.not_connected")],
                            ["success", __("wireguard.status.connected")]
                        ]}/>
                </FormRow>
            </>;
    }
}

render(<WireGuardStatus/>, $('#status-wireguard')[0]);

export function init() {}

export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-wireguard').prop('hidden', !module_init.wireguard);
}
