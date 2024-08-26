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
import { SungrowHybridInverterVirtualMeter } from "./sungrow_hybrid_inverter_virtual_meter.enum";
import { SungrowStringInverterVirtualMeter } from "./sungrow_string_inverter_virtual_meter.enum";
import { SolarmaxMaxStorageVirtualMeter } from "./solarmax_max_storage_virtual_meter.enum";
import { VictronEnergyGXVirtualMeter } from "./victron_energy_gx_virtual_meter.enum";
import { DeyeHybridInverterVirtualMeter } from "./deye_hybrid_inverter_virtual_meter.enum";
import { AlphaESSHybridInverterVirtualMeter } from "./alpha_ess_hybrid_inverter_virtual_meter.enum";
import { ShellyPro3EMDeviceProfile } from "./shelly_pro_3em_device_profile.enum";
import { ShellyEMMonophaseChannel } from "./shelly_em_monophase_channel.enum";
import { ShellyEMMonophaseMapping } from "./shelly_em_monophase_mapping.enum";
import { GoodweHybridInverterVirtualMeter } from "./goodwe_hybrid_inverter_virtual_meter.enum";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { InputAnyFloat } from "../../ts/components/input_any_float";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { Table, TableRow } from "../../ts/components/table";

const MAX_CUSTOM_REGISTERS = 36;

type TableConfigNone = [
    MeterModbusTCPTableID.None,
    {},
];

