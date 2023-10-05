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

import { SubPage } from "src/ts/components/sub_page";
import { PageHeader } from "../../ts/components/page_header";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { Button } from "react-bootstrap";

interface SunSpecMeterState {
    host: string;
    port: number;
    device_address: number;
    discovery_log: string;
}

export class SunSpecMeter extends Component<{}, SunSpecMeterState> {
    constructor()
    {
        super();

        this.state = {
            host: 'batti.ddns.net',//'192.168.0.64',
            port: 5002,//502,
            device_address: 1,
            discovery_log: ''
        } as any;

        util.addApiEventListener('sun_spec_meter/discovery_log', (e) => {
            if (e.data == '<<<clear_discovery_log>>>') {
                this.setState({discovery_log: ''})
            }
            else {
                this.setState({discovery_log: this.state.discovery_log + e.data + '\n'})
            }
        }, false);
    }

    render(props: {}, state: SunSpecMeterState) {
        if (!util.render_allowed())
            return <></>

        return (
            <SubPage>
                <PageHeader title={__("sun_spec_meter.content.sun_spec_meter")}/>

                <FormRow label={__("sun_spec_meter.content.host")}>
                    <InputText maxLength={32}
                                required
                                value={state.host}
                                onValue={(v) => this.setState({host: v})}
                                />
                </FormRow>


                <FormRow label={__("sun_spec_meter.content.port")} label_muted={__("sun_spec_meter.content.port_muted")}>
                    <InputNumber required
                                min={1}
                                max={65535}
                                value={state.port}
                                onValue={(v) => this.setState({port: v})}
                                />
                </FormRow>

                <FormRow label={__("sun_spec_meter.content.device_address")} label_muted={__("sun_spec_meter.content.device_address_muted")}>
                    <InputNumber required
                                min={0}
                                max={247}
                                value={state.device_address}
                                onValue={(v) => this.setState({device_address: v})}
                                />
                </FormRow>

                <FormRow label={__("sun_spec_meter.content.discovery_log")}>
                    <textarea class="text-monospace mb-1 form-control"
                              readonly
                              rows={20}
                              style="resize: both; width: 100%; white-space: pre; line-height: 1.2; text-shadow: none; font-size: 0.875rem;">
                        {state.discovery_log}
                    </textarea>
                </FormRow>

                <FormRow label="">
                    <Button variant="primary"
                            className="form-control"
                            onClick={() => {
                                API.call('sun_spec_meter/start_discovery',
                                        {host: state.host, port: state.port, device_address: state.device_address},
                                        __("sun_spec_meter.content.start_discovery_failed"));
                            }}>
                        {__("sun_spec_meter.content.start_discovery")}
                    </Button>
                </FormRow>
            </SubPage>
        );
    }
}

render(<SunSpecMeter/>, $('#sun_spec_meter')[0])

export function add_event_listeners(source: API.APIEventTarget) {}

export function init() {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-sun_spec_meter').prop('hidden', !module_init.sun_spec_meter);
}
