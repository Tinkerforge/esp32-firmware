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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
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

export function ModbusTCPDebugNavbar() {
    return (
        <NavbarItem name="modbus_tcp" module="modbus_tcp_debug" title={__("modbus_tcp_debug.navbar.modbus_tcp")} symbol={
            <svg fill="currentColor" viewBox="0 0 24 24" width="24" height="24" xmlns="http://www.w3.org/2000/svg"><g><g stroke="currentColor"><path d="M19.587 12a7.442 7.442 0 1 1-7.442-7.441A7.441 7.441 0 0 1 19.587 12" fill="#fff" stroke-width="1.674"/><path d="M22.192 16.242a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M7.68 16.242a2.79 2.79 0 1 1-2.791-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M7.568 7.814a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M22.192 7.814a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M14.936 20.373a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M14.936 3.628a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path class="cls-3" d="M5.533 8.44 11.698 12c0-.595.893-.595.894 0l6.165-3.56-.224-.388-6.165 3.56c-.024.032 0-7.119 0-7.119-.094-.006-.446 0-.446 0s.01 7.146 0 7.12L5.755 8.054c-.067.106-.222.386-.222.386z" stroke-width=".5" fill="#000"/><path class="cls-3" d="M18.758 15.56 12.591 12c0 .595-.893.595-.893 0l-6.165 3.56.226.388s6.174-3.578 6.163-3.562v7.121s.426.002.446 0c0 0-.019-7.138 0-7.12l6.168 3.56s.208-.36.222-.387z" stroke-width=".5"/></g></g></svg>
        } />);
}

interface ModbusTCPDebugToolState {
    host: string;
    port: number;
    device_address: number;
    function_code: number;
    start_address: number;
    data_count: number;
    write_data: string;
    timeout: number;
    waiting: boolean;
    cookie: number;
    result: string;
}

function printable_ascii(x: number) {
    if (x >= 32 && x <= 126) {
        return String.fromCharCode(x);
    }

    return '.';
}

export class ModbusTCPDebugTool extends Component<{}, ModbusTCPDebugToolState> {
    constructor() {
        super();

        this.state = {
            host: "",
            port: 502,
            device_address: 1,
            function_code: 3,
            start_address: 0,
            data_count: 1,
            write_data: "",
            timeout: 2000,
            waiting: false,
            cookie: null,
            result: "",
        } as any;

        util.addApiEventListener('modbus_tcp_debug/transact_result', () => {
            let transact_result = API.get('modbus_tcp_debug/transact_result');

            if (!this.state.waiting || transact_result.cookie !== this.state.cookie) {
                return;
            }

            let result = "Success";

            if (transact_result.error !== null) {
                result = "Error: " + transact_result.error;
            }
            else if (transact_result.read_data !== null) {
                let header = " Addr  Off   Hex   UInt";

                result = header + "\n";

                for (let r = 0; r < transact_result.read_data.length / 4; ++r) {
                    let a_pad = "    " + (this.state.start_address + r);

                    a_pad = a_pad.substring(a_pad.length - 5);

                    let r_pad = "  " + r;

                    r_pad = r_pad.substring(r_pad.length - 3);

                    let hex = transact_result.read_data.substring(r * 4, r * 4 + 4);
                    let int_pad = "    " + parseInt(hex, 16);

                    int_pad = int_pad.substring(int_pad.length - 5);

                    let ascii_0 = printable_ascii(parseInt(hex.substring(0, 2), 16));
                    let ascii_1 = printable_ascii(parseInt(hex.substring(2, 4), 16));

                    result += "\n" + a_pad + "  " + r_pad + "  " + hex + "  " + int_pad + "  " + ascii_0 + ascii_1;

                    if (r % 20 == 19 && r < transact_result.read_data.length / 4 - 1) {
                        result += "\n\n" + header + "\n";
                    }
                }
            }

            this.setState({waiting: false, cookie: null, result: result});
        });
    }

