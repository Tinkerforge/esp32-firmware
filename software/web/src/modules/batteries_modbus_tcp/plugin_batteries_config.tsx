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
import { h, Fragment, Component, ComponentChildren } from "preact";
import { Button } from "react-bootstrap";
import { __ } from "../../ts/translation";
import { BatteryClassID } from "../batteries/battery_class_id.enum";
import { BatteryConfig } from "../batteries/types";
import { BatteryModbusTCPTableID } from "./battery_modbus_tcp_table_id.enum";
import { ModbusRegisterType } from "../modbus_tcp_client/modbus_register_type.enum";
import { ModbusRegisterAddressMode } from "../modbus_tcp_client/modbus_register_address_mode.enum";
import { InputText, InputTextPatterned } from "../../ts/components/input_text";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { InputNumber } from "../../ts/components/input_number";
import { InputAnyFloat } from "../../ts/components/input_any_float";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { Table, TableRow } from "../../ts/components/table";

const MAX_CUSTOM_REGISTERS = 16;

type TableConfigNone = [
    BatteryModbusTCPTableID.None,
    {},
];

type Register = {
    rtype: number; // ModbusRegisterType
    addr: number;
    value: number;
};

type RegisterTable = {
    device_address: number;
    registers: Register[];
};

type TableConfigCustom = [
    BatteryModbusTCPTableID.Custom,
    {
        register_address_mode: number, // ModbusRegisterAddressMode
        permit_grid_charge: RegisterTable,
        revoke_grid_charge_override: RegisterTable,
        forbid_discharge: RegisterTable,
        revoke_discharge_override: RegisterTable,
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
                permit_grid_charge:          {device_address: 1, registers: []},
                revoke_grid_charge_override: {device_address: 1, registers: []},
                forbid_discharge:            {device_address: 1, registers: []},
                revoke_discharge_override:   {device_address: 1, registers: []},
            }];

        default:
            return [BatteryModbusTCPTableID.None, null];
    }
}

interface RegisterEditorProps {
    register_address_mode: ModbusRegisterAddressMode;
    table: RegisterTable;
    on_table: (table: RegisterTable) => void;
}

interface RegisterEditorState {
    register: Register,
}

class RegisterEditor extends Component<RegisterEditorProps, RegisterEditorState> {
    constructor(props: RegisterEditorProps) {
        super(props);

        this.state = {
            register: {
                rtype: null,
                addr: null,
                value: null,
            },
        } as RegisterEditorState;
    }

