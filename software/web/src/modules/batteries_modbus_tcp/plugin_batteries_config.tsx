/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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
import * as options from "../../options";
import { h, Fragment, Component, ComponentChildren } from "preact";
import { Button, Dropdown } from "react-bootstrap";
import { __ } from "../../ts/translation";
import { BatteryClassID } from "../batteries/battery_class_id.enum";
import { BatteryConfig } from "../batteries/types";
import { BatteryMode } from "../batteries/battery_mode.enum";
import { BatteryModbusTCPTableID } from "./battery_modbus_tcp_table_id.enum";
import { TableConfigCustom, TableConfig, RegisterTable, RegisterBlock, get_default_device_address, new_table_config, import_table_config } from "./battery_modbus_tcp_specs";
import { ModbusFunctionCode } from "../modbus_tcp_client/modbus_function_code.enum";
import { ModbusRegisterAddressMode } from "../modbus_tcp_client/modbus_register_address_mode.enum";
import { InputText, InputTextPatterned } from "../../ts/components/input_text";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { InputNumber } from "../../ts/components/input_number";
import { InputFloat } from "../../ts/components/input_float";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { OutputTextarea } from "../../ts/components/output_textarea";
import { Table, TableRow } from "../../ts/components/table";

export type ModbusTCPBatteriesConfig = [
    BatteryClassID.ModbusTCP,
    {
        display_name: string;
        host: string;
        port: number;
        table: TableConfig;
    },
];

interface RegisterEditorProps {
    register_address_mode: ModbusRegisterAddressMode;
    other_total_values_count: number;
    table: RegisterTable;
    on_table: (table: RegisterTable) => void;
}

interface RegisterEditorState {
    register_block: RegisterBlock,
    values: string,
}

function get_total_values_count(table: RegisterTable) {
    let total_values_count = 0;

    for (let register of table.register_blocks) {
        total_values_count += register.vals.length;
    }

    return total_values_count;
}

class RegisterEditor extends Component<RegisterEditorProps, RegisterEditorState> {
    constructor(props: RegisterEditorProps) {
        super(props);

        this.state = {
            register_block: {
                desc: "",
                func: null,
                addr: null,
                vals: [],
            },
            values: "",
        } as RegisterEditorState;
    }

