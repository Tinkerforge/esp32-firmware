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
import { ModbusFunctionCode } from "../modbus_tcp_client/modbus_function_code.enum";
import { ModbusRegisterAddressMode } from "../modbus_tcp_client/modbus_register_address_mode.enum";

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
    function_code: number; // ModbusFunctionCode
    register_address_mode: number; // ModbusRegisterAddressMode
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

function to_value_hex(value: number) {
    let value_hex = value.toString(16).toUpperCase();

    while (value_hex.length < 4) {
        value_hex = "0" + value_hex;
    }

    return value_hex;
}

export class ModbusTCPDebugTool extends Component<{}, ModbusTCPDebugToolState> {
    constructor() {
        super();

        this.state = {
            host: "",
            port: 502,
            device_address: 1,
            function_code: 3,
            register_address_mode: ModbusRegisterAddressMode.Address,
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
                if (this.state.function_code == 1 || this.state.function_code == 2) {
                    let header = " AdrD  AdrH   NumD   NumH   Idx  V ";

                    result = header + "\n";

                    for (let i = 0, j = 0; j < transact_result.read_data.length / 2; ++j) {
                        let values = parseInt(transact_result.read_data.substring(j * 2, j * 2 + 2), 16);

                        for (let k = 0; k < 8 && i < this.state.data_count; ++k, ++i) {
                            let a = this.state.start_address + i;

                            let ad_pad = "    " + a;

                            ad_pad = ad_pad.substring(ad_pad.length - 5);

                            let ah_pad = "   " + a.toString(16).toUpperCase();

                            ah_pad = ah_pad.substring(ah_pad.length - 4);

                            let n = a + 1;

                            let nd_pad = "    " + n;

                            nd_pad = nd_pad.substring(nd_pad.length - 5);

                            let nh_pad = "    " + n.toString(16).toUpperCase();

                            nh_pad = nh_pad.substring(nh_pad.length - 5);

                            let i_pad = "   " + i;

                            i_pad = i_pad.substring(i_pad.length - 4);

                            let v = values & (1 << k) ? "1" : "0";

                            result += "\n" + ad_pad + "  " + ah_pad + "  " + nd_pad + "  " + nh_pad + "  " + i_pad + "  " + v + " ";

                            if (i % 20 == 19 && i < this.state.data_count - 1) {
                                result += "\n\n" + header + "\n";
                            }
                        }
                    }
                }
                else {
                    let header = " AdrD  AdrH   NumD   NumH  Idx   Hex  AS    U16     S16       U32BE       U32LE        S32BE        S32LE                 U64BE                 U64LE                  S64BE                  S64LE ";

                    result = header + "\n";

                    for (let i = 0; i < transact_result.read_data.length / 4; ++i) {
                        let a = this.state.start_address + i;

                        let ad_pad = "    " + a;

                        ad_pad = ad_pad.substring(ad_pad.length - 5);

                        let ah_pad = "   " + a.toString(16).toUpperCase();

                        ah_pad = ah_pad.substring(ah_pad.length - 4);

                        let n = a + 1;

                        let nd_pad = "    " + n;

                        nd_pad = nd_pad.substring(nd_pad.length - 5);

                        let nh_pad = "    " + n.toString(16).toUpperCase();

                        nh_pad = nh_pad.substring(nh_pad.length - 5);

                        let i_pad = "  " + i;

                        i_pad = i_pad.substring(i_pad.length - 3);

                        let hex = transact_result.read_data.substring(i * 4, i * 4 + 4);

                        let ascii_0 = printable_ascii(parseInt(hex.substring(0, 2), 16));
                        let ascii_1 = printable_ascii(parseInt(hex.substring(2, 4), 16));

                        let u16 = parseInt(hex, 16);
                        let u16_pad = "    " + u16;

                        u16_pad = u16_pad.substring(u16_pad.length - 5);

                        let s16 = (u16 << 16) >> 16;
                        let s16_pad = "     " + s16;

                        s16_pad = s16_pad.substring(s16_pad.length - 6);

                        let u32be_pad = "          ";
                        let u32le_pad = "          ";
                        let s32be_pad = "           ";
                        let s32le_pad = "           ";

                        if ((i % 2) == 1) {
                            let u32be = parseInt(transact_result.read_data.substring((i - 1) * 4,     (i - 1) * 4 + 8), 16);
                            let u32le = parseInt(transact_result.read_data.substring((i - 1) * 4 + 4, (i - 1) * 4 + 8) +
                                                 transact_result.read_data.substring((i - 1) * 4,     (i - 1) * 4 + 4), 16);
                            let s32be = u32be >> 0;
                            let s32le = u32le >> 0;

                            u32be_pad += u32be;
                            u32le_pad += u32le;
                            s32be_pad += s32be;
                            s32le_pad += s32le;
                        }

                        u32be_pad = u32be_pad.substring(u32be_pad.length - 10);
                        u32le_pad = u32le_pad.substring(u32le_pad.length - 10);
                        s32be_pad = s32be_pad.substring(s32be_pad.length - 11);
                        s32le_pad = s32le_pad.substring(s32le_pad.length - 11);

                        let u64be_pad = "                    ";
                        let u64le_pad = "                    ";
                        let s64be_pad = "                     ";
                        let s64le_pad = "                     ";

                        if ((i % 4) == 3) {
                            let u64be = BigInt("0x" + transact_result.read_data.substring((i - 3) * 4,      (i - 3) * 4 + 16));
                            let u64le = BigInt("0x" + transact_result.read_data.substring((i - 3) * 4 + 12, (i - 3) * 4 + 16) +
                                                      transact_result.read_data.substring((i - 3) * 4 + 8,  (i - 3) * 4 + 12) +
                                                      transact_result.read_data.substring((i - 3) * 4 + 4,  (i - 3) * 4 + 8) +
                                                      transact_result.read_data.substring((i - 3) * 4,      (i - 3) * 4 + 4));
                            let s64be = BigInt.asIntN(64, u64be);
                            let s64le = BigInt.asIntN(64, u64le);

                            u64be_pad += u64be.toString();
                            u64le_pad += u64le.toString();
                            s64be_pad += s64be.toString();
                            s64le_pad += s64le.toString();
                        }

                        u64be_pad = u64be_pad.substring(u64be_pad.length - 20);
                        u64le_pad = u64le_pad.substring(u64le_pad.length - 20);
                        s64be_pad = s64be_pad.substring(s64be_pad.length - 21);
                        s64le_pad = s64le_pad.substring(s64le_pad.length - 21);

                        result += "\n" + ad_pad + "  " + ah_pad + "  " + nd_pad + "  " + nh_pad + "  " + i_pad + "  " + hex + "  " + ascii_0 + ascii_1 + "  " + u16_pad + "  " + s16_pad + "  " + u32be_pad + "  " + u32le_pad + "  " + s32be_pad + "  " + s32le_pad + "  " + u64be_pad + "  " + u64le_pad + "  " + s64be_pad + "  " + s64le_pad + " ";

                        if (i % 20 == 19 && i < transact_result.read_data.length / 4 - 1) {
                            result += "\n\n" + header + "\n";
                        }
                    }
                }
            }

            this.setState({waiting: false, cookie: null, result: result});
        });
    }

    render() {
        let start_address_offset = this.state.register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1;

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

                        if (this.state.function_code == ModbusFunctionCode.WriteSingleRegister
                         || this.state.function_code == ModbusFunctionCode.WriteMultipleRegisters) {
                            let max_values = 0;

                            if (this.state.function_code == ModbusFunctionCode.WriteSingleRegister) {
                                max_values = 1;
                            }
                            else if (this.state.function_code == ModbusFunctionCode.WriteMultipleRegisters) {
                                max_values = 123;
                            }

                            let values_dec = this.state.write_data.split(",");

                            if (values_dec.length > max_values) {
                                this.setState({waiting: false, cookie: null, result: "Error: Too many values"});
                                return;
                            }

                            for (let value_dec of values_dec) {
                                value_dec = value_dec.trim();

                                let value = parseInt(value_dec, 10);

                                if (isNaN(value)) {
                                    this.setState({waiting: false, cookie: null, result: "Error: Value is invalid"});
                                    return;
                                }

                                if (value > 65535) {
                                    this.setState({waiting: false, cookie: null, result: "Error: Value is too big"});
                                    return;
                                }

                                if ("" + value !== value_dec) {
                                    this.setState({waiting: false, cookie: null, result: "Error: Value is malformed"});
                                    return;
                                }

                                values_hex.push(to_value_hex(value));
                            }

                            data_count = values_hex.length;
                        }
                        else if (this.state.function_code == ModbusFunctionCode.MaskWriteRegister) {
                            let masks = this.state.write_data.split(",");

                            if (masks.length > 1) {
                                this.setState({waiting: false, cookie: null, result: "Error: Too many bitmasks"});
                                return;
                            }

                            for (let mask of masks) {
                                mask = mask.trim().toUpperCase();

                                if (mask.length > 16) {
                                    this.setState({waiting: false, cookie: null, result: "Error: Bitmask is too long"});
                                    return;
                                }

                                let mask_pad = "XXXXXXXXXXXXXXX" + mask;

                                mask_pad = mask_pad.substring(mask_pad.length - 16);

                                let and_mask = 0x0000;
                                let or_mask = 0x0000;

                                for (let i = 0; i < 16; ++i) {
                                    let bit = mask_pad[15 - i];

                                    if (bit == '1') {
                                        or_mask |= 1 << i;
                                    }
                                    else if (bit == '0') {
                                        // masks already correct
                                    }
                                    else if (bit == 'X') {
                                        and_mask |= 1 << i;
                                    }
                                    else {
                                        this.setState({waiting: false, cookie: null, result: "Error: Bitmask is malformed"});
                                        return;
                                    }
                                }

                                values_hex.push(to_value_hex(and_mask));
                                values_hex.push(to_value_hex(or_mask));
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
                    disabled={this.state.waiting}
                    value={this.state.host}
                    onValue={(v) => this.setState({host: v})} />
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.port")} label_muted={__("modbus_tcp_debug.content.port_muted")}>
                <InputNumber
                    required
                    disabled={this.state.waiting}
                    min={1}
                    max={65535}
                    value={this.state.port}
                    onValue={(v) => this.setState({port: v})} />
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.device_address")}>
                <InputNumber
                    required
                    disabled={this.state.waiting}
                    min={0}
                    max={255}
                    value={this.state.device_address}
                    onValue={(v) => this.setState({device_address: v})} />
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.function_code")}>
                <InputSelect
                    required
                    disabled={this.state.waiting}
                    items={[
                        [ModbusFunctionCode.ReadCoils.toString(), __("modbus_tcp_debug.content.function_code_read_coils")],
                        [ModbusFunctionCode.ReadDiscreteInputs.toString(), __("modbus_tcp_debug.content.function_code_read_discrete_inputs")],
                        [ModbusFunctionCode.ReadHoldingRegisters.toString(), __("modbus_tcp_debug.content.function_code_read_holding_registers")],
                        [ModbusFunctionCode.ReadInputRegisters.toString(), __("modbus_tcp_debug.content.function_code_read_input_registers")],
                        [ModbusFunctionCode.WriteSingleRegister.toString(), __("modbus_tcp_debug.content.function_code_write_single_register")],
                        [ModbusFunctionCode.WriteMultipleRegisters.toString(), __("modbus_tcp_debug.content.function_code_write_multiple_registers")],
                        [ModbusFunctionCode.MaskWriteRegister.toString(), __("modbus_tcp_debug.content.function_code_mask_write_register")],
                    ]}
                    placeholder={__("select")}
                    value={this.state.function_code.toString()}
                    onValue={(v) => this.setState({function_code: parseInt(v)})} />
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.register_address_mode")}>
                <InputSelect
                    required
                    disabled={this.state.waiting}
                    items={[
                        [ModbusRegisterAddressMode.Address.toString(), __("modbus_tcp_debug.content.register_address_mode_address")],
                        [ModbusRegisterAddressMode.Number.toString(), __("modbus_tcp_debug.content.register_address_mode_number")]
                    ]}
                    placeholder={__("select")}
                    value={this.state.register_address_mode.toString()}
                    onValue={(v) => this.setState({register_address_mode: parseInt(v)})} />
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.start_address_dec")}>
                <InputNumber
                    required
                    disabled={this.state.waiting}
                    min={start_address_offset}
                    max={start_address_offset + 65535}
                    value={this.state.start_address + start_address_offset}
                    onValue={(v) => {
                        let start_address = Math.max(0, Math.min(v - start_address_offset, 65535));
                        let data_count = Math.min(start_address + this.state.data_count, 65535 + 1) - start_address;

                        this.setState({start_address: start_address, data_count: data_count});
                    }} />
            </FormRow>
            <FormRow label={__("modbus_tcp_debug.content.start_address_hex")}>
                <InputTextPatterned
                    required
                    disabled={this.state.waiting}
                    pattern={this.state.register_address_mode == ModbusRegisterAddressMode.Address ? "^([1-9A-F][0-9A-F]{1,3}|0)$" : "^(10000|[1-9A-F][0-9A-F]{1,3}|1)$"}
                    value={util.hasValue(this.state.start_address) ? (this.state.start_address + start_address_offset).toString(16).toUpperCase() : null}
                    onValue={(v) => {
                        let start_address = parseInt(v, 16) - start_address_offset;
                        let data_count = this.state.data_count;

                        if (isNaN(start_address)) {
                            start_address = null;
                        }
                        else {
                            start_address = Math.max(0, Math.min(start_address, 65535));
                            data_count = Math.min(start_address + data_count, 65535 + 1) - start_address;
                        }

                        this.setState({start_address: start_address, data_count: data_count});
                    }}
                    invalidFeedback={__("modbus_tcp_debug.content.start_address_hex_invalid")} />
            </FormRow>

            {this.state.function_code == ModbusFunctionCode.ReadCoils
          || this.state.function_code == ModbusFunctionCode.ReadDiscreteInputs
          || this.state.function_code == ModbusFunctionCode.ReadHoldingRegisters
          || this.state.function_code == ModbusFunctionCode.ReadInputRegisters ?
                <FormRow label={__("modbus_tcp_debug.content.data_count")}>
                    <InputNumber
                        required
                        disabled={this.state.waiting}
                        min={1}
                        max={Math.min(this.state.start_address + (this.state.function_code == 1 || this.state.function_code == 2 ? 2000 : 125), 65535 + 1) - this.state.start_address}
                        value={this.state.data_count}
                        onValue={(v) => this.setState({data_count: Math.min(this.state.start_address + v, 65535 + 1) - this.state.start_address})} />
                </FormRow>
                : undefined}

            {this.state.function_code == ModbusFunctionCode.WriteSingleRegister ?
                <FormRow label={__("modbus_tcp_debug.content.write_data_single_value")}>
                    <InputTextPatterned
                        required
                        disabled={this.state.waiting}
                        pattern={`^ *${util.UINT16_PATTERN} *$`}
                        value={this.state.write_data}
                        onValue={(v) => this.setState({write_data: v})}
                        invalidFeedback={__("modbus_tcp_debug.content.write_data_single_value_invalid")}
                        />
                </FormRow>
                : undefined}

            {this.state.function_code == ModbusFunctionCode.WriteMultipleRegisters ?
                <FormRow label={__("modbus_tcp_debug.content.write_data_multiple_values")} label_muted={__("modbus_tcp_debug.content.write_data_multiple_values_muted")}>
                    <InputTextPatterned
                        required
                        disabled={this.state.waiting}
                        pattern={`^ *${util.UINT16_PATTERN} *(, *${util.UINT16_PATTERN} *){0,122}$`}
                        value={this.state.write_data}
                        onValue={(v) => this.setState({write_data: v})}
                        invalidFeedback={__("modbus_tcp_debug.content.write_data_multiple_values_invalid")}
                        />
                </FormRow>
                : undefined}

            {this.state.function_code == ModbusFunctionCode.MaskWriteRegister ?
                <FormRow label={__("modbus_tcp_debug.content.write_data_mask")}>
                    <InputTextPatterned
                        required
                        disabled={this.state.waiting}
                        pattern={`^ *[01Xx]{1,16} *$`}
                        value={this.state.write_data}
                        onValue={(v) => this.setState({write_data: v.toUpperCase()})}
                        invalidFeedback={__("modbus_tcp_debug.content.write_data_mask_invalid")}
                        />
                </FormRow>
                : undefined}

            <FormRow label={__("modbus_tcp_debug.content.transact_timeout")}>
                <InputNumber
                    required
                    disabled={this.state.waiting}
                    value={this.state.timeout}
                    onValue={(v) => this.setState({timeout: v})}
                    unit="ms" />
            </FormRow>
            <FormRow label="">
                <Button variant="primary" className="form-control" type="submit" disabled={this.state.waiting}>{__("modbus_tcp_debug.content.execute")}</Button>
            </FormRow>

            {this.state.waiting || this.state.result.length > 0 ?
                <FormRow label={__("modbus_tcp_debug.content.response")}>
                    <OutputTextarea rows={15} resize="vertical" value={this.state.waiting ? "Waiting..." : this.state.result} />
                </FormRow>
                : undefined}
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
