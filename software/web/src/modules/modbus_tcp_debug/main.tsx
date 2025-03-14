/* esp32-firmware
 * Copyright (C) 2020-2023 Erik Fleckstein <erik@tinkerforge.com>
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
import { Button } from "react-bootstrap";
import { FormRow } from "../../ts/components/form_row";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { InputHost } from "../../ts/components/input_host";
import { OutputTextarea } from "../../ts/components/output_textarea";
import { NavbarItem } from "../../ts/components/navbar_item";
import { PageHeader } from "../../ts/components/page_header";
import { SubPage } from "../../ts/components/sub_page";
import { Terminal } from "react-feather";

export function ModbusTCPDebugNavbar() {
    return <NavbarItem name="modbus_tcp_debug" module="modbus_tcp_debug" title={__("modbus_tcp_debug.navbar.modbus_tcp_debug")} symbol={<Terminal />} />;
}

interface ModbusTCPDebugState {
    host: string;
    port: number;
    device_address: number;
    function_code: number;
    start_address: number;
    data_count: number;
    write_data: string;
    timeout: number;
    byte_order: number;
    response: string;
}

function printable_ascii(x: number) {
    if (x >= 32 && x <= 126) {
        return String.fromCharCode(x);
    }

    return '.';
}

export class ModbusTCPDebug extends Component<{}, ModbusTCPDebugState> {
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
            byte_order: 0,
            response: "",
        } as any;
    }

    render() {
        if (!util.render_allowed())
            return <SubPage name="modbus_tcp_debug" />;

        return (
            <SubPage name="modbus_tcp_debug">
                <PageHeader title={__("modbus_tcp_debug.content.modbus_tcp_debug")} />

                <FormRow label={__("modbus_tcp_debug.content.host")}>
                    <InputHost
                        value={this.state.host}
                        onValue={(v) => this.setState({host: v})}
                        required />
                </FormRow>
                <FormRow label={__("modbus_tcp_debug.content.port")} label_muted={__("modbus_tcp_debug.content.port_muted")}>
                    <InputNumber
                        min={1}
                        max={65535}
                        value={this.state.port}
                        onValue={(v) => this.setState({port: v})} />
                </FormRow>
                <FormRow label={__("modbus_tcp_debug.content.device_address")}>
                    <InputNumber
                        min={0}
                        max={255}
                        value={this.state.device_address}
                        onValue={(v) => this.setState({device_address: v})} />
                </FormRow>
                <FormRow label={__("modbus_tcp_debug.content.function_code")}>
                    <InputSelect
                        items={[
                            ["3", __("modbus_tcp_debug.content.function_code_read_holding_registers")],
                            ["4", __("modbus_tcp_debug.content.function_code_read_input_registers")],
                        ]}
                        placeholder={__("select")}
                        value={this.state.function_code.toString()}
                        onValue={(v) => this.setState({function_code: parseInt(v)})} />
                </FormRow>
                <FormRow label={__("modbus_tcp_debug.content.start_address")} label_muted={__("modbus_tcp_debug.content.start_address_muted")}>
                    <InputNumber
                        min={0}
                        max={65535}
                        value={this.state.start_address}
                        onValue={(v) => this.setState({start_address: v})} />
                </FormRow>
                <FormRow label={__("modbus_tcp_debug.content.data_count")}>
                    <InputNumber
                        min={1}
                        max={65535 /* FIXME: depends on function code */}
                        value={this.state.data_count}
                        onValue={(v) => this.setState({data_count: v})} />
                </FormRow>
                <FormRow label="">
                    <Button variant="primary" className="form-control" onClick={async () => {
                        this.setState({response: "Waiting..."});

                        let response;

                        try {
                            response = await (await util.put('modbus_tcp_debug/transact', {
                                host: this.state.host,
                                port: this.state.port,
                                device_address: this.state.device_address,
                                function_code: this.state.function_code,
                                start_address: this.state.start_address,
                                data_count: this.state.data_count,
                                write_data: this.state.write_data,
                                timeout: this.state.timeout,
                                byte_order: this.state.byte_order})).text();
                        } catch (e) {
                            this.setState({response: "" + e});
                            console.log('Modbus/TCP Debug: Could not transact: ' + e);
                            return;
                        }

                        let output = response;

                        if (response.startsWith("READDATA:")) {
                            let readdata = response.substring(9);
                            let header = " Addr  Off   Hex   UInt";

                            output = header + "\n";

                            for (let r = 0; r < readdata.length / 4; ++r) {
                                let a_pad = "    " + (this.state.start_address + r);

                                a_pad = a_pad.substring(a_pad.length - 5);

                                let r_pad = "  " + r;

                                r_pad = r_pad.substring(r_pad.length - 3);

                                let hex = readdata.substring(r * 4, r * 4 + 4);
                                let int_pad = "    " + parseInt(hex, 16);

                                int_pad = int_pad.substring(int_pad.length - 5);

                                let ascii_0 = printable_ascii(parseInt(hex.substring(0, 2), 16));
                                let ascii_1 = printable_ascii(parseInt(hex.substring(2, 4), 16));

                                output += "\n" + a_pad + "  " + r_pad + "  " + hex + "  " + int_pad + "  " + ascii_0 + ascii_1;

                                if (r % 20 == 19 && r < readdata.length / 4 - 1) {
                                    output += "\n\n" + header + "\n";
                                }
                            }
                        }

                        this.setState({response: output});
                    }} >{__("modbus_tcp_debug.content.execute")}</Button>
                </FormRow>
                <FormRow label={__("modbus_tcp_debug.content.response")}>
                    <OutputTextarea rows={35} resize='vertical' value={this.state.response} />
                </FormRow>
            </SubPage>
        );
    }
}

export function init() {
}
