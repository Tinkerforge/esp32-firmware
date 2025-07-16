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
import { Button } from "react-bootstrap";
import { __ } from "../../ts/translation";
import { BatteryClassID } from "../batteries/battery_class_id.enum";
import { BatteryConfig } from "../batteries/types";
import { BatteryModbusTCPTableID } from "./battery_modbus_tcp_table_id.enum";
import { ModbusFunctionCode } from "../modbus_tcp_client/modbus_function_code.enum";
import { ModbusRegisterAddressMode } from "../modbus_tcp_client/modbus_register_address_mode.enum";
import { InputText, InputTextPatterned } from "../../ts/components/input_text";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { Table, TableRow } from "../../ts/components/table";

type TableConfigNone = [
    BatteryModbusTCPTableID.None,
    {},
];

type RegisterBlock = {
    desc: string;
    func: number; // ModbusFunctionCode
    addr: number;
    vals: number[];
};

type RegisterTable = {
    device_address: number;
    register_blocks: RegisterBlock[];
};

type TableConfigCustom = [
    BatteryModbusTCPTableID.Custom,
    {
        register_address_mode: number, // ModbusRegisterAddressMode
        permit_grid_charge: RegisterTable,
        revoke_grid_charge_override: RegisterTable,
        forbid_discharge: RegisterTable,
        revoke_discharge_override: RegisterTable,
        forbid_charge: RegisterTable,
        revoke_charge_override: RegisterTable,
    },
];

type TableConfig = TableConfigNone |
                   TableConfigCustom;

export type ModbusTCPBatteriesConfig = [
    BatteryClassID.ModbusTCP,
    {
        display_name: string;
        host: string;
        port: number;
        table: TableConfig;
    },
];