type Register = {
    rtype: number; // ModbusRegisterType
    addr: number;
    vtype: number; // ModbusValueType
    off: number;
    scale: number;
    id: number; // MeterValueID
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

type TableConfigAlphaESSHybridInverter = [
    MeterModbusTCPTableID.AlphaESSHybridInverter,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigShellyProEM = [
    MeterModbusTCPTableID.ShellyProEM,
    {
        device_address: number;
        monophase_channel: number;
        monophase_mapping: number;
    },
];

type TableConfigShellyPro3EM = [
    MeterModbusTCPTableID.ShellyPro3EM,
    {
        device_address: number;
        device_profile: number;
        monophase_channel: number;
        monophase_mapping: number;
    },
];

type TableConfigGoodweHybridInverter = [
    MeterModbusTCPTableID.GoodweHybridInverter,
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
                   TableConfigDeyeHybridInverter |
                   TableConfigAlphaESSHybridInverter |
                   TableConfigShellyProEM |
                   TableConfigShellyPro3EM |
                   TableConfigGoodweHybridInverter;

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

        case MeterModbusTCPTableID.AlphaESSHybridInverter:
            return [MeterModbusTCPTableID.AlphaESSHybridInverter, {virtual_meter: null, device_address: 85}];

        case MeterModbusTCPTableID.ShellyProEM:
            return [MeterModbusTCPTableID.ShellyProEM, {device_address: 1, monophase_channel: null, monophase_mapping: null}];

        case MeterModbusTCPTableID.ShellyPro3EM:
            return [MeterModbusTCPTableID.ShellyPro3EM, {device_address: 1, device_profile: ShellyPro3EMDeviceProfile.Triphase, monophase_channel: ShellyEMMonophaseChannel.None, monophase_mapping: ShellyEMMonophaseMapping.None}];

        case MeterModbusTCPTableID.GoodweHybridInverter:
            return [MeterModbusTCPTableID.GoodweHybridInverter, {virtual_meter: null, device_address: 247}];

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
                rtype: null,
                addr: 0,
                vtype: null,
                off: 0.0,
                scale: 1.0,
                id: null,
            },
        } as RegisterEditorState;
    }

    get_children() {
        let start_address_offset = this.props.table[1].register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1;

        return [
            <FormRow label={__("meters_modbus_tcp.content.registers_register_type")}>
                <InputSelect
                    required
                    items={[
                        [ModbusRegisterType.HoldingRegister.toString(), __("meters_modbus_tcp.content.registers_register_type_holding_register")],
                        [ModbusRegisterType.InputRegister.toString(), __("meters_modbus_tcp.content.registers_register_type_input_register")],
                    ]}
                    placeholder={__("meters_modbus_tcp.content.registers_register_type_select")}
                    value={util.hasValue(this.state.register.rtype) ? this.state.register.rtype.toString() : undefined}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, rtype: parseInt(v)}});
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
                    min={start_address_offset}
                    max={start_address_offset + 65535}
                    value={this.state.register.addr + start_address_offset}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, addr: v - start_address_offset}});
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
                    value={util.hasValue(this.state.register.vtype) ? this.state.register.vtype.toString() : undefined}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, vtype: parseInt(v)}});
                    }} />
            </FormRow>,
            <FormRow label={__("meters_modbus_tcp.content.registers_offset")}>
                <InputAnyFloat
                    required
                    value={this.state.register.off}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, off: v}});
                    }} />
            </FormRow>,
            <FormRow label={__("meters_modbus_tcp.content.registers_scale_factor")}>
                <InputAnyFloat
                    required
                    value={this.state.register.scale}
                    onValue={(v) => {
                        this.setState({register: {...this.state.register, scale: v}});
                    }} />
            </FormRow>,
            <FormRow label={__("meters_modbus_tcp.content.registers_value_id")}>
                <MeterValueIDSelector value_id={this.state.register.id} value_ids={[]} on_value_id={
                    (v) => this.setState({register: {...this.state.register, id: v}})
                } />
            </FormRow>,
        ];
    }

    render() {
        let start_address_offset = this.props.table[1].register_address_mode == ModbusRegisterAddressMode.Address ? 0 : 1;

        return <Table
            nestingDepth={1}
            rows={this.props.table[1].registers.map((register, i) => {
                const row: TableRow = {
                    columnValues: [__("meters_modbus_tcp.content.registers_register")(register.addr + start_address_offset, get_meter_value_id_name(register.id))],
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
                        rtype: null,
                        addr: 0,
                        vtype: null,
                        off: 0.0,
                        scale: 1.0,
                        id: null,
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
                                [MeterModbusTCPTableID.AlphaESSHybridInverter.toString(), __("meters_modbus_tcp.content.table_alpha_ess_hybrid_inverter")],
                                [MeterModbusTCPTableID.ShellyProEM.toString(), __("meters_modbus_tcp.content.table_shelly_pro_em")],
                                [MeterModbusTCPTableID.ShellyPro3EM.toString(), __("meters_modbus_tcp.content.table_shelly_pro_3em")],
                                [MeterModbusTCPTableID.GoodweHybridInverter.toString(), __("meters_modbus_tcp.content.table_goodwe_hybrid_inverter")],
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
                  || config[1].table[0] == MeterModbusTCPTableID.DeyeHybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.AlphaESSHybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.ShellyProEM
                  || config[1].table[0] == MeterModbusTCPTableID.ShellyPro3EM
                  || config[1].table[0] == MeterModbusTCPTableID.GoodweHybridInverter)) {
                    let virtual_meter_items: [string, string][] = [];
                    let device_address_default: number = 1;

                    if (config[1].table[0] == MeterModbusTCPTableID.SungrowHybridInverter) {
                        virtual_meter_items = [
                            [SungrowHybridInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [SungrowHybridInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SungrowHybridInverterVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [SungrowHybridInverterVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SungrowStringInverter) {
                        virtual_meter_items = [
                            [SungrowStringInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [SungrowStringInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SungrowStringInverterVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SolarmaxMaxStorage) {
                        virtual_meter_items = [
                            [SolarmaxMaxStorageVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [SolarmaxMaxStorageVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SolarmaxMaxStorageVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.VictronEnergyGX) {
                        virtual_meter_items = [
                            [VictronEnergyGXVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [VictronEnergyGXVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [VictronEnergyGXVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [VictronEnergyGXVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];

                        device_address_default = 100;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.DeyeHybridInverter) {
                        virtual_meter_items = [
                            [DeyeHybridInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [DeyeHybridInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [DeyeHybridInverterVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [DeyeHybridInverterVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.AlphaESSHybridInverter) {
                        virtual_meter_items = [
                            [AlphaESSHybridInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [AlphaESSHybridInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [AlphaESSHybridInverterVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                        ];

                        device_address_default = 85;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.GoodweHybridInverter) {
                        virtual_meter_items = [
                            [GoodweHybridInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [GoodweHybridInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [GoodweHybridInverterVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [GoodweHybridInverterVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                            [GoodweHybridInverterVirtualMeter.BackupLoad.toString(), __("meters_modbus_tcp.content.virtual_meter_backup_load")],
                            [GoodweHybridInverterVirtualMeter.Meter.toString(), __("meters_modbus_tcp.content.virtual_meter_meter")],
                        ];

                        device_address_default = 247;
                    }

                    if (virtual_meter_items.length > 0) {
                        edit_children.push(
                            <FormRow label={__("meters_modbus_tcp.content.virtual_meter")}>
                                <InputSelect
                                    required
                                    items={virtual_meter_items}
                                    placeholder={__("meters_modbus_tcp.content.virtual_meter_select")}
                                    value={util.hasValue(config[1].table[1]) && util.hasValue((config[1].table[1] as any).virtual_meter) ? (config[1].table[1] as any).virtual_meter.toString() : undefined}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {virtual_meter: parseInt(v)})}));
                                    }} />
                            </FormRow>);
                    }

                    edit_children.push(
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

                    if (config[1].table[0] == MeterModbusTCPTableID.ShellyPro3EM) {
                        edit_children.push(
                            <FormRow label={__("meters_modbus_tcp.content.shelly_pro_3em_device_profile")}>
                                <InputSelect
                                    required
                                    items={[
                                        [ShellyPro3EMDeviceProfile.Triphase.toString(), __("meters_modbus_tcp.content.shelly_pro_3em_device_profile_triphase")],
                                        [ShellyPro3EMDeviceProfile.Monophase.toString(), __("meters_modbus_tcp.content.shelly_pro_3em_device_profile_monophase")],
                                    ]}
                                    value={util.hasValue(config[1].table[1]) && util.hasValue((config[1].table[1] as any).device_profile) ? (config[1].table[1] as any).device_profile.toString() : undefined}
                                    onValue={(v) => {
                                        let device_profile = parseInt(v);
                                        let monophase_channel = (config[1].table[1] as any).monophase_channel;
                                        let monophase_mapping = (config[1].table[1] as any).monophase_mapping;

                                        if (device_profile == ShellyPro3EMDeviceProfile.Triphase) {
                                            monophase_channel = ShellyEMMonophaseChannel.None;
                                            monophase_mapping = ShellyEMMonophaseMapping.None;
                                        }

                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {device_profile: device_profile, monophase_channel: monophase_channel, monophase_mapping: monophase_mapping})}));
                                    }} />
                            </FormRow>);
                    }

                    if (config[1].table[0] == MeterModbusTCPTableID.ShellyProEM
                     || (config[1].table[0] == MeterModbusTCPTableID.ShellyPro3EM
                      && (config[1].table[1] as any).device_profile == ShellyPro3EMDeviceProfile.Monophase)) {
                        let monophase_channel_items: [string, string][] = [
                            [ShellyEMMonophaseChannel.First.toString(), __("meters_modbus_tcp.content.shelly_em_monophase_channel_1")],
                            [ShellyEMMonophaseChannel.Second.toString(), __("meters_modbus_tcp.content.shelly_em_monophase_channel_2")],
                        ];

                        if (config[1].table[0] == MeterModbusTCPTableID.ShellyPro3EM) {
                            monophase_channel_items.push([ShellyEMMonophaseChannel.Third.toString(), __("meters_modbus_tcp.content.shelly_em_monophase_channel_3")]);
                        }

                        edit_children.push(
                            <FormRow label={__("meters_modbus_tcp.content.shelly_em_monophase_channel")}>
                                <InputSelect
                                    required
                                    items={monophase_channel_items}
                                    placeholder={__("meters_modbus_tcp.content.shelly_em_monophase_channel_select")}
                                    value={util.hasValue(config[1].table[1]) && util.hasValue((config[1].table[1] as any).monophase_channel) ? (config[1].table[1] as any).monophase_channel.toString() : undefined}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {monophase_channel: parseInt(v)})}));
                                    }} />
                            </FormRow>,
                            <FormRow label={__("meters_modbus_tcp.content.shelly_em_monophase_mapping")}>
                                <InputSelect
                                    required
                                    items={[
                                        [ShellyEMMonophaseMapping.L1.toString(), __("meters_modbus_tcp.content.shelly_em_monophase_mapping_l1")],
                                        [ShellyEMMonophaseMapping.L2.toString(), __("meters_modbus_tcp.content.shelly_em_monophase_mapping_l2")],
                                        [ShellyEMMonophaseMapping.L3.toString(), __("meters_modbus_tcp.content.shelly_em_monophase_mapping_l3")],
                                    ]}
                                    placeholder={__("meters_modbus_tcp.content.shelly_em_monophase_mapping_select")}
                                    value={util.hasValue(config[1].table[1]) && util.hasValue((config[1].table[1] as any).monophase_mapping) ? (config[1].table[1] as any).monophase_mapping.toString() : undefined}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {monophase_mapping: parseInt(v)})}));
                                    }} />
                            </FormRow>);
                    }
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
