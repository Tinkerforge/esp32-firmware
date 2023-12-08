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

import { Component, h, render, Fragment } from "preact";
import { __ } from "../../ts/translation";

import { WifiAP } from "./wifi_ap";
import { WifiSTA, wifi_symbol } from "./wifi_sta";
import { FormRow } from "../../ts/components/form_row";
import { IndicatorGroup } from "../../ts/components/indicator_group";

render(<WifiAP />, $("#wifi-ap")[0]);
render(<WifiSTA />, $("#wifi-sta")[0]);

interface WifiStatusState {
    state: API.getType["wifi/state"];
    ap_config: API.getType["wifi/ap_config"];
    sta_config: API.getType["wifi/sta_config"];
}

export class WifiStatus extends Component<{}, WifiStatusState> {
    constructor() {
        super();

        util.addApiEventListener('wifi/state', () => {
            this.setState({state: API.get('wifi/state')})
        });

        util.addApiEventListener('wifi/ap_config', () => {
            this.setState({ap_config: API.get('wifi/ap_config')})
        });

        util.addApiEventListener('wifi/sta_config', () => {
            this.setState({sta_config: API.get('wifi/sta_config')})
        });
    }

    render(props: {}, state: WifiStatusState) {
        if (!util.render_allowed())
            return <></>;

        let sta_row = !state.sta_config.enable_sta ? <></>
            : <FormRow label={__("wifi.status.wifi_connection")}
                       label_infix={<span class="pl-2 pr-2" style="vertical-align: top;">{wifi_symbol(state.state.sta_rssi)}</span>}
                       label_muted={state.state.sta_ip != "0.0.0.0" ? state.state.sta_ip : ""}
                       labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <IndicatorGroup
                    style="width: 100%"
                    class="flex-wrap"
                    value={state.state.connection_state}
                    items={[
                        ["primary", __("wifi.status.not_configured")],
                        ["danger", __("wifi.status.not_connected")],
                        ["warning", __("wifi.status.connecting")],
                        ["success", __("wifi.status.connected")],
                    ]}/>
            </FormRow>;

        let ap_row = !state.ap_config.enable_ap ? <></>
            : <FormRow label={__("wifi.status.wifi_ap")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <IndicatorGroup
                    style="width: 100%"
                    class="flex-wrap"
                    value={state.state.ap_state}
                    items={[
                        ["primary", __("wifi.status.deactivated")],
                        ["success", __("wifi.status.activated")],
                        ["success", __("wifi.status.fallback_inactive")],
                        ["danger",  __("wifi.status.fallback_active")],
                    ]}/>
            </FormRow>;

        return <>
                {sta_row}
                {ap_row}
            </>;
    }
}

render(<WifiStatus />, $("#status-wifi")[0]);

export function add_event_listeners(source: API.APIEventTarget) {}

export function init() {}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-wifi-sta").prop("hidden", !module_init.wifi);
    $("#sidebar-wifi-ap").prop("hidden", !module_init.wifi);
}