function new_table_config(table: BatteryModbusTCPTableID): TableConfig {
    switch (table) {
        case BatteryModbusTCPTableID.Custom:
            return [BatteryModbusTCPTableID.Custom, {
                register_address_mode:       null,
                permit_grid_charge:          {device_address: 1, register_blocks: []},
                revoke_grid_charge_override: {device_address: 1, register_blocks: []},
                forbid_discharge:            {device_address: 1, register_blocks: []},
                revoke_discharge_override:   {device_address: 1, register_blocks: []},
                forbid_charge:               {device_address: 1, register_blocks: []},
                revoke_charge_override:      {device_address: 1, register_blocks: []},
            }];

        default:
            return [BatteryModbusTCPTableID.None, null];
    }
}

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

        switch (this.state.register_block.func) {
        case ModbusFunctionCode.WriteSingleCoil:
            values_pattern = "^ *[01] *$";
            values_label = __("batteries_modbus_tcp.content.register_blocks_value");
            values_label_muted = __("batteries_modbus_tcp.content.register_blocks_value_muted");
            break;

        case ModbusFunctionCode.WriteSingleRegister:
            values_pattern = `^ *${util.UINT16_PATTERN} *$`;
            values_label = __("batteries_modbus_tcp.content.register_blocks_value");
            values_label_muted = __("batteries_modbus_tcp.content.register_blocks_value_muted");
            break;

        case ModbusFunctionCode.WriteMultipleCoils:
            values_pattern = `^ *[01] *(, *[01] *){0,${options.BATTERIES_MODBUS_TCP_MAX_CUSTOM_VALUES_PER_REGISTER_BLOCK - 1}}$`;
            values_label = __("batteries_modbus_tcp.content.register_blocks_values");
            values_label_muted = __("batteries_modbus_tcp.content.register_blocks_values_muted");
            break;

        case ModbusFunctionCode.WriteMultipleRegisters:
            values_pattern = `^ *${util.UINT16_PATTERN} *(?:, *${util.UINT16_PATTERN} *){0,${options.BATTERIES_MODBUS_TCP_MAX_CUSTOM_VALUES_PER_REGISTER_BLOCK - 1}}$`;
            values_label = __("batteries_modbus_tcp.content.register_blocks_values");
            values_label_muted = __("batteries_modbus_tcp.content.register_blocks_values_muted");
            break;
        }

        return [
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
            </FormRow>,
            <FormRow label={values_label} label_muted={values_label_muted}>
                <InputTextPatterned
                    required
                    pattern={values_pattern}
                    value={this.state.values}
                    onValue={(v) => {
                        this.setState({values: v});
                    }}
                    invalidFeedback={__("batteries_modbus_tcp.content.register_blocks_values_invalid")} />
            </FormRow>,
        ];
    }

    parse_values(values_str: string) {
        let max_values_count = 0;
        let max_value = 0;

        switch (this.state.register_block.func) {
        case ModbusFunctionCode.WriteSingleCoil:
            max_values_count = 1;
            max_value = 1;
            break;

        case ModbusFunctionCode.WriteSingleRegister:
            max_values_count = 1;
            max_value = 65535;
            break;

        case ModbusFunctionCode.WriteMultipleCoils:
            max_values_count = 1968;
            max_value = 1;
            break;

        case ModbusFunctionCode.WriteMultipleRegisters:
            max_values_count = 123;
            max_value = 65535;
            break;
        }

        let values_dec = values_str.split(",");

        if (values_dec.length > max_values_count) {
            return [];
        }

        let values: number[] = [];

        for (let value_dec of values_dec) {
            value_dec = value_dec.trim();

            let value = parseInt(value_dec, 10);

            if (isNaN(value)) {
                return [];
            }

            if (value > max_value) {
                return [];
            }

            if ("" + value !== value_dec) {
                return [];
            }

            values.push(value);
        }

        return values;
    }

    render() {
        let start_address_offset = this.props.register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1;
        let total_values_count = this.props.other_total_values_count + get_total_values_count(this.props.table);

        return <Table
            nestingDepth={1}
            rows={this.props.table.register_blocks.map((register_block, i) => {
                const row: TableRow = {
                    columnValues: [<>{register_block.desc.length > 0 ? <div>{register_block.desc}</div> : undefined}<div style={register_block.desc.length > 0 ? "font-size: 80%" : ""}>{__("batteries_modbus_tcp.content.register_blocks_values_desc")(register_block.func, register_block.addr + start_address_offset, register_block.vals)}</div></>],
                    onRemoveClick: async () => {
                        this.props.on_table({...this.props.table, register_blocks: this.props.table.register_blocks.filter((r, k) => k !== i)});
                        return true;
                    },
                    onEditShow: async () => {
                        this.setState({
                            register_block: register_block,
                            values: register_block.vals.join(", "),
                        });
                    },
                    onEditSubmit: async () => {
                        this.props.on_table({...this.props.table, register_blocks: this.props.table.register_blocks.map((r, k) => k === i ? {...this.state.register_block, vals: this.parse_values(this.state.values)} : r)});
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
                this.props.on_table({...this.props.table, register_blocks: this.props.table.register_blocks.concat([{...this.state.register_block, vals: this.parse_values(this.state.values)}])});
            }}/>;
    }
}

interface ExecutorProps {
    host: string;
    port: number;
    table: RegisterTable;
}

interface ExecutorState {
    waiting: boolean;
    cookie: number;
    result: string;
}

class Executor extends Component<ExecutorProps, ExecutorState> {
    constructor(props: ExecutorProps) {
        super(props);

        this.state = {
            waiting: false,
            cookie: null,
            result: "",
        } as ExecutorState;

        util.addApiEventListener('batteries_modbus_tcp/execute_result', () => {
            let execute_result = API.get('batteries_modbus_tcp/execute_result');

            if (!this.state.waiting || execute_result.cookie !== this.state.cookie) {
                return;
            }

            let result: string = __("batteries_modbus_tcp.content.execute_done");

            if (execute_result.error !== null) {
                result = __("batteries_modbus_tcp.content.execute_error_prefix") + execute_result.error; // FIXME: translate
            }

            this.setState({waiting: false, cookie: null, result: result});
        });
    }

    async execute() {
        let cookie: number = Math.floor(Math.random() * 0xFFFFFFFF);

        this.setState({waiting: true, cookie: cookie, result: ""}, async () => {
            let result = "<unknown>";

            try {
                result = await (await util.put("/batteries_modbus_tcp/execute", {
                    host: this.props.host,
                    port: this.props.port,
                    table: [BatteryModbusTCPTableID.Custom, {
                        device_address: this.props.table.device_address,
                        register_blocks: this.props.table.register_blocks,
                    }],
                    cookie: cookie,
                })).text();
            }
            catch (e) {
                result = e.message.replace("400(Bad Request) ", "");
            }

            if (result.length > 0) {
                this.setState({waiting: false, cookie: null, result: __("batteries_modbus_tcp.content.execute_error_prefix") + result});
            }
        });
    }

    render() {
        return <>
            <FormRow label="">
                <Button
                    variant="primary"
                    className="form-control"
                    disabled={!util.hasValue(this.props.host) || this.props.host.length == 0 || this.props.table.register_blocks.length == 0 || this.state.waiting}
                    onClick={async () => await this.execute()}>{__("batteries_modbus_tcp.content.execute")}
                </Button>
            </FormRow>

            {this.state.waiting || this.state.result.length > 0 ?
                <FormRow label={__("batteries_modbus_tcp.content.execute_response")}>
                    <InputText value={this.state.waiting ? __("batteries_modbus_tcp.content.execute_waiting") : this.state.result} />
                </FormRow>
                : undefined}
        </>;
    }
}

function import_register_table(table: RegisterTable)
{
    if (!util.isNonNullObject(table)) {
        console.log("Batteries Modbus/TCP: Imported config register table is not an object");
        return null;
    }

    if (typeof table.device_address != "number") {
        console.log("Batteries Modbus/TCP: Imported config register table device address is not a number");
        return null;
    }

    if (!util.isNonNullObject(table.register_blocks)) {
        console.log("Batteries Modbus/TCP: Imported config register table register blocks is not an object");
        return null;
    }

    if (typeof table.register_blocks.length != "number") {
        console.log("Batteries Modbus/TCP: Imported config register table register blocks has no length");
        return null;
    }

    let register_blocks: RegisterBlock[] = [];

    for (let i = 0; i < table.register_blocks.length; ++i) {
        if (!util.isNonNullObject(table.register_blocks[i])) {
            console.log("Batteries Modbus/TCP: Imported config register table register blocks item is not an object");
            return null;
        }

        if (typeof table.register_blocks[i].desc != "string") {
            console.log("Batteries Modbus/TCP: Imported config register table register blocks item desc is not a number");
            return null;
        }

        if (typeof table.register_blocks[i].func != "number") {
            console.log("Batteries Modbus/TCP: Imported config register table register blocks item func is not a number");
            return null;
        }

        if (typeof table.register_blocks[i].addr != "number") {
            console.log("Batteries Modbus/TCP: Imported config register table register blocks item addr is not a number");
            return null;
        }

        if (!util.isNonNullObject(table.register_blocks[i].vals)) {
            console.log("Batteries Modbus/TCP: Imported config register table register blocks item vals is not an object");
            return null;
        }

        if (typeof table.register_blocks[i].vals.length != "number") {
            console.log("Batteries Modbus/TCP: Imported config register table register blocks item vals has no length");
            return null;
        }

        let values: number[] = [];

        for (let k = 0; k < table.register_blocks[i].vals.length; ++k) {
            if (typeof table.register_blocks[i].vals[k] != "number") {
                console.log("Batteries Modbus/TCP: Imported config register table register blocks item vals item is not a number");
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

    return {device_address: table.device_address, register_blocks: register_blocks};
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
                    console.log("Batteries Modbus/TCP: Imported config union value is not an object");
                    return null;
                }

                if (typeof new_config[1].port != "number") {
                    console.log("Batteries Modbus/TCP: Imported config port is not a number");
                    return null;
                }

                if (!util.isNonNullObject(new_config[1].table)) {
                    console.log("Batteries Modbus/TCP: Imported config table union value is not an object");
                    return null;
                }

                let table: TableConfigCustom;

                switch (new_config[1].table[0]) {
                    case BatteryModbusTCPTableID.Custom:
                        if (typeof new_config[1].table[1].register_address_mode != "number") {
                            console.log("Batteries Modbus/TCP: Imported config register address mode is not a number");
                            return null;
                        }

                        table = [BatteryModbusTCPTableID.Custom, {
                            register_address_mode:       new_config[1].table[1].register_address_mode,
                            permit_grid_charge:          import_register_table(new_config[1].table[1].permit_grid_charge),
                            revoke_grid_charge_override: import_register_table(new_config[1].table[1].revoke_grid_charge_override),
                            forbid_discharge:            import_register_table(new_config[1].table[1].forbid_discharge),
                            revoke_discharge_override:   import_register_table(new_config[1].table[1].revoke_discharge_override),
                            forbid_charge:               import_register_table(new_config[1].table[1].forbid_charge),
                            revoke_charge_override:      import_register_table(new_config[1].table[1].revoke_charge_override),
                        }]

                        if (!util.hasValue(table[1].permit_grid_charge)
                         || !util.hasValue(table[1].revoke_grid_charge_override)
                         || !util.hasValue(table[1].forbid_discharge)
                         || !util.hasValue(table[1].revoke_discharge_override)
                         || !util.hasValue(table[1].forbid_charge)
                         || !util.hasValue(table[1].revoke_charge_override)) {
                            return null;
                        }

                        break;

                    default:
                        console.log("Batteries Modbus/TCP: Imported config table has unknown class:", new_config[1].table[0]);
                        return null;
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
            get_edit_children: (config: ModbusTCPBatteriesConfig, on_config: (config: ModbusTCPBatteriesConfig) => void): ComponentChildren => {
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
                                [BatteryModbusTCPTableID.Custom.toString(), __("batteries_modbus_tcp.content.table_custom")],
                            ]}
                            placeholder={__("select")}
                            value={util.hasValue(config[1].table) ? config[1].table[0].toString() : undefined}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {table: new_table_config(parseInt(v))}));
                            }} />
                    </FormRow>,
                ];

                if (util.hasValue(config[1].table) && config[1].table[0] == BatteryModbusTCPTableID.Custom) {
                    let total_values_count_permit_grid_charge = get_total_values_count(config[1].table[1].permit_grid_charge);
                    let total_values_count_revoke_grid_charge_override = get_total_values_count(config[1].table[1].revoke_grid_charge_override);
                    let total_values_count_forbid_discharge = get_total_values_count(config[1].table[1].forbid_discharge);
                    let total_values_count_revoke_discharge_override = get_total_values_count(config[1].table[1].revoke_discharge_override);
                    let total_values_count_forbid_charge = get_total_values_count(config[1].table[1].forbid_charge);
                    let total_values_count_revoke_charge_override = get_total_values_count(config[1].table[1].revoke_charge_override);

                    let total_values_count = total_values_count_permit_grid_charge
                                           + total_values_count_revoke_grid_charge_override
                                           + total_values_count_forbid_discharge
                                           + total_values_count_revoke_discharge_override
                                           + total_values_count_forbid_charge
                                           + total_values_count_revoke_charge_override;

                    edit_children.push(
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
                        </FormRow>,

                        <CollapsedSection heading={__("batteries_modbus_tcp.content.permit_grid_charge")} modal={true}>
                            <FormRow label={__("batteries_modbus_tcp.content.device_address")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={255}
                                    value={config[1].table[1].permit_grid_charge.device_address}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {permit_grid_charge: {...(config[1].table as TableConfigCustom)[1].permit_grid_charge, device_address: v}})}));
                                    }} />
                            </FormRow>
                            <FormRow label={__("batteries_modbus_tcp.content.register_blocks")}>
                                <RegisterEditor
                                    register_address_mode={config[1].table[1].register_address_mode}
                                    other_total_values_count={total_values_count - total_values_count_permit_grid_charge}
                                    table={config[1].table[1].permit_grid_charge}
                                    on_table={(table: RegisterTable) => on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {permit_grid_charge: table})}))} />
                            </FormRow>
                            <Executor host={config[1].host} port={config[1].port} table={config[1].table[1].permit_grid_charge} />
                        </CollapsedSection>,

                        <CollapsedSection heading={__("batteries_modbus_tcp.content.revoke_grid_charge_override")} modal={true}>
                            <FormRow label={__("batteries_modbus_tcp.content.device_address")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={255}
                                    value={config[1].table[1].revoke_grid_charge_override.device_address}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {revoke_grid_charge_override: {...(config[1].table as TableConfigCustom)[1].revoke_grid_charge_override, device_address: v}})}));
                                    }} />
                            </FormRow>
                            <FormRow label={__("batteries_modbus_tcp.content.register_blocks")}>
                                <RegisterEditor
                                    register_address_mode={config[1].table[1].register_address_mode}
                                    other_total_values_count={total_values_count - total_values_count_revoke_grid_charge_override}
                                    table={config[1].table[1].revoke_grid_charge_override}
                                    on_table={(table: RegisterTable) => on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {revoke_grid_charge_override: table})}))} />
                            </FormRow>
                            <Executor host={config[1].host} port={config[1].port} table={config[1].table[1].revoke_grid_charge_override} />
                        </CollapsedSection>,

                        <CollapsedSection heading={__("batteries_modbus_tcp.content.forbid_discharge")} modal={true}>
                            <FormRow label={__("batteries_modbus_tcp.content.device_address")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={255}
                                    value={config[1].table[1].forbid_discharge.device_address}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {forbid_discharge: {...(config[1].table as TableConfigCustom)[1].forbid_discharge, device_address: v}})}));
                                    }} />
                            </FormRow>
                            <FormRow label={__("batteries_modbus_tcp.content.register_blocks")}>
                                <RegisterEditor
                                    register_address_mode={config[1].table[1].register_address_mode}
                                    other_total_values_count={total_values_count - total_values_count_forbid_discharge}
                                    table={config[1].table[1].forbid_discharge}
                                    on_table={(table: RegisterTable) => on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {forbid_discharge: table})}))} />
                            </FormRow>
                            <Executor host={config[1].host} port={config[1].port} table={config[1].table[1].forbid_discharge} />
                        </CollapsedSection>,

                        <CollapsedSection heading={__("batteries_modbus_tcp.content.revoke_discharge_override")} modal={true}>
                            <FormRow label={__("batteries_modbus_tcp.content.device_address")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={255}
                                    value={config[1].table[1].revoke_discharge_override.device_address}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {revoke_discharge_override: {...(config[1].table as TableConfigCustom)[1].revoke_discharge_override, device_address: v}})}));
                                    }} />
                            </FormRow>
                            <FormRow label={__("batteries_modbus_tcp.content.register_blocks")}>
                                <RegisterEditor
                                    register_address_mode={config[1].table[1].register_address_mode}
                                    other_total_values_count={total_values_count - total_values_count_revoke_discharge_override}
                                    table={config[1].table[1].revoke_discharge_override}
                                    on_table={(table: RegisterTable) => on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {revoke_discharge_override: table})}))} />
                            </FormRow>
                            <Executor host={config[1].host} port={config[1].port} table={config[1].table[1].revoke_discharge_override} />
                        </CollapsedSection>,

                        <CollapsedSection heading={__("batteries_modbus_tcp.content.forbid_charge")} modal={true}>
                            <FormRow label={__("batteries_modbus_tcp.content.device_address")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={255}
                                    value={config[1].table[1].forbid_charge.device_address}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {forbid_charge: {...(config[1].table as TableConfigCustom)[1].forbid_charge, device_address: v}})}));
                                    }} />
                            </FormRow>
                            <FormRow label={__("batteries_modbus_tcp.content.register_blocks")}>
                                <RegisterEditor
                                    register_address_mode={config[1].table[1].register_address_mode}
                                    other_total_values_count={total_values_count - total_values_count_forbid_charge}
                                    table={config[1].table[1].forbid_charge}
                                    on_table={(table: RegisterTable) => on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {forbid_charge: table})}))} />
                            </FormRow>
                            <Executor host={config[1].host} port={config[1].port} table={config[1].table[1].forbid_charge} />
                        </CollapsedSection>,

                        <CollapsedSection heading={__("batteries_modbus_tcp.content.revoke_charge_override")} modal={true}>
                            <FormRow label={__("batteries_modbus_tcp.content.device_address")}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={255}
                                    value={config[1].table[1].revoke_charge_override.device_address}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {revoke_charge_override: {...(config[1].table as TableConfigCustom)[1].revoke_charge_override, device_address: v}})}));
                                    }} />
                            </FormRow>
                            <FormRow label={__("batteries_modbus_tcp.content.register_blocks")}>
                                <RegisterEditor
                                    register_address_mode={config[1].table[1].register_address_mode}
                                    other_total_values_count={total_values_count - total_values_count_revoke_charge_override}
                                    table={config[1].table[1].revoke_charge_override}
                                    on_table={(table: RegisterTable) => on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {revoke_charge_override: table})}))} />
                            </FormRow>
                            <Executor host={config[1].host} port={config[1].port} table={config[1].table[1].revoke_charge_override} />
                        </CollapsedSection>,
                    );
                }

                return edit_children;
            },
        },
    };
}
