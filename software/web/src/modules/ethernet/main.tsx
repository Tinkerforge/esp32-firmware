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

import * as API from "../../ts/api";
import * as util from "../../ts/util";

import { h, render, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "src/ts/components/config_component";
import { ConfigForm } from "src/ts/components/config_form";
import { FormRow } from "src/ts/components/form_row";
import { IPConfiguration } from "src/ts/components/ip_configuration";
import { IndicatorGroup } from "src/ts/components/indicator_group";

type EthernetConfig = API.getType['ethernet/config'];

export class Ethernet extends ConfigComponent<'ethernet/config'> {
    ipconfig_valid: boolean = true;

    constructor() {
        super('ethernet/config',
              __("ethernet.script.config_failed"),
              __("ethernet.script.reboot_content_changed"));
    }

    override async isSaveAllowed(cfg: EthernetConfig) { return this.ipconfig_valid; }

    override async transformSave(cfg: EthernetConfig) {
        cfg.dns = cfg.dns == "" ? "0.0.0.0" : cfg.dns;
        cfg.dns2 = cfg.dns2 == "" ? "0.0.0.0" : cfg.dns2;
        return cfg;
    }

    render(props: {}, state: Readonly<EthernetConfig>) {
        if (!util.render_allowed())
            return <></>

        return (
            <>
                <ConfigForm id="ethernet_config_form"
                            title={__("ethernet.content.ethernet")}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={(d) => this.ignore_updates = d}
                            isModified={this.isModified()}>
                    <FormRow label={__("ethernet.content.enable")}>
                        <Switch desc={__("ethernet.content.enable_desc")}
                                checked={state.enable_ethernet}
                                onClick={this.toggle('enable_ethernet')}/>
                    </FormRow>

                    <IPConfiguration
                        showDhcp
                        showDns
                        onValue={(v) => this.setState(v)}
                        value={state}
                        setValid={(v) => this.ipconfig_valid = v}
                        />

                </ConfigForm>
            </>
        );
    }
}

render(<Ethernet/>, $('#ethernet')[0])

interface EthernetStatusState {
    state: API.getType['ethernet/state']
    config: API.getType['ethernet/config'];
}

export class EthernetStatus extends Component<{}, EthernetStatusState>
{
    constructor()
    {
        super();

        util.addApiEventListener('ethernet/state', () => {
            this.setState({state: API.get('ethernet/state')})
        });

        util.addApiEventListener('ethernet/config', () => {
            this.setState({config: API.get('ethernet/config')})
        });
    }

    render(props: {}, state: EthernetStatusState)
    {
        if (!util.allow_render || !state.config.enable_ethernet)
            return <></>;

        return <>
                <FormRow label={__("ethernet.status.ethernet_connection")} label_muted={state.state.ip != "0.0.0.0" ? state.state.ip : ""} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={state.state.connection_state}
                        items={[
                            ["primary", __("ethernet.status.not_configured")],
                            ["danger", __("ethernet.status.not_connected")],
                            ["warning", __("ethernet.status.connecting")],
                            ["success", __("ethernet.status.connected")],
                        ]}/>
                </FormRow>
            </>;
    }
}

render(<EthernetStatus/>, $('#status-ethernet')[0]);

export function add_event_listeners(source: API.APIEventTarget) {}

export function init() {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-ethernet').prop('hidden', !module_init.ethernet);
}
