/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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
import { h, Fragment, Component, ComponentChildren } from "preact";
import { __ } from "../../ts/translation";
import { MeterClassID } from "../meters/meter_class_id.enum";
import { MeterConfig } from "../meters/types";
import { MeterValueIDSelector, get_meter_value_id_name } from "../meters_api/plugin_meters_config";
import { MeterModbusTCPTableID } from "./meter_modbus_tcp_table_id.enum";
import { ModbusRegisterType } from "./modbus_register_type.enum";
import { ModbusRegisterAddressMode } from "./modbus_register_address_mode.enum";
import { ModbusValueType } from "./modbus_value_type.enum";
import { SungrowHybridInverterVirtualMeterID } from "./sungrow_hybrid_inverter_virtual_meter_id.enum";
import { SungrowStringInverterVirtualMeterID } from "./sungrow_string_inverter_virtual_meter_id.enum";
import { SolarmaxMaxStorageVirtualMeterID } from "./solarmax_max_storage_virtual_meter_id.enum";
import { VictronEnergyGXVirtualMeterID } from "./victron_energy_gx_virtual_meter_id.enum";
import { DeyeHybridInverterVirtualMeterID } from "./deye_hybrid_inverter_virtual_meter_id.enum";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { InputAnyFloat } from "../../ts/components/input_any_float";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { Table, TableRow } from "../../ts/components/table";

const MAX_CUSTOM_REGISTERS = 10;

type TableConfigNone = [
    MeterModbusTCPTableID.None,
    {},
];

type Register = {
    register_type: number; // ModbusRegisterType
    start_address: number;
    value_type: number; // ModbusValueType
    offset: number;
    scale_factor: number;
    value_id: number; // MeterValueID
};

type TableConfigCustom = [
    MeterModbusTCPTableID.Custom,
    {
        device_address: number;
        register_address_mode: number; // ModbusRegisterAddressMode
        registers: Register[];
    },
];