    get_children() {
        let start_address_offset = this.props.register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1;

        return [
            <FormRow label={__("batteries_modbus_tcp.content.registers_register_type")}>
                <InputSelect
                    required
                    items={[
                        [ModbusRegisterType.HoldingRegister.toString(), __("batteries_modbus_tcp.content.registers_register_type_holding_register")],
                        [ModbusRegisterType.Coil.toString(), __("batteries_modbus_tcp.content.registers_register_type_coil")],
                    ]}
                    placeholder={__("select")}
                    value={util.hasValue(this.state.register.rtype) ? this.state.register.rtype.toString() : undefined}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, rtype: parseInt(v)}});
                    }} />
            </FormRow>,
            <FormRow
                label={
                    this.props.register_address_mode == ModbusRegisterAddressMode.Address
                    ? __("batteries_modbus_tcp.content.registers_start_address")
                    : __("batteries_modbus_tcp.content.registers_start_number")
                }
                label_muted={
                    this.props.register_address_mode == ModbusRegisterAddressMode.Address
                    ? __("batteries_modbus_tcp.content.registers_start_address_muted")
                    : __("batteries_modbus_tcp.content.registers_start_number_muted")
                }>
                <InputNumber
                    required
                    min={start_address_offset}
                    max={start_address_offset + 65535}
                    value={this.state.register.addr + start_address_offset}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, addr: v - start_address_offset}});
                    }} />
            </FormRow>,
            <FormRow label={__("batteries_modbus_tcp.content.registers_value")}>
                <InputAnyFloat
                    required
                    min={0}
                    max={this.state.register.rtype == ModbusRegisterType.Coil ? 1 : 65535}
                    value={this.state.register.value}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, value: v}});
                    }} />
            </FormRow>,
        ];
    }

    get_register_type_name(rtype: number) {
        if (rtype == ModbusRegisterType.HoldingRegister) {
            return __("batteries_modbus_tcp.content.registers_register_type_holding_register_desc");
        }
        else if (rtype == ModbusRegisterType.Coil) {
            return __("batteries_modbus_tcp.content.registers_register_type_coil_desc");
        }
        else {
            return "?";
        }
    }

    render() {
        let start_address_offset = this.props.register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1;

        return <Table
            nestingDepth={1}
            rows={this.props.table.registers.map((register, i) => {
                const row: TableRow = {
                    columnValues: [__("batteries_modbus_tcp.content.registers_value_desc")(this.get_register_type_name(register.rtype), register.addr + start_address_offset, register.value)],
                    onRemoveClick: async () => {
                        this.props.on_table({...this.props.table, registers: this.props.table.registers.filter((r, k) => k !== i)});
                    },
                    onEditShow: async () => {
                        this.setState({
                            register: register,
                        });
                    },
                    onEditSubmit: async () => {
                        this.props.on_table({...this.props.table, registers: this.props.table.registers.map((r, k) => k === i ? this.state.register : r)});
                    },
                    onEditGetChildren: () => this.get_children(),
                    editTitle: __("batteries_modbus_tcp.content.registers_edit_title"),
                }
                return row
            })}
            columnNames={[""]}
            addEnabled={util.hasValue(this.props.register_address_mode) && this.props.table.registers.length < MAX_CUSTOM_REGISTERS}
            addMessage={
                util.hasValue(this.props.register_address_mode)
                    ? __("batteries_modbus_tcp.content.registers_add_count")(this.props.table.registers.length, MAX_CUSTOM_REGISTERS)
                    : __("batteries_modbus_tcp.content.registers_add_select_address_mode")
            }
            addTitle={__("batteries_modbus_tcp.content.registers_add_title")}
            onAddShow={async () => {
                this.setState({
                    register: {
                        rtype: null,
                        addr: null,
                        value: null,
                    },
                });
            }}
            onAddGetChildren={() => this.get_children()}
            onAddSubmit={async () => {
                this.props.on_table({...this.props.table, registers: this.props.table.registers.concat([this.state.register])});
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
                        registers: this.props.table.registers,
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
                    disabled={this.props.table.registers.length == 0 || this.state.waiting}
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

export function init() {
    return {
        [BatteryClassID.ModbusTCP]: {
            name: () => __("batteries_modbus_tcp.content.battery_class"),
            new_config: () => [BatteryClassID.ModbusTCP, {display_name: "", host: "", port: 502, table: null}] as BatteryConfig,
            clone_config: (config: BatteryConfig) => [config[0], {...config[1]}] as BatteryConfig,
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
                            <FormRow label={__("batteries_modbus_tcp.content.registers")}>
                                <RegisterEditor
                                    register_address_mode={config[1].table[1].register_address_mode}
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
                            <FormRow label={__("batteries_modbus_tcp.content.registers")}>
                                <RegisterEditor
                                    register_address_mode={config[1].table[1].register_address_mode}
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
                            <FormRow label={__("batteries_modbus_tcp.content.registers")}>
                                <RegisterEditor
                                    register_address_mode={config[1].table[1].register_address_mode}
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
                            <FormRow label={__("batteries_modbus_tcp.content.registers")}>
                                <RegisterEditor
                                    register_address_mode={config[1].table[1].register_address_mode}
                                    table={config[1].table[1].revoke_discharge_override}
                                    on_table={(table: RegisterTable) => on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {revoke_discharge_override: table})}))} />
                            </FormRow>
                            <Executor host={config[1].host} port={config[1].port} table={config[1].table[1].revoke_discharge_override} />
                        </CollapsedSection>,
                    );
                }

                return edit_children;
            },
        },
    };
}