    get_children() {
        let start_address_offset = this.props.register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1;
        let values_pattern = "";
        let values_label = undefined;
        let values_label_muted = undefined;
        let values_invalid = "";
        let values_filter = (v: string) => v;

        switch (this.state.register_block.func) {
        case ModbusFunctionCode.WriteSingleCoil:
            values_pattern = "^ *[01] *$";
            values_label = __("batteries_modbus_tcp.content.register_blocks_value");
            values_label_muted = __("batteries_modbus_tcp.content.register_blocks_value_muted");
            values_invalid = __("batteries_modbus_tcp.content.register_blocks_value_invalid");
            break;

        case ModbusFunctionCode.WriteSingleRegister:
            values_pattern = `^ *${util.UINT16_PATTERN} *$`;
            values_label = __("batteries_modbus_tcp.content.register_blocks_value");
            values_label_muted = __("batteries_modbus_tcp.content.register_blocks_value_muted");
            values_invalid = __("batteries_modbus_tcp.content.register_blocks_value_invalid");
            break;

        case ModbusFunctionCode.WriteMultipleCoils:
            values_pattern = `^ *[01] *(, *[01] *){0,${options.BATTERIES_MODBUS_TCP_MAX_CUSTOM_VALUES_PER_REGISTER_BLOCK - 1}}$`;
            values_label = __("batteries_modbus_tcp.content.register_blocks_values");
            values_label_muted = __("batteries_modbus_tcp.content.register_blocks_values_muted");
            values_invalid = __("batteries_modbus_tcp.content.register_blocks_values_invalid");
            break;

        case ModbusFunctionCode.WriteMultipleRegisters:
            values_pattern = `^ *${util.UINT16_PATTERN} *(?:, *${util.UINT16_PATTERN} *){0,${options.BATTERIES_MODBUS_TCP_MAX_CUSTOM_VALUES_PER_REGISTER_BLOCK - 1}}$`;
            values_label = __("batteries_modbus_tcp.content.register_blocks_values");
            values_label_muted = __("batteries_modbus_tcp.content.register_blocks_values_muted");
            values_invalid = __("batteries_modbus_tcp.content.register_blocks_values_invalid");
            break;

        case ModbusFunctionCode.MaskWriteRegister:
        case ModbusFunctionCode.ReadMaskWriteSingleRegister:
            values_pattern = "^ *[01x]{1,16} *$";
            values_label = __("batteries_modbus_tcp.content.register_blocks_mask");
            values_label_muted = __("batteries_modbus_tcp.content.register_blocks_mask_muted");
            values_invalid = __("batteries_modbus_tcp.content.register_blocks_mask_invalid");
            values_filter = (v: string) => v.toLowerCase();
            break;

        case ModbusFunctionCode.ReadMaskWriteMultipleRegisters:
            values_pattern = `^ *[01x]{1,16} *(?:, *[01x]{1,16} *){0,${options.BATTERIES_MODBUS_TCP_MAX_CUSTOM_VALUES_PER_REGISTER_BLOCK / 2 - 1}}$`;
            values_label = __("batteries_modbus_tcp.content.register_blocks_masks");
            values_label_muted = __("batteries_modbus_tcp.content.register_blocks_masks_muted");
            values_invalid = __("batteries_modbus_tcp.content.register_blocks_masks_invalid");
            values_filter = (v: string) => v.toLowerCase();
            break;
        }

        let children = [
            <FormRow label={__("batteries_modbus_tcp.content.register_blocks_desc")} label_muted={__("batteries_modbus_tcp.content.register_blocks_desc_muted")}>
                <InputText
                    maxLength={32}
                    value={this.state.register_block.desc}
                    onValue={(v) => {
                        this.setState({register_block: {...this.state.register_block, desc: v}});
                    }} />
            </FormRow>,
            <FormRow label={__("batteries_modbus_tcp.content.register_blocks_function_code")}>
                <InputSelect
                    required
                    items={[
                        [ModbusFunctionCode.WriteSingleCoil.toString(), __("batteries_modbus_tcp.content.register_blocks_function_code_write_single_coil")],
                        [ModbusFunctionCode.WriteSingleRegister.toString(), __("batteries_modbus_tcp.content.register_blocks_function_code_write_single_register")],
                        [ModbusFunctionCode.WriteMultipleCoils.toString(), __("batteries_modbus_tcp.content.register_blocks_function_code_write_multiple_coils")],
                        [ModbusFunctionCode.WriteMultipleRegisters.toString(), __("batteries_modbus_tcp.content.register_blocks_function_code_write_multiple_registers")],
                        [ModbusFunctionCode.MaskWriteRegister.toString(), __("batteries_modbus_tcp.content.register_blocks_function_code_mask_write_register")],
                        [ModbusFunctionCode.ReadMaskWriteSingleRegister.toString(), __("batteries_modbus_tcp.content.register_blocks_function_code_read_mask_write_single_register")],
                        [ModbusFunctionCode.ReadMaskWriteMultipleRegisters.toString(), __("batteries_modbus_tcp.content.register_blocks_function_code_read_mask_write_multiple_registers")],
                    ]}
                    placeholder={__("select")}
                    value={util.hasValue(this.state.register_block.func) ? this.state.register_block.func.toString() : undefined}
                    onValue={(v) => {
                        this.setState({register_block: {...this.state.register_block, func: parseInt(v)}});
                    }} />
            </FormRow>,
            <FormRow
                label={
                    this.props.register_address_mode == ModbusRegisterAddressMode.Address
                    ? __("batteries_modbus_tcp.content.register_blocks_start_address")
                    : __("batteries_modbus_tcp.content.register_blocks_start_number")
                }
                label_muted={
                    this.props.register_address_mode == ModbusRegisterAddressMode.Address
                    ? __("batteries_modbus_tcp.content.register_blocks_start_address_muted")
                    : __("batteries_modbus_tcp.content.register_blocks_start_number_muted")
                }>
                <InputNumber
                    required
                    min={start_address_offset}
                    max={start_address_offset + 65535}
                    value={this.state.register_block.addr + start_address_offset}
                    onValue={(v) => {
                        this.setState({register_block: {...this.state.register_block, addr: v - start_address_offset}});
                    }} />
            </FormRow>
        ];

        if (util.hasValue(values_label)) {
            children.push(
                <FormRow label={values_label} label_muted={values_label_muted}>
                    <InputTextPatterned
                        required
                        pattern={values_pattern}
                        value={this.state.values}
                        onValue={(v) => {
                            this.setState({values: values_filter(v)});
                        }}
                        invalidFeedback={values_invalid} />
                </FormRow>);
        }

        return children;
    }

    format_vals(function_code: number, vals: number[]) {
        let values = "";

        if (function_code == ModbusFunctionCode.WriteSingleCoil
         || function_code == ModbusFunctionCode.WriteSingleRegister
         || function_code == ModbusFunctionCode.WriteMultipleCoils
         || function_code == ModbusFunctionCode.WriteMultipleRegisters) {
            values = vals.join(", ");
        }
        else if (function_code == ModbusFunctionCode.MaskWriteRegister
              || function_code == ModbusFunctionCode.ReadMaskWriteSingleRegister
              || function_code == ModbusFunctionCode.ReadMaskWriteMultipleRegisters) {
            let and_mask = vals[0];
            let or_mask = vals[1];

            for (let i = 15; i >= 0; --i) {
                if ((and_mask & (1 << i)) != 0) {
                    values += "x";
                }
                else if ((or_mask & (1 << i)) != 0) {
                    values += "1";
                }
                else {
                    values += "0";
                }
            }

            while (values.length > 1 && values[0] == "x") {
                values = values.substring(1);
            }
        }

        return values;
    }

