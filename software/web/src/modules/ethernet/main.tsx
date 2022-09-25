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

import { h, render, Fragment, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "src/ts/components/config_component";
import { ConfigForm } from "src/ts/components/config_form";
import { FormRow } from "src/ts/components/form_row";
import { IPConfiguration } from "src/ts/components/ip_configuration";

type EthernetConfig = API.getType['ethernet/config'];

export class Ethernet extends ConfigComponent<'ethernet/config'> {
    ipconfig_valid: boolean = true;

    constructor() {
        super('ethernet/config',
              __("ethernet.script.config_failed"),
              __("ethernet.script.reboot_content_changed"));
    }

    override isSaveAllowed(cfg: EthernetConfig) { return this.ipconfig_valid; }

    override transformSave(cfg: EthernetConfig) {
        cfg.dns = cfg.dns == "" ? "0.0.0.0" : cfg.dns;
        cfg.dns2 = cfg.dns2 == "" ? "0.0.0.0" : cfg.dns2;
        return cfg;
    }

    render(props: {}, state: Readonly<EthernetConfig>) {
        if (!state)
            return (<></>);

        return (
            <>
                <ConfigForm id="ethernet_config_form"
                            title={__("ethernet.content.ethernet")}
                            onSave={this.save}
                            onDirtyChange={(d) => this.ignore_updates = d}>
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

function update_ethernet_state() {
    let state = API.default_updater('ethernet/state', ['ip', 'full_duplex', 'link_speed'], false);

    if (state.ip != "0.0.0.0") {
        $('#ethernet_state_ip').html(state.ip);
    } else {
        $('#ethernet_state_ip').html("");
    }
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('ethernet/state', update_ethernet_state);
}

export function init() {

}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-ethernet').prop('hidden', !module_init.ethernet);
}