type TableConfigSungrowHybridInverter = [
    MeterModbusTCPTableID.SungrowHybridInverter,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigSungrowStringInverter = [
    MeterModbusTCPTableID.SungrowStringInverter,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigSolarmaxMaxStorage = [
    MeterModbusTCPTableID.SolarmaxMaxStorage,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigVictronEnergyGX = [
    MeterModbusTCPTableID.VictronEnergyGX,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigDeyeHybridInverter = [
    MeterModbusTCPTableID.DeyeHybridInverter,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfig = TableConfigNone |
                   TableConfigCustom |
                   TableConfigSungrowHybridInverter |
                   TableConfigSungrowStringInverter |
                   TableConfigSolarmaxMaxStorage |
                   TableConfigVictronEnergyGX |
                   TableConfigDeyeHybridInverter;

export type ModbusTCPMetersConfig = [
    MeterClassID.ModbusTCP,
    {
        display_name: string;
        host: string;
        port: number;
        table: TableConfig;
    },
];

function new_table_config(table: MeterModbusTCPTableID): TableConfig {
    switch (table) {
        case MeterModbusTCPTableID.Custom:
            return [MeterModbusTCPTableID.Custom, {device_address: 1, register_address_mode: null, registers: []}];

        case MeterModbusTCPTableID.SungrowHybridInverter:
            return [MeterModbusTCPTableID.SungrowHybridInverter, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.SungrowStringInverter:
            return [MeterModbusTCPTableID.SungrowStringInverter, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.SolarmaxMaxStorage:
            return [MeterModbusTCPTableID.SolarmaxMaxStorage, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.VictronEnergyGX:
            return [MeterModbusTCPTableID.VictronEnergyGX, {virtual_meter: null, device_address: 100}];

        case MeterModbusTCPTableID.DeyeHybridInverter:
            return [MeterModbusTCPTableID.DeyeHybridInverter, {virtual_meter: null, device_address: 1}];

        default:
            return [MeterModbusTCPTableID.None, {}];
    }
}

interface RegisterEditorProps {
    table: TableConfigCustom;
    on_table: (table: TableConfigCustom) => void;
}

interface RegisterEditorState {
    register: Register,
}

class RegisterTable extends Component<RegisterEditorProps, RegisterEditorState> {
    constructor(props: RegisterEditorProps) {
        super(props);

        this.state = {
            register: {
                register_type: null,
                start_address: props.table[1].register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1,
                value_type: null,
                offset: 0.0,
                scale_factor: 1.0,
                value_id: null,
            },
        } as any;
    }

    get_children() {
        return [
            <FormRow label={__("meters_modbus_tcp.content.registers_register_type")}>
                <InputSelect
                    required
                    items={[
                        [ModbusRegisterType.HoldingRegister.toString(), __("meters_modbus_tcp.content.registers_register_type_holding_register")],
                        [ModbusRegisterType.InputRegister.toString(), __("meters_modbus_tcp.content.registers_register_type_input_register")],
                    ]}
                    placeholder={__("meters_modbus_tcp.content.registers_register_type_select")}
                    value={util.hasValue(this.state.register.register_type) ? this.state.register.register_type.toString() : undefined}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, register_type: parseInt(v)}});
                    }} />
            </FormRow>,
            <FormRow
                label={
                    this.props.table[1].register_address_mode == ModbusRegisterAddressMode.Address
                    ? __("meters_modbus_tcp.content.registers_start_address")
                    : __("meters_modbus_tcp.content.registers_start_number")
                }
                label_muted={
                    this.props.table[1].register_address_mode == ModbusRegisterAddressMode.Address
                    ? __("meters_modbus_tcp.content.registers_start_address_muted")
                    : __("meters_modbus_tcp.content.registers_start_number_muted")
                }>
                <InputNumber
                    required
                    min={this.props.table[1].register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1}
                    max={(this.props.table[1].register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1) + 65535}
                    value={this.state.register.start_address}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, start_address: v}});
                    }} />
            </FormRow>,
            <FormRow label={__("meters_modbus_tcp.content.registers_value_type")}>
                <InputSelect
                    required
                    items={[
                        [ModbusValueType.U16.toString(), __("meters_modbus_tcp.content.registers_value_type_u16")],
                        [ModbusValueType.S16.toString(), __("meters_modbus_tcp.content.registers_value_type_s16")],
                        [ModbusValueType.U32BE.toString(), __("meters_modbus_tcp.content.registers_value_type_u32be")],
                        [ModbusValueType.U32LE.toString(), __("meters_modbus_tcp.content.registers_value_type_u32le")],
                        [ModbusValueType.S32BE.toString(), __("meters_modbus_tcp.content.registers_value_type_s32be")],
                        [ModbusValueType.S32LE.toString(), __("meters_modbus_tcp.content.registers_value_type_s32le")],
                        [ModbusValueType.F32BE.toString(), __("meters_modbus_tcp.content.registers_value_type_f32be")],
                        [ModbusValueType.F32LE.toString(), __("meters_modbus_tcp.content.registers_value_type_f32le")],
                        [ModbusValueType.U64BE.toString(), __("meters_modbus_tcp.content.registers_value_type_u64be")],
                        [ModbusValueType.U64LE.toString(), __("meters_modbus_tcp.content.registers_value_type_u64le")],
                        [ModbusValueType.S64BE.toString(), __("meters_modbus_tcp.content.registers_value_type_s64be")],
                        [ModbusValueType.S64LE.toString(), __("meters_modbus_tcp.content.registers_value_type_s64le")],
                        [ModbusValueType.F64BE.toString(), __("meters_modbus_tcp.content.registers_value_type_f64be")],
                        [ModbusValueType.F64LE.toString(), __("meters_modbus_tcp.content.registers_value_type_f64le")],
                    ]}
                    placeholder={__("meters_modbus_tcp.content.registers_value_type_select")}
                    value={util.hasValue(this.state.register.value_type) ? this.state.register.value_type.toString() : undefined}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, value_type: parseInt(v)}});
                    }} />
            </FormRow>,
            <FormRow label={__("meters_modbus_tcp.content.registers_offset")}>
                <InputAnyFloat
                    required
                    value={this.state.register.offset}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, offset: v}});
                    }} />
            </FormRow>,
            <FormRow label={__("meters_modbus_tcp.content.registers_scale_factor")}>
                <InputAnyFloat
                    required
                    value={this.state.register.scale_factor}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, scale_factor: v}});
                    }} />
            </FormRow>,
            <FormRow label={__("meters_modbus_tcp.content.registers_value_id")}>
                <MeterValueIDSelector value_id={this.state.register.value_id} value_ids={[]} on_value_id={
                    (v) => this.setState({register: {...this.state.register, value_id: v}})
                } />
            </FormRow>,
        ];
    }

    render() {
        return <Table
            nestingDepth={1}
            rows={this.props.table[1].registers.map((register, i) => {
                const row: TableRow = {
                    columnValues: [__("meters_modbus_tcp.content.registers_register")(register.start_address, get_meter_value_id_name(register.value_id))],
                    onRemoveClick: async () => {
                        this.props.on_table(util.get_updated_union(this.props.table, {registers: this.props.table[1].registers.filter((r, k) => k !== i)}));
                    },
                    onEditShow: async () => {
                        this.setState({
                            register: register,
                        });
                    },
                    onEditSubmit: async () => {
                        this.props.on_table(util.get_updated_union(this.props.table, {registers: this.props.table[1].registers.map((r, k) => k === i ? this.state.register : r)}));
                    },
                    onEditGetChildren: () => this.get_children(),
                    editTitle: __("meters_modbus_tcp.content.registers_edit_title"),
                }
                return row
            })}
            columnNames={[""]}
            addEnabled={util.hasValue(this.props.table[1].register_address_mode) && this.props.table[1].registers.length < MAX_CUSTOM_REGISTERS}
            addMessage={
                util.hasValue(this.props.table[1].register_address_mode)
                    ? __("meters_modbus_tcp.content.registers_add_count")(this.props.table[1].registers.length, MAX_CUSTOM_REGISTERS)
                    : __("meters_modbus_tcp.content.registers_add_select_address_mode")
            }
            addTitle={__("meters_modbus_tcp.content.registers_add_title")}
            onAddShow={async () => {
                this.setState({
                    register: {
                        register_type: null,
                        start_address: this.props.table[1].register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1,
                        value_type: null,
                        offset: 0.0,
                        scale_factor: 1.0,
                        value_id: null,
                    },
                });
            }}
            onAddGetChildren={() => this.get_children()}
            onAddSubmit={async () => {
                this.props.on_table(util.get_updated_union(this.props.table, {registers: this.props.table[1].registers.concat([this.state.register])}));
            }}/>;
    }
}