    render() {
        return <form onSubmit={async (e) => {
                    e.preventDefault();
                    e.stopPropagation();

                    if (!(e.target as HTMLFormElement).checkValidity() || (e.target as HTMLFormElement).querySelector(".is-invalid")) {
                        return;
                    }

                    let cookie: number = Math.floor(Math.random() * 0xFFFFFFFF);

                    this.setState({waiting: true, cookie: cookie, result: ""}, async () => {
                        let data_count = this.state.data_count;
                        let values_hex: string[] = [];

                        if (this.state.function_code == 6 || this.state.function_code == 16) {
                            for (let value_dec of this.state.write_data.split(",")) {
                                let value = parseInt(value_dec);

                                if (value > 65535) {
                                    this.setState({waiting: false, cookie: null, result: "Error: Value too big"});
                                    return;
                                }

                                let value_hex = value.toString(16).toUpperCase();

                                while (value_hex.length < 4) {
                                    value_hex = "0" + value_hex;
                                }

                                values_hex.push(value_hex);
                            }

                            data_count = values_hex.length;
                        }

                        let result = "<unknown>";

                        try {
                            result = await (await util.put("/modbus_tcp_debug/transact", {
                                host: this.state.host,
                                port: this.state.port,
                                device_address: this.state.device_address,
                                function_code: this.state.function_code,
                                start_address: this.state.start_address,
                                data_count: data_count,
                                write_data: values_hex.join(""),
                                timeout: this.state.timeout,
                                cookie: cookie,
                            })).text();
                        }
                        catch (e) {
                            result = e.message.replace("400(Bad Request) ", "");
                        }

                        if (result.length > 0) {
                            this.setState({waiting: false, cookie: null, result: "Error: " + result});
                        }
                    });
                }}>
            <FormRow label={__("modbus_tcp_debug.content.host")}>
                <InputHost
                    required
                    value={this.state.host}
                    onValue={(v) => this.setState({host: v})} />
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.port")} label_muted={__("modbus_tcp_debug.content.port_muted")}>
                <InputNumber
                    required
                    min={1}
                    max={65535}
                    value={this.state.port}
                    onValue={(v) => this.setState({port: v})} />
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.device_address")}>
                <InputNumber
                    required
                    min={0}
                    max={255}
                    value={this.state.device_address}
                    onValue={(v) => this.setState({device_address: v})} />
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.function_code")}>
                <InputSelect
                    required
                    items={[
                        ["3", __("modbus_tcp_debug.content.function_code_read_holding_registers")],
                        ["4", __("modbus_tcp_debug.content.function_code_read_input_registers")],
                        ["6", __("modbus_tcp_debug.content.function_code_write_single_register")],
                        ["16", __("modbus_tcp_debug.content.function_code_write_multiple_registers")],
                    ]}
                    placeholder={__("select")}
                    value={this.state.function_code.toString()}
                    onValue={(v) => this.setState({function_code: parseInt(v)})} />
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.start_address")} label_muted={__("modbus_tcp_debug.content.start_address_muted")}>
                <InputNumber
                    required
                    min={0}
                    max={65535}
                    value={this.state.start_address}
                    onValue={(v) => this.setState({start_address: v})} />
            </FormRow>

            {this.state.function_code == 3 || this.state.function_code == 4 ?
                <FormRow label={__("modbus_tcp_debug.content.data_count")}>
                    <InputNumber
                        required
                        min={1}
                        max={65535 /* FIXME: depends on function code */}
                        value={this.state.data_count}
                        onValue={(v) => this.setState({data_count: v})} />
                </FormRow>
                : undefined}

            {this.state.function_code == 6 ?
                <FormRow label={__("modbus_tcp_debug.content.write_data_single_register")}>
                    <InputTextPatterned
                        required
                        pattern="^[0-9]+$"
                        value={this.state.write_data}
                        onValue={(v) => this.setState({write_data: v})}
                        invalidFeedback={__("modbus_tcp_debug.content.write_data_single_register_invalid")}
                        />
                </FormRow>
                : undefined}

            {this.state.function_code == 16 ?
                <FormRow label={__("modbus_tcp_debug.content.write_data_multiple_registers")} label_muted={__("modbus_tcp_debug.content.write_data_multiple_registers_muted")}>
                    <InputTextPatterned
                        required
                        pattern="^[0-9]+(,[0-9]+)*$"
                        value={this.state.write_data}
                        onValue={(v) => this.setState({write_data: v})}
                        invalidFeedback={__("modbus_tcp_debug.content.write_data_multiple_registers_invalid")}
                        />
                </FormRow>
                : undefined}

            <FormRow label={__("modbus_tcp_debug.content.transact_timeout")}>
                <InputNumber
                    required
                    value={this.state.timeout}
                    onValue={(v) => this.setState({timeout: v})}
                    unit="ms" />
            </FormRow>
            <FormRow label="">
                <Button variant="primary" className="form-control" type="submit">{__("modbus_tcp_debug.content.execute")}</Button>
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.response")}>
                <OutputTextarea rows={15} resize="vertical" value={this.state.waiting ? "Waiting..." : this.state.result} />
            </FormRow>
        </form>;
    }
}

export class ModbusTCPDebug extends Component {
    render() {
        if (!util.render_allowed())
            return <SubPage name="modbus_tcp" />;

        return (
            <SubPage name="modbus_tcp">
                <PageHeader title={__("modbus_tcp_debug.content.modbus_tcp")} />

                <Alert variant="warning"> {__("modbus_tcp_debug.content.server_missing")}</Alert>

                <CollapsedSection heading={__("modbus_tcp_debug.content.debug")}>
                    <ModbusTCPDebugTool />
                </CollapsedSection>
            </SubPage>
        );
    }
}

export function init() {
}
