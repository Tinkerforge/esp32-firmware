/* esp32-firmware
 * Copyright (C) 2025 Matthias Bolte <matthias@tinkerforge.com>
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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import * as options from "../../options";
import { __ } from "../../ts/translation";
import { h, Fragment, Component } from "preact";
import { Button, Alert } from "react-bootstrap";
import { FormRow } from "../../ts/components/form_row";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { InputTextPatterned } from "../../ts/components/input_text";
import { InputHost } from "../../ts/components/input_host";
import { OutputTextarea } from "../../ts/components/output_textarea";
import { NavbarItem } from "../../ts/components/navbar_item";
import { PageHeader } from "../../ts/components/page_header";
import { SubPage } from "../../ts/components/sub_page";
import { CollapsedSection } from "../../ts/components/collapsed_section";
//#if MODULE_METERS_AVAILABLE
import { MeterClassID } from "../meters/meter_class_id.enum";
import { MeterConfig } from "../meters/types";
//#endif

export function RCTPowerDebugNavbar() {
    return (
        <NavbarItem name="rct_power_debug" module="rct_power_debug" title={__("rct_power_debug.navbar.rct_power_debug")} symbol={
            <svg fill="currentColor" viewBox="0 0 24 24" width="24" height="24" xmlns="http://www.w3.org/2000/svg"><g><g stroke="currentColor"><path d="M19.587 12a7.442 7.442 0 1 1-7.442-7.441A7.441 7.441 0 0 1 19.587 12" fill="#fff" stroke-width="1.674"/><path d="M22.192 16.242a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M7.68 16.242a2.79 2.79 0 1 1-2.791-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M7.568 7.814a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M22.192 7.814a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M14.936 20.373a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M14.936 3.628a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path class="cls-3" d="M5.533 8.44 11.698 12c0-.595.893-.595.894 0l6.165-3.56-.224-.388-6.165 3.56c-.024.032 0-7.119 0-7.119-.094-.006-.446 0-.446 0s.01 7.146 0 7.12L5.755 8.054c-.067.106-.222.386-.222.386z" stroke-width=".5" fill="#000"/><path class="cls-3" d="M18.758 15.56 12.591 12c0 .595-.893.595-.893 0l-6.165 3.56.226.388s6.174-3.578 6.163-3.562v7.121s.426.002.446 0c0 0-.019-7.138 0-7.12l6.168 3.56s.208-.36.222-.387z" stroke-width=".5"/></g></g></svg>
        } />);
}

interface RCTPowerDebugToolState {
//#if MODULE_METERS_AVAILABLE
    meter_configs: {[meter_slot: number]: MeterConfig};
    meter_slot: number;
    meter_host: string;
    meter_port: number;
    meter_device_address: number;
//#endif
    was_validated: boolean;
    custom_host: string;
    custom_port: number;
    id: number;
    timeout: number;
    waiting: boolean;
    cookie: number;
    result: string;
}

export class RCTPowerDebugTool extends Component<{}, RCTPowerDebugToolState> {
    constructor() {
        super();

        this.state = {
//#if MODULE_METERS_AVAILABLE
            meter_configs: {},
            meter_slot: -1,
            meter_host: "",
            meter_port: 8899,
//#endif
            was_validated: false,
            custom_host: "",
            custom_port: 8899,
            id: 0,
            timeout: 2000,
            waiting: false,
            cookie: null,
            result: "",
        } as any;

//#if MODULE_METERS_AVAILABLE
        for (let meter_slot = 0; meter_slot < options.METERS_MAX_SLOTS; ++meter_slot) {
            util.addApiEventListener_unchecked(`meters/${meter_slot}/config`, () => {
                let config = API.get_unchecked(`meters/${meter_slot}/config`);

                this.setState((prevState) => ({
                    meter_configs: {
                        ...prevState.meter_configs,
                        [meter_slot]: config
                    }
                }));
            });
        }
//#endif

        util.addApiEventListener('rct_power_debug/read_result', () => {
            let read_result = API.get('rct_power_debug/read_result');

            if (!this.state.waiting || read_result.cookie !== this.state.cookie) {
                return;
            }

            let result = "Success";

            if (read_result.error !== null) {
                result = "Error: " + read_result.error;
            }
            else if (read_result.value !== null) {
                result = "" + read_result.value;
            }

            this.setState({waiting: false, cookie: null, result: result});
        });
    }

    render() {
//#if MODULE_METERS_AVAILABLE
        let meter_items: [string, string][] = [["-1", __("rct_power_debug.content.meter_none")]];

        for (let meter_slot = 0; meter_slot < options.METERS_MAX_SLOTS; ++meter_slot) {
            if (util.hasValue(this.state.meter_configs[meter_slot])) {
                let meter_config = this.state.meter_configs[meter_slot];

                if (meter_config[0] == MeterClassID.RCTPower) {
                    meter_items.push([meter_slot.toString(), meter_config[1].display_name]);
                }
            }
        }
//#endif

        return <form
                class={"needs-validation" + (this.state.was_validated ? " was-validated" : "")}
                noValidate
                onSubmit={async (e) => {
                    e.preventDefault();
                    e.stopPropagation();

                    this.setState({was_validated: false});

                    if (!(e.target as HTMLFormElement).checkValidity() || (e.target as HTMLFormElement).querySelector(".is-invalid")) {
                        this.setState({was_validated: true});
                        return;
                    }

                    let cookie: number = Math.floor(Math.random() * 0xFFFFFFFF);

                    this.setState({was_validated: false, waiting: true, cookie: cookie, result: ""}, async () => {
                        let result = "<unknown>";

//#if MODULE_METERS_AVAILABLE
                        let host = this.state.meter_slot >= 0 ? this.state.meter_host : this.state.custom_host;
                        let port = this.state.meter_slot >= 0 ? this.state.meter_port : this.state.custom_port;
//#else
                        let host = this.state.custom_host;
                        let port = this.state.custom_port;
//#endif

                        try {
                            result = await (await util.put("/rct_power_debug/read", {
                                host: host,
                                port: port,
                                id: this.state.id,
                                timeout: this.state.timeout,
                                cookie: cookie,
                            }, true)).text();
                        }
                        catch (e) {
                            result = e.message.replace("400(Bad Request) ", "");
                        }

                        if (result.length > 0) {
                            this.setState({waiting: false, cookie: null, result: "Error: " + result});
                        }
                    });
                }}>
{/*#if MODULE_METERS_AVAILABLE*/}
            <FormRow label={__("rct_power_debug.content.meter")}>
                <InputSelect
                    required
                    disabled={this.state.waiting}
                    items={meter_items}
                    value={util.hasValue(this.state.meter_slot) ? this.state.meter_slot.toString() : "-1"}
                    onValue={(v) => {
                        let meter_slot = parseInt(v);
                        let meter_host = "";
                        let meter_port = 8899;

                        if (meter_slot >= 0) {
                            let meter_config = this.state.meter_configs[meter_slot];

                            if (meter_config[0] == MeterClassID.RCTPower) {
                                meter_host = meter_config[1].host;
                                meter_port = meter_config[1].port;
                            }
                        }

                        this.setState({
                            meter_slot: meter_slot,
                            meter_host: meter_host,
                            meter_port: meter_port,
                        });
                    }} />
            </FormRow>
{/*#endif*/}
            <FormRow label={__("rct_power_debug.content.host")}>
                <InputHost
                    required
                    disabled={this.state.waiting || this.state.meter_slot >= 0}
                    value={this.state.meter_slot >= 0 ? this.state.meter_host : this.state.custom_host}
                    onValue={(v) => this.state.meter_slot < 0 ? this.setState({custom_host: v}) : undefined} />
            </FormRow>
            <FormRow label={__("rct_power_debug.content.port")} label_muted={__("rct_power_debug.content.port_muted")}>
                <InputNumber
                    required
                    disabled={this.state.waiting || this.state.meter_slot >= 0}
                    min={1}
                    max={65535}
                    value={this.state.meter_slot >= 0 ? this.state.meter_port : this.state.custom_port}
                    onValue={(v) => this.state.meter_slot < 0 ? this.setState({custom_port: v}) : undefined} />
            </FormRow>
            <FormRow label={__("rct_power_debug.content.id")}>
                <InputTextPatterned
                    required
                    disabled={this.state.waiting}
                    pattern="^([1-9a-f][0-9a-f]{0,7}|0)$"
                    value={this.state.id.toString(16)}
                    onValue={(v) => this.setState({id: parseInt(v, 16)})}
                    invalidFeedback={__("rct_power_debug.content.id_invalid")} />
            </FormRow>
            <FormRow label={__("rct_power_debug.content.read_timeout")}>
                <InputNumber
                    required
                    disabled={this.state.waiting}
                    value={this.state.timeout}
                    onValue={(v) => this.setState({timeout: v})}
                    unit="ms" />
            </FormRow>
            <FormRow>
                <Button variant="primary" className="w-100" type="submit" disabled={this.state.waiting}>{__("rct_power_debug.content.execute")}</Button>
            </FormRow>

            {this.state.waiting || this.state.result.length > 0 ?
                <FormRow label={__("rct_power_debug.content.result")}>
                    <OutputTextarea rows={15} resize="vertical" value={this.state.waiting ? "Waiting..." : this.state.result} />
                </FormRow>
                : undefined}
        </form>;
    }
}

export class RCTPowerDebug extends Component {
    render() {
        if (!util.render_allowed())
            return <SubPage name="rct_power_debug" />;

        return (
            <SubPage name="rct_power_debug">
                <PageHeader title={__("rct_power_debug.content.rct_power_debug")} />

                <RCTPowerDebugTool />
            </SubPage>
        );
    }
}

export function pre_init() {
}

export function init() {
}
