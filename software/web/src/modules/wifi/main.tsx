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
import { h, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";
export { WifiAP } from "./wifi_ap";
export { WifiSTA } from "./wifi_sta";
import { wifi_symbol } from "./wifi_sta";
import { FormRow } from "../../ts/components/form_row";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { Wifi } from "react-feather";

export function WifiSTANavbar() {
    return <NavbarItem name="wifi_sta" module="wifi" title={__("wifi.navbar.wifi_sta")} symbol={<Wifi />} />;
}

export function WifiAPNavbar() {
    return (
        <NavbarItem name="wifi_ap" module="wifi" title={__("wifi.navbar.wifi_ap")} symbol={
            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><g id="g22" transform="rotate(90,16.055,16.055)"><path d="m 5,12.55 a 11,11 0 0 1 14.08,0" id="path2"/><path d="m 8.53,16.11 a 6,6 0 0 1 6.95,0" id="path6"/></g><line x1="12" y1="12" x2="12.01" y2="12" id="line8"/><g id="g22-3" transform="rotate(-90,8.0373834,16.042617)"><path d="m 5,12.55 a 11,11 0 0 1 14.08,0" id="path2-6"/><path d="m 8.53,16.11 a 6,6 0 0 1 6.95,0" id="path6-7"/></g></svg>
        } />);
}

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
            return <StatusSection name="wifi" />;

        let sta_row = !state.sta_config.enable_sta ? <></>
            : <FormRow label={__("wifi.status.wifi_connection")}
                       label_infix={<span class="pl-2 pr-2" style="vertical-align: top;">{wifi_symbol(state.state.sta_rssi)}</span>}
                       label_muted={state.state.sta_ip != "0.0.0.0" ? state.state.sta_ip : ""}>
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
            : <FormRow label={__("wifi.status.wifi_ap")}>
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

        return <StatusSection name="wifi">
                {ap_row}
                {sta_row}
            </StatusSection>;
    }
}

export function init() {
}