    parse_values(function_code: number, values_str: string) {
        let vals: number[] = [];

        if (function_code == ModbusFunctionCode.WriteSingleCoil
         || function_code == ModbusFunctionCode.WriteSingleRegister
         || function_code == ModbusFunctionCode.WriteMultipleCoils
         || function_code == ModbusFunctionCode.WriteMultipleRegisters) {
            let max_values = 0;
            let mac_value = 0;

            if (function_code == ModbusFunctionCode.WriteSingleCoil) {
                max_values = 1;
                mac_value = 1;
            }
            else if (function_code == ModbusFunctionCode.WriteSingleRegister) {
                max_values = 1;
                mac_value = 65535;
            }
            else if (function_code == ModbusFunctionCode.WriteMultipleCoils) {
                max_values = 1968;
                mac_value = 1;
            }
            else if (function_code == ModbusFunctionCode.WriteMultipleRegisters) {
                max_values = 123;
                mac_value = 65535;
            }

            let values_dec = values_str.split(",");

            if (values_dec.length > max_values) {
                return [];
            }

            for (let value_dec of values_dec) {
                value_dec = value_dec.trim();

                let value = parseInt(value_dec, 10);

                if (isNaN(value)) {
                    return [];
                }

                if (value > mac_value) {
                    return [];
                }

                if ("" + value !== value_dec) {
                    return [];
                }

                vals.push(value);
            }
        }
        else if (function_code == ModbusFunctionCode.MaskWriteRegister
              || function_code == ModbusFunctionCode.ReadMaskWriteSingleRegister
              || function_code == ModbusFunctionCode.ReadMaskWriteMultipleRegisters) {
            let max_masks = 0;

            if (function_code == ModbusFunctionCode.MaskWriteRegister
             || function_code == ModbusFunctionCode.ReadMaskWriteSingleRegister) {
                max_masks = 1;
            }
            else if (function_code == ModbusFunctionCode.ReadMaskWriteMultipleRegisters) {
                max_masks = 123;
            }

            let masks = values_str.split(",");

            if (masks.length > max_masks) {
                return [];
            }

            for (let mask of masks) {
                mask = mask.trim().toLowerCase();

                if (mask.length > 16) {
                    return [];
                }

                mask = "xxxxxxxxxxxxxxx" + mask;
                mask = mask.substring(mask.length - 16);

                let and_mask = 0x0000;
                let or_mask = 0x0000;

                for (let i = 0; i < 16; ++i) {
                    let bit = mask[15 - i];

                    if (bit == '1') {
                        or_mask |= 1 << i;
                    }
                    else if (bit == '0') {
                        // masks already correct
                    }
                    else if (bit == 'x') {
                        and_mask |= 1 << i;
                    }
                    else {
                        return [];
                    }
                }

                vals.push(and_mask);
                vals.push(or_mask);
            }
        }

        return vals;
    }

    render() {
        let start_address_offset = this.props.register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1;
        let total_values_count = this.props.other_total_values_count + get_total_values_count(this.props.table);

        return <Table
            nestingDepth={1}
            rows={this.props.table.register_blocks.map((register_block, i) => {
                const row: TableRow = {
                    columnValues: [<>{register_block.desc.length > 0 ? <div>{register_block.desc}</div> : undefined}<div style={register_block.desc.length > 0 ? "font-size: 80%" : ""}>{__("batteries_modbus_tcp.content.register_blocks_values_desc")(register_block.func, register_block.addr + start_address_offset, this.format_vals(register_block.func, register_block.vals))}</div></>],
                    onRemoveClick: async () => {
                        this.props.on_table({...this.props.table, register_blocks: this.props.table.register_blocks.filter((r, k) => k !== i)});
                        return true;
                    },
                    onEditShow: async () => {
                        this.setState({
                            register_block: register_block,
                            values: this.format_vals(register_block.func, register_block.vals),
                        });
                    },
                    onEditSubmit: async () => {
                        this.props.on_table({...this.props.table, register_blocks: this.props.table.register_blocks.map((r, k) => k === i ? {...this.state.register_block, vals: this.parse_values(this.state.register_block.func, this.state.values)} : r)});
                    },
                    onEditGetChildren: () => this.get_children(),
                    editTitle: __("batteries_modbus_tcp.content.register_blocks_edit_title"),
                }
                return row
            })}
            columnNames={[""]}
            addEnabled={util.hasValue(this.props.register_address_mode)
                     && this.props.table.register_blocks.length < options.BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS
                     && total_values_count < options.BATTERIES_MODBUS_TCP_MAX_CUSTOM_TOTAL_VALUES}
            addMessage={
                util.hasValue(this.props.register_address_mode)
                    ? <>
                        <div>{__("batteries_modbus_tcp.content.register_blocks_add_message_register_blocks")(this.props.table.register_blocks.length, options.BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS)}</div>
                        <div style="font-size: 80%">{__("batteries_modbus_tcp.content.register_blocks_add_message_total_values")(total_values_count, options.BATTERIES_MODBUS_TCP_MAX_CUSTOM_TOTAL_VALUES)}</div>
                    </>
                    : __("batteries_modbus_tcp.content.register_blocks_add_select_address_mode")
            }
            addTitle={__("batteries_modbus_tcp.content.register_blocks_add_title")}
            onAddShow={async () => {
                this.setState({
                    register_block: {
                        desc: "",
                        func: null,
                        addr: null,
                        vals: [],
                    },
                    values: "",
                });
            }}
            onAddGetChildren={() => this.get_children()}
            onAddSubmit={async () => {
                this.props.on_table({...this.props.table, register_blocks: this.props.table.register_blocks.concat([{...this.state.register_block, vals: this.parse_values(this.state.register_block.func, this.state.values)}])});
            }}/>;
    }
}