export function init() {
    return {
        [MeterClassID.ModbusTCP]: {
            name: __("meters_modbus_tcp.content.meter_class"),
            new_config: () => [MeterClassID.ModbusTCP, {display_name: "", host: "", port: 502, table: null}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: ModbusTCPMetersConfig, on_config: (config: ModbusTCPMetersConfig) => void): ComponentChildren => {
                let edit_children = [
                    <FormRow label={__("meters_modbus_tcp.content.display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {display_name: v}));
                            }} />
                    </FormRow>,
                    <FormRow label={__("meters_modbus_tcp.content.host")}>
                        <InputText
                            required
                            maxLength={64}
                            pattern="^[a-zA-Z0-9\-\.]+$"
                            value={config[1].host}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {host: v}));
                            }}
                            invalidFeedback={__("meters_modbus_tcp.content.host_invalid")} />
                    </FormRow>,
                    <FormRow label={__("meters_modbus_tcp.content.port")} label_muted={__("meters_modbus_tcp.content.port_muted")}>
                        <InputNumber
                            required
                            min={1}
                            max={65535}
                            value={config[1].port}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {port: v}));
                            }} />
                    </FormRow>,
                    <FormRow label={__("meters_modbus_tcp.content.table")}>
                        <InputSelect
                            required
                            items={[
                                [MeterModbusTCPTableID.Custom.toString(), __("meters_modbus_tcp.content.table_custom")],
                                [MeterModbusTCPTableID.SungrowHybridInverter.toString(), __("meters_modbus_tcp.content.table_sungrow_hybrid_inverter")],
                                [MeterModbusTCPTableID.SungrowStringInverter.toString(), __("meters_modbus_tcp.content.table_sungrow_string_inverter")],
                                [MeterModbusTCPTableID.SolarmaxMaxStorage.toString(), __("meters_modbus_tcp.content.table_solarmax_max_storage")],
                                [MeterModbusTCPTableID.VictronEnergyGX.toString(), __("meters_modbus_tcp.content.table_victron_energy_gx")],
                                [MeterModbusTCPTableID.DeyeHybridInverter.toString(), __("meters_modbus_tcp.content.table_deye_hybrid_inverter")],
                            ]}
                            placeholder={__("meters_modbus_tcp.content.table_select")}
                            value={util.hasValue(config[1].table) ? config[1].table[0].toString() : undefined}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {table: new_table_config(parseInt(v))}));
                            }} />
                    </FormRow>,
                ];

                if (util.hasValue(config[1].table)
                 && (config[1].table[0] == MeterModbusTCPTableID.SungrowHybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.SungrowStringInverter
                  || config[1].table[0] == MeterModbusTCPTableID.SolarmaxMaxStorage
                  || config[1].table[0] == MeterModbusTCPTableID.VictronEnergyGX
                  || config[1].table[0] == MeterModbusTCPTableID.DeyeHybridInverter)) {
                    let items: [string, string][] = [];
                    let device_address_default: number = 1;

                    if (config[1].table[0] == MeterModbusTCPTableID.SungrowHybridInverter) {
                        items = [
                            [SungrowHybridInverterVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [SungrowHybridInverterVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SungrowHybridInverterVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [SungrowHybridInverterVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SungrowStringInverter) {
                        items = [
                            [SungrowStringInverterVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [SungrowStringInverterVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SungrowStringInverterVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SolarmaxMaxStorage) {
                        items = [
                            [SolarmaxMaxStorageVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [SolarmaxMaxStorageVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SolarmaxMaxStorageVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.VictronEnergyGX) {
                        items = [
                            [VictronEnergyGXVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [VictronEnergyGXVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [VictronEnergyGXVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [VictronEnergyGXVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];

                        device_address_default = 100;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.DeyeHybridInverter) {
                        items = [
                            [DeyeHybridInverterVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [DeyeHybridInverterVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [DeyeHybridInverterVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [DeyeHybridInverterVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];
                    }

                    edit_children.push(
                        <FormRow label={__("meters_modbus_tcp.content.virtual_meter")}>
                            <InputSelect
                                required
                                items={items}
                                placeholder={__("meters_modbus_tcp.content.virtual_meter_select")}
                                value={util.hasValue(config[1].table[1]) && util.hasValue(config[1].table[1].virtual_meter) ? config[1].table[1].virtual_meter.toString() : undefined}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {virtual_meter: parseInt(v)})}));
                                }} />
                        </FormRow>,
                        <FormRow label={__("meters_modbus_tcp.content.device_address")} label_muted={__("meters_modbus_tcp.content.device_address_muted")(device_address_default)}>
                            <InputNumber
                                required
                                min={1}
                                max={247}
                                value={config[1].table[1].device_address}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {device_address: v})}));
                                }} />
                        </FormRow>);

                }
                else if (util.hasValue(config[1].table)
                      && config[1].table[0] == MeterModbusTCPTableID.Custom) {
                    edit_children.push(
                        <FormRow label={__("meters_modbus_tcp.content.device_address")}>
                            <InputNumber
                                required
                                min={1}
                                max={247}
                                value={config[1].table[1].device_address}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {device_address: v})}));
                                }} />
                        </FormRow>,
                        <FormRow label={__("meters_modbus_tcp.content.register_address_mode")}>
                            <InputSelect
                                required
                                items={[
                                    [ModbusRegisterAddressMode.Address.toString(), __("meters_modbus_tcp.content.register_address_mode_address")],
                                    [ModbusRegisterAddressMode.Number.toString(), __("meters_modbus_tcp.content.register_address_mode_number")]
                                ]}
                                placeholder={__("meters_modbus_tcp.content.register_address_mode_select")}
                                value={util.hasValue(config[1].table[1].register_address_mode) ? config[1].table[1].register_address_mode.toString() : undefined}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {register_address_mode: parseInt(v)})}));
                                }} />
                        </FormRow>,
                        <FormRow label={__("meters_modbus_tcp.content.registers")}>
                            <RegisterTable
                                table={config[1].table}
                                on_table={(table: TableConfigCustom) => on_config(util.get_updated_union(config, {table: table}))} />
                        </FormRow>
                    );
                }

                return edit_children;
            },
        },
    };
}
