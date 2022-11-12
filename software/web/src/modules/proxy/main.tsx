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

import { h, render, Fragment, Component} from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";

import { FormRow } from "../../ts/components/form_row";
import { Table} from "react-bootstrap";

interface ProxyState {
    devices: Readonly<API.getType['proxy/devices']>;
    error_counters: Readonly<API.getType['proxy/error_counters']>;
}


export class Proxy extends Component<{}, ProxyState> {
    constructor() {
        super();

        util.eventTarget.addEventListener('proxy/devices', () => {
            this.setState({devices: API.get('proxy/devices')});
        });

        util.eventTarget.addEventListener('proxy/error_counters', () => {
            this.setState({error_counters: API.get('proxy/error_counters')});
        });

    }

    render(props: {}, state: Readonly<ProxyState>) {
        if (!state || !state.devices)
            return (<></>);

        return (
            <>
                <PageHeader title={__("proxy.content.proxy")} />
                <FormRow label={__("proxy.content.bricklet_table")}>
                    <Table hover borderless>
                        <thead class="thead-light">
                            <tr>
                                <th scope="col">{__("proxy.content.port")}</th>
                                <th scope="col">{__("proxy.content.UID")}</th>
                                <th scope="col">{__("proxy.content.device_type")}</th>
                            </tr>
                        </thead>
                        <tbody>
                            {state.devices.length == 0 ? <tr><td colSpan={3}>{__("proxy.script.no_bricklets")}</td></tr>
                                : state.devices.map(d => <tr>
                                <td>{d.port}</td>
                                <td>{d.uid}</td>
                                <td>{d.name == "unknown device" ? __("proxy.script.unknown_device") : d.name}</td>
                            </tr>)}
                        </tbody>
                    </Table>
                </FormRow>

                <FormRow label={__("proxy.content.error_counters")}>
                    <Table hover borderless>
                        <thead class="thead-light">
                            <tr>
                                <th scope="col">{__("proxy.content.port")}</th>
                                <th scope="col">{__("proxy.content.spitfp_checksum")}</th>
                                <th scope="col">{__("proxy.content.spitfp_frame")}</th>
                                <th scope="col">{__("proxy.content.tfp_frame")}</th>
                                <th scope="col">{__("proxy.content.tfp_unexpected")}</th>
                            </tr>
                        </thead>
                        <tbody>
                            {Object.keys(state.error_counters).map(port => <tr>
                                <td>{port}</td>
                                <td>{state.error_counters[port].SpiTfpChecksum}</td>
                                <td>{state.error_counters[port].SpiTfpFrame}</td>
                                <td>{state.error_counters[port].TfpFrame}</td>
                                <td>{state.error_counters[port].TfpUnexpected}</td>
                            </tr>)}
                        </tbody>
                    </Table>
                </FormRow>
            </>
        )
    }
}

render(<Proxy />, $('#proxy')[0]);

export function init() {}

export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-proxy').prop('hidden', !module_init.proxy);
}