const TEST_CONTINUE_INTERVAL = 3000; // milliseconds
const TEST_LOG_INTERVAL = 250; // milliseconds

interface TestRunnerProps {
    slot: number;
    host: string;
    port: number;
    table_id: number;
    device_address: number;
    repeat_interval?: number;
    register_blocks?: RegisterBlock[];
    modes?: [number, string][];
    extra_values?: {[key: string]: number};
}

interface TestRunnerState {
    table_id: number;
    mode: [number, string];
    running: boolean;
    cookie: number;
    stopped: boolean;
    log: string;
    show_log: boolean;
}

class TestRunner extends Component<TestRunnerProps, TestRunnerState> {
    continue_timer: number = undefined;
    log_timer: number = undefined;
    pending_log: string = '';

    constructor() {
        super();

        this.state = {
            table_id: null,
            mode: null,
            running: false,
            cookie: null,
            stopped: false,
            log: '',
            show_log: false,
        } as any;

        util.addApiEventListener('batteries_modbus_tcp/test_log', () => {
            let test_log = API.get('batteries_modbus_tcp/test_log');

            if (!this.state.running || test_log.cookie !== this.state.cookie) {
                return;
            }

            this.pending_log += test_log.message;

            if (this.state.log.length == 0) {
                this.update_log();
            }
        });

        util.addApiEventListener('batteries_modbus_tcp/test_done', () => {
            let test_done = API.get('batteries_modbus_tcp/test_done');

            if (!this.state.running || test_done.cookie !== this.state.cookie) {
                return;
            }

            if (this.continue_timer !== undefined) {
                clearInterval(this.continue_timer);
                this.continue_timer = undefined;
            }

            if (this.log_timer !== undefined) {
                clearInterval(this.log_timer);
                this.log_timer = undefined;
            }

            this.update_log();
            this.setState({running: false, cookie: null});
        });
    }

    update_log(message?: string) {
        let log = this.state.log + this.pending_log;

        this.pending_log = '';

        if (message) {
            log += message;
        }

        this.setState({log: log});
    }

    async stop_test() {
        if (!this.state.running || this.state.stopped) {
            return;
        }

        let result;

        try {
            result = await (await util.put('/batteries_modbus_tcp/test_stop', {cookie: this.state.cookie})).text();
        }
        catch (e) {
            result = e.message.replace('400(Bad Request) ', '');
        }

        if (result.length > 0) {
            this.update_log("Error while stopping test: " + result + "\n");
        }
        else {
            this.setState({stopped: true});
        }
    }

    override async componentWillUnmount() {
        await this.stop_test();
    }

    async test_continue() {
        if (!this.state.running) {
            return;
        }

        let result;

        try {
            result = await (await util.put('/batteries_modbus_tcp/test_continue', {cookie: this.state.cookie})).text();
        }
        catch (e) {
            result = e.message.replace('400(Bad Request) ', '');
        }

        if (result.length > 0) {
            this.update_log("Error while keeping test running: " + result + "\n");
        }
    }

    async start_mode(mode?: [number, string]) {
        let cookie: number = Math.floor(Math.random() * 0xFFFFFFFF);

        this.pending_log = '';

        this.setState({
            table_id: this.props.table_id,
            mode: mode,
            running: true,
            cookie: cookie,
            stopped: false,
            show_log: true,
            log: '',
        }, async () => {
            let table = [this.props.table_id, {device_address: this.props.device_address}];

            if (util.hasValue(this.props.repeat_interval)) {
                (table[1] as any)["repeat_interval"] = this.props.repeat_interval;
            }

            if (util.hasValue(this.props.register_blocks)) {
                (table[1] as any)["register_blocks"] = this.props.register_blocks;
            }

            if (util.hasValue(mode)) {
                (table[1] as any)["mode"] = mode[0];
            }

            if (util.hasValue(this.props.extra_values)) {
                for (let key in this.props.extra_values) {
                    (table[1] as any)[key] = this.props.extra_values[key];
                }
            }

            let result;

            try {
                result = await (await util.put('/batteries_modbus_tcp/test', {
                    slot: this.props.slot,
                    host: this.props.host,
                    port: this.props.port,
                    table: table,
                    cookie: cookie,
                })).text();
            }
            catch (e) {
                result = e.message.replace('400(Bad Request) ', '');
            }

            if (result.length > 0) {
                this.update_log("Error while starting test: " + result + "\n");
                this.setState({running: false, cookie: null});

                return;
            }

            this.continue_timer = window.setInterval(async () => {await this.test_continue()}, TEST_CONTINUE_INTERVAL);
            this.log_timer = window.setInterval(() => this.update_log(), TEST_LOG_INTERVAL);
        });
    }

    render() {
        let start_button_disabled = this.props.host.trim().length == 0
                                 || !util.hasValue(this.props.port)
                                 || (util.hasValue(this.props.register_blocks) && this.props.register_blocks.length == 0);

        return <>
            <FormRow label={__("batteries_modbus_tcp.content.test")} label_muted={__("batteries_modbus_tcp.content.test_muted")}>
            {!this.state.running ?
                (this.props.modes ?
                    <Dropdown key="test_start">
                        <Dropdown.Toggle
                            variant="primary"
                            className="form-control"
                            disabled={start_button_disabled}>
                            {__("batteries_modbus_tcp.content.test_start")}
                        </Dropdown.Toggle>
                        <Dropdown.Menu alignRight>
                            <Dropdown.Header class="text-wrap">{__("batteries_modbus_tcp.content.test_mode")}</Dropdown.Header>
                            {this.props.modes.map(mode =>
                                <Dropdown.Item
                                    as="button"
                                    className="py-2"
                                    onClick={async () => await this.start_mode(mode)}>
                                    {mode[1]}
                                </Dropdown.Item>)}
                        </Dropdown.Menu>
                    </Dropdown> :
                    <Button key="test_start"
                            variant="primary"
                            className="form-control"
                            onClick={async () => await this.start_mode()}
                            disabled={start_button_disabled}>
                        {__("batteries_modbus_tcp.content.test_start")}
                    </Button>) :
                <Button key="test_stop"
                        variant="primary"
                        className="form-control"
                        onClick={async () => await this.stop_test()}
                        disabled={this.state.stopped}>
                    {__("batteries_modbus_tcp.content.test_stop")(util.hasValue(this.state.mode) ? this.state.mode[1] : null)}
                </Button>}
            </FormRow>

            {this.props.table_id === this.state.table_id && this.state.show_log ?
                <FormRow label="">
                    <OutputTextarea rows={10} resize='vertical' value={this.state.log} />
                </FormRow>
                : undefined}
        </>;
    }
}

function import_custom_table(table: RegisterTable)
{
    if (!util.isNonNullObject(table.register_blocks)) {
        console.log("Batteries Modbus/TCP: Imported config register_blocks is not an object");
        return null;
    }

    if (typeof table.register_blocks.length != "number") {
        console.log("Batteries Modbus/TCP: Imported config register_blocks has no length");
        return null;
    }

    let register_blocks: RegisterBlock[] = [];

    for (let i = 0; i < table.register_blocks.length; ++i) {
        if (!util.isNonNullObject(table.register_blocks[i])) {
            console.log("Batteries Modbus/TCP: Imported config register_blocks item is not an object");
            return null;
        }

        if (typeof table.register_blocks[i].desc != "string") {
            console.log("Batteries Modbus/TCP: Imported config register_blocks item desc is not a number");
            return null;
        }

        if (typeof table.register_blocks[i].func != "number") {
            console.log("Batteries Modbus/TCP: Imported config register_blocks item func is not a number");
            return null;
        }

        if (typeof table.register_blocks[i].addr != "number") {
            console.log("Batteries Modbus/TCP: Imported config register_blocks item addr is not a number");
            return null;
        }

        if (!util.isNonNullObject(table.register_blocks[i].vals)) {
            console.log("Batteries Modbus/TCP: Imported config register_blocks item vals is not an object");
            return null;
        }

        if (typeof table.register_blocks[i].vals.length != "number") {
            console.log("Batteries Modbus/TCP: Imported config register_blocks item vals has no length");
            return null;
        }

        let values: number[] = [];

        for (let k = 0; k < table.register_blocks[i].vals.length; ++k) {
            if (typeof table.register_blocks[i].vals[k] != "number") {
                console.log("Batteries Modbus/TCP: Imported config register_blocks item vals item is not a number");
                return null;
            }

            values.push(table.register_blocks[i].vals[k]);
        }

        register_blocks.push({
            desc: table.register_blocks[i].desc,
            func: table.register_blocks[i].func,
            addr: table.register_blocks[i].addr,
            vals: values,
        });
    }

    return {register_blocks: register_blocks};
}

export function init() {
    return {
        [BatteryClassID.ModbusTCP]: {
            name: () => __("batteries_modbus_tcp.content.battery_class"),
            new_config: () => [BatteryClassID.ModbusTCP, {display_name: "", host: "", port: 502, table: null}] as BatteryConfig,
            clone_config: (config: BatteryConfig) => [config[0], {...config[1]}] as BatteryConfig,
            import_config: (new_config: BatteryConfig, current_config: BatteryConfig) => {
                if (!util.isNonNullObject(new_config)) {
                    console.log("Batteries Modbus/TCP: JSON of imported config has no toplevel object");
                    return null;
                }

                if (new_config[0] != BatteryClassID.ModbusTCP) {
                    console.log("Batteries Modbus/TCP: Imported config has wrong class:", new_config[0]);
                    return null;
                }

                if (!util.isNonNullObject(new_config[1])) {
                    console.log("Batteries Modbus/TCP: Imported config union is not an object");
                    return null;
                }

                if (typeof new_config[1].port != "number") {
                    console.log("Batteries Modbus/TCP: Imported config port is not a number");
                    return null;
                }

                if (!util.isNonNullObject(new_config[1].table)) {
                    console.log("Batteries Modbus/TCP: Imported config table union is not an object");
                    return null;
                }

                if (typeof new_config[1].table[0] != "number") {
                    console.log("Batteries Modbus/TCP: Imported config table union class is not a number");
                    return null;
                }

                if (!util.isNonNullObject(new_config[1].table[1])) {
                    console.log("Batteries Modbus/TCP: Imported config table union value is not an object");
                    return null;
                }

                let table: TableConfig;

                if (new_config[1].table[0] == BatteryModbusTCPTableID.Custom) {
                    if (typeof new_config[1].table[1].device_address != "number") {
                        console.log("Batteries Modbus/TCP: Imported config device_address is not a number");
                        return null;
                    }

                    if (typeof new_config[1].table[1].repeat_interval != "number") {
                        console.log("Batteries Modbus/TCP: Imported config repeat_interval is not a number");
                        return null;
                    }

                    if (typeof new_config[1].table[1].register_address_mode != "number") {
                        console.log("Batteries Modbus/TCP: Imported config register_address_mode is not a number");
                        return null;
                    }

                    if (!util.isNonNullObject(new_config[1].table[1].battery_modes)) {
                        console.log("Batteries Modbus/TCP: Imported config battery_modes is not an object");
                        return null;
                    }

                    table = [BatteryModbusTCPTableID.Custom, {
                        device_address:        new_config[1].table[1].device_address,
                        repeat_interval:       new_config[1].table[1].repeat_interval,
                        register_address_mode: new_config[1].table[1].register_address_mode,
                        battery_modes:         [],
                    }];

                    for (let i = 0; i < 6; ++i) {
                        if (!util.isNonNullObject(table)) {
                            console.log(`Batteries Modbus/TCP: Imported config battery_modes[${i}] is not an object`);
                            return null;
                        }

                        table[1].battery_modes[i] = import_custom_table(new_config[1].table[1].battery_modes[i]);

                        if (!util.hasValue(table[1].battery_modes[i])) {
                            return null;
                        }
                    }
                }
                else {
                    table = import_table_config(new_config[1].table);

                    if (!util.hasValue(table)) {
                        return null;
                    }
                }

                let display_name = "";
                let host = "";

                if (util.isNonNullObject(current_config[1])) {
                    if (typeof current_config[1].display_name == "string") {
                        display_name = current_config[1].display_name;
                    }

                    if (typeof current_config[1].host == "string") {
                        host = current_config[1].host;
                    }
                }

                return [BatteryClassID.ModbusTCP, {display_name: display_name, host: host, port: new_config[1].port, table: table}] as BatteryConfig;
            },
            export_basename_suffix: () => __("batteries_modbus_tcp.content.export_basename_suffix"),
            export_config: (config: BatteryConfig) => {
                let clone: BatteryConfig = {...config};

                delete clone[1].display_name;
                delete clone[1].host;

                return clone;
            },
            get_edit_children: (battery_slot: number, config: ModbusTCPBatteriesConfig, on_config: (config: ModbusTCPBatteriesConfig) => void): ComponentChildren => {
                let edit_children = [
                    <FormRow label={__("batteries_modbus_tcp.content.display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {display_name: v}));
                            }} />
                    </FormRow>,
                    <FormRow label={__("batteries_modbus_tcp.content.host")}>
                        <InputTextPatterned
                            required
                            maxLength={64}
                            pattern="^[a-zA-Z0-9\-\.]+$"
                            value={config[1].host}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {host: v}));
                            }}
                            invalidFeedback={__("batteries_modbus_tcp.content.host_invalid")} />
                    </FormRow>,
                    <FormRow label={__("batteries_modbus_tcp.content.port")} label_muted={__("batteries_modbus_tcp.content.port_muted")}>
                        <InputNumber
                            required
                            min={1}
                            max={65535}
                            value={config[1].port}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {port: v}));
                            }} />
                    </FormRow>,
                    <FormRow label={__("batteries_modbus_tcp.content.table")}>
                        <InputSelect
                            required
                            items={[
                                // Keep alphabetically sorted
                                [BatteryModbusTCPTableID.AlphaESSHybridInverter.toString(), __("batteries_modbus_tcp.content.table_alpha_ess_hybrid_inverter")],
                                [BatteryModbusTCPTableID.DeyeHybridInverter.toString(), __("batteries_modbus_tcp.content.table_deye_hybrid_inverter")],
                                [BatteryModbusTCPTableID.HaileiHybridInverter.toString(), __("batteries_modbus_tcp.content.table_hailei_hybrid_inverter")],
                                [BatteryModbusTCPTableID.SungrowHybridInverter.toString(), __("batteries_modbus_tcp.content.table_sungrow_hybrid_inverter")],
                                [BatteryModbusTCPTableID.VictronEnergyGX.toString(), __("batteries_modbus_tcp.content.table_victron_energy_gx")],
                                [BatteryModbusTCPTableID.Custom.toString(), __("batteries_modbus_tcp.content.table_custom")],
                            ]}
                            placeholder={__("select")}
                            value={util.hasValue(config[1].table) ? config[1].table[0].toString() : undefined}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {table: new_table_config(parseInt(v))}));
                            }} />
                    </FormRow>,
                ];

                let battery_mode_names = [
                    __("batteries_modbus_tcp.content.battery_mode_disable"),
                    __("batteries_modbus_tcp.content.battery_mode_normal"),
                    __("batteries_modbus_tcp.content.battery_mode_charge_from_excess"),
                    __("batteries_modbus_tcp.content.battery_mode_charge_from_grid"),
                    __("batteries_modbus_tcp.content.battery_mode_discharge_to_load"),
                    __("batteries_modbus_tcp.content.battery_mode_discharge_to_grid"),
                ];

                let battery_mode_order = [
                    BatteryMode.Normal,
                    BatteryMode.ChargeFromExcess,
                    BatteryMode.DischargeToLoad,
                    BatteryMode.Disable,
                    BatteryMode.DischargeToGrid,
                    BatteryMode.ChargeFromGrid,
                ];

                let battery_mode_items = battery_mode_order.map(mode => [mode, battery_mode_names[mode]]);

                if (util.hasValue(config[1].table)
                 && (config[1].table[0] == BatteryModbusTCPTableID.VictronEnergyGX
                  || config[1].table[0] == BatteryModbusTCPTableID.DeyeHybridInverter
                  || config[1].table[0] == BatteryModbusTCPTableID.AlphaESSHybridInverter
                  || config[1].table[0] == BatteryModbusTCPTableID.HaileiHybridInverter
                  || config[1].table[0] == BatteryModbusTCPTableID.SungrowHybridInverter)) {
                    edit_children.push(
                        <FormRow label={__("batteries_modbus_tcp.content.device_address")} label_muted={__("batteries_modbus_tcp.content.device_address_muted")(get_default_device_address(config[1].table[0]))}>
                            <InputNumber
                                required
                                min={0}
                                max={255}
                                value={config[1].table[1].device_address}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {device_address: v})}));
                                }} />
                        </FormRow>);

                    let extra_values = undefined;

                    if (config[1].table[0] == BatteryModbusTCPTableID.VictronEnergyGX) {
                        edit_children.push(
                            <FormRow label={__("batteries_modbus_tcp.content.grid_draw_setpoint_normal")}>
                                <InputNumber
                                    required
                                    min={-2147483648}
                                    max={2147483647}
                                    unit="W"
                                    value={config[1].table[1].grid_draw_setpoint_normal}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {grid_draw_setpoint_normal: v})}));
                                    }} />
                            </FormRow>,
                            <FormRow label={__("batteries_modbus_tcp.content.grid_draw_setpoint_charge")}>
                                <InputNumber
                                    required
                                    min={-2147483648}
                                    max={2147483647}
                                    unit="W"
                                    value={config[1].table[1].grid_draw_setpoint_charge}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {grid_draw_setpoint_charge: v})}));
                                    }} />
                            </FormRow>,
                            <FormRow label={__("batteries_modbus_tcp.content.grid_draw_setpoint_discharge")}>
                                <InputNumber
                                    required
                                    min={-2147483648}
                                    max={2147483647}
                                    unit="W"
                                    value={config[1].table[1].grid_draw_setpoint_discharge}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {grid_draw_setpoint_discharge: v})}));
                                    }} />
                            </FormRow>);

                        extra_values = {
                            grid_draw_setpoint_normal: config[1].table[1].grid_draw_setpoint_normal,
                            grid_draw_setpoint_charge: config[1].table[1].grid_draw_setpoint_charge,
                            grid_draw_setpoint_discharge: config[1].table[1].grid_draw_setpoint_discharge,
                        };
                    }
                    else if (config[1].table[0] == BatteryModbusTCPTableID.DeyeHybridInverter) {
                        edit_children.push(
                            <FormRow label={__("batteries_modbus_tcp.content.max_charge_current")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={185}
                                    unit="A"
                                    value={config[1].table[1].max_charge_current}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {max_charge_current: v})}));
                                    }} />
                            </FormRow>,
                            <FormRow label={__("batteries_modbus_tcp.content.max_discharge_current")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={185}
                                    unit="A"
                                    value={config[1].table[1].max_discharge_current}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {max_discharge_current: v})}));
                                    }} />
                            </FormRow>,
                            <FormRow label={__("batteries_modbus_tcp.content.min_soc")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={100}
                                    unit="%"
                                    value={config[1].table[1].min_soc}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {min_soc: v})}));
                                    }} />
                            </FormRow>,
                            <FormRow label={__("batteries_modbus_tcp.content.max_soc")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={100}
                                    unit="%"
                                    value={config[1].table[1].max_soc}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {max_soc: v})}));
                                    }} />
                            </FormRow>);

                        extra_values = {
                            max_charge_current: config[1].table[1].max_charge_current,
                            max_discharge_current: config[1].table[1].max_discharge_current,
                            min_soc: config[1].table[1].min_soc,
                            max_soc: config[1].table[1].max_soc,
                        };
                    }
                    else if (config[1].table[0] == BatteryModbusTCPTableID.AlphaESSHybridInverter
                          || config[1].table[0] == BatteryModbusTCPTableID.HaileiHybridInverter) {
                        edit_children.push(
                            <FormRow label={__("batteries_modbus_tcp.content.max_soc")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={100}
                                    unit="%"
                                    value={config[1].table[1].max_soc}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {max_soc: v})}));
                                    }} />
                            </FormRow>);

                        extra_values = {
                            max_soc: config[1].table[1].max_soc,
                        };
                    }
                    else if (config[1].table[0] == BatteryModbusTCPTableID.SungrowHybridInverter) {
                        edit_children.push(
                            <FormRow label={__("batteries_modbus_tcp.content.grid_charge_power")}>
                                <InputFloat
                                    required
                                    digits={3}
                                    min={0}
                                    max={65535}
                                    unit="kW"
                                    value={config[1].table[1].grid_charge_power}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {grid_charge_power: v})}));
                                    }} />
                            </FormRow>,
                            <FormRow label={__("batteries_modbus_tcp.content.grid_discharge_power")}>
                                <InputFloat
                                    required
                                    digits={3}
                                    min={0}
                                    max={65535}
                                    unit="kW"
                                    value={config[1].table[1].grid_discharge_power}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {grid_discharge_power: v})}));
                                    }} />
                            </FormRow>,
                            <FormRow label={__("batteries_modbus_tcp.content.max_charge_power")}>
                                <InputFloat
                                    required
                                    digits={2}
                                    min={1}
                                    max={65535}
                                    unit="kW"
                                    value={config[1].table[1].max_charge_power}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {max_charge_power: v})}));
                                    }} />
                            </FormRow>,
                            <FormRow label={__("batteries_modbus_tcp.content.max_discharge_power")}>
                                <InputFloat
                                    required
                                    digits={2}
                                    min={1}
                                    max={65535}
                                    unit="kW"
                                    value={config[1].table[1].max_discharge_power}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {max_discharge_power: v})}));
                                    }} />
                            </FormRow>);

                        extra_values = {
                            grid_charge_power: config[1].table[1].grid_charge_power,
                            grid_discharge_power: config[1].table[1].grid_discharge_power,
                            max_charge_power: config[1].table[1].max_charge_power,
                            max_discharge_power: config[1].table[1].max_discharge_power,
                        };
                    }

                    edit_children.push(
                        <TestRunner
                            slot={battery_slot}
                            host={config[1].host}
                            port={config[1].port}
                            table_id={config[1].table[0]}
                            device_address={config[1].table[1].device_address}
                            modes={battery_mode_items as [number, string][]}
                            extra_values={extra_values} />);
                }
                else if (util.hasValue(config[1].table)
                      && config[1].table[0] == BatteryModbusTCPTableID.Custom) {
                    let total_values_count = 0;

                    for (let i = 0; i < 6; ++i) {
                        total_values_count += get_total_values_count(config[1].table[1].battery_modes[i]);
                    }

                    edit_children.push(
                        <FormRow label={__("batteries_modbus_tcp.content.device_address")}>
                            <InputNumber
                                required
                                min={0}
                                max={255}
                                value={config[1].table[1].device_address}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {device_address: v})}));
                                }} />
                        </FormRow>,
                        <FormRow label={__("batteries_modbus_tcp.content.repeat_interval")} label_muted={__("batteries_modbus_tcp.content.repeat_interval_muted")}>
                            <InputNumber
                                required
                                unit={"s"}
                                min={0}
                                max={65535}
                                value={config[1].table[1].repeat_interval}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {repeat_interval: v})}));
                                }} />
                        </FormRow>,
                        <FormRow label={__("batteries_modbus_tcp.content.register_address_mode")}>
                            <InputSelect
                                required
                                items={[
                                    [ModbusRegisterAddressMode.Address.toString(), __("batteries_modbus_tcp.content.register_address_mode_address")],
                                    [ModbusRegisterAddressMode.Number.toString(), __("batteries_modbus_tcp.content.register_address_mode_number")]
                                ]}
                                placeholder={__("select")}
                                value={util.hasValue(config[1].table[1].register_address_mode) ? config[1].table[1].register_address_mode.toString() : undefined}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {...(config[1].table as TableConfigCustom)[1], register_address_mode: parseInt(v)})}));
                                }} />
                        </FormRow>);

                    for (let i = 0; i < 6; ++i) {
                        let mode = battery_mode_order[i];

                        edit_children.push(
                            <CollapsedSection heading={__("batteries_modbus_tcp.content.register_title")(battery_mode_names[mode])} modal={true}>
                                <FormRow label={__("batteries_modbus_tcp.content.register_blocks")}>
                                    <RegisterEditor
                                        register_address_mode={config[1].table[1].register_address_mode}
                                        other_total_values_count={total_values_count - get_total_values_count(config[1].table[1].battery_modes[mode])}
                                        table={config[1].table[1].battery_modes[mode]}
                                        on_table={(table: RegisterTable) => on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {battery_modes:(config[1].table as TableConfigCustom)[1].battery_modes.map((battery_mode, k) => k == mode ? table : battery_mode)})}))} />
                                </FormRow>
                                <TestRunner
                                    slot={battery_slot}
                                    host={config[1].host}
                                    port={config[1].port}
                                    table_id={config[1].table[0]}
                                    device_address={config[1].table[1].device_address}
                                    repeat_interval={config[1].table[1].repeat_interval}
                                    register_blocks={config[1].table[1].battery_modes[mode].register_blocks} />
                            </CollapsedSection>);
                    }
                }

                return edit_children;
            },
        },
    };
}
