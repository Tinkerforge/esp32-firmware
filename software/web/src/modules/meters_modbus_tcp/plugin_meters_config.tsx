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

//#include "../../options.inc"

import * as options from "../../options";
import * as util from "../../ts/util";
import { h, Fragment, Component, ComponentChildren } from "preact";
import { __ } from "../../ts/translation";
import { MeterClassID } from "../meters/meter_class_id.enum";
import { MeterLocation } from "../meters/meter_location.enum";
import { get_meter_location_items } from "../meters/meter_location";
import { MeterConfig } from "../meters/types";
import { MeterValueIDSelector, get_meter_value_id_name } from "../meters_api/plugin_meters_config";
import { MeterModbusTCPTableID } from "./meter_modbus_tcp_table_id.enum";
import { TableConfig, TableConfigCustom, Register, get_default_device_address, new_table_config, get_virtual_meter_items, get_default_location } from "./meter_modbus_tcp_specs";
import { ModbusRegisterType } from "../modbus_tcp_client/modbus_register_type.enum";
import { ModbusRegisterAddressMode } from "../modbus_tcp_client/modbus_register_address_mode.enum";
import { ModbusValueType } from "../modbus_tcp_client/modbus_value_type.enum";
import { ShellyPro3EMDeviceProfile } from "./shelly_pro_3em_device_profile.enum";
import { ShellyEMMonophaseChannel } from "./shelly_em_monophase_channel.enum";
import { ShellyEMMonophaseMapping } from "./shelly_em_monophase_mapping.enum";
import { CarloGavazziPhase } from "./carlo_gavazzi_phase.enum";
import { InputText } from "../../ts/components/input_text";
import { InputHost } from "../../ts/components/input_host";
import { InputNumber } from "../../ts/components/input_number";
import { InputAnyFloat } from "../../ts/components/input_any_float";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { SwitchableInputSelect } from "../../ts/components/switchable_input_select";
import { Table, TableRow } from "../../ts/components/table";

export type ModbusTCPMetersConfig = [
    MeterClassID.ModbusTCP,
    {
        display_name: string;
        location: number;
        host: string;
        port: number;
        table: TableConfig;
    },
];

interface RegisterEditorProps {
    table: TableConfigCustom;
    on_table: (table: TableConfigCustom) => void;
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
                    placeholder={__("select")}
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
                    placeholder={__("select")}
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
                let register_type = "<unknown>";

                switch (register.rtype) {
                case ModbusRegisterType.HoldingRegister:
                    register_type = __("meters_modbus_tcp.content.registers_register_type_holding_register");
                    break;

                case ModbusRegisterType.InputRegister:
                    register_type = __("meters_modbus_tcp.content.registers_register_type_input_register");
                    break;
                }

                const row: TableRow = {
                    columnValues: [__("meters_modbus_tcp.content.registers_register")(register_type, register.addr + start_address_offset, get_meter_value_id_name(register.id))],
                    onRemoveClick: async () => {
                        this.props.on_table(util.get_updated_union(this.props.table, {registers: this.props.table[1].registers.filter((r, k) => k !== i)}));
                        return true;
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
            addEnabled={util.hasValue(this.props.table[1].register_address_mode) && this.props.table[1].registers.length < options.METERS_MODBUS_TCP_MAX_CUSTOM_REGISTERS}
            addMessage={
                util.hasValue(this.props.table[1].register_address_mode)
                    ? __("meters_modbus_tcp.content.registers_add_message")(this.props.table[1].registers.length, options.METERS_MODBUS_TCP_MAX_CUSTOM_REGISTERS)
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
            name: () => __("meters_modbus_tcp.content.meter_class"),
            new_config: () => [MeterClassID.ModbusTCP, {display_name: "", location: MeterLocation.Unknown, host: "", port: 502, table: null}] as MeterConfig,
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
                        <InputHost
                            required
                            value={config[1].host}
                            onValue={(v) => on_config(util.get_updated_union(config, {host: v}))} />
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
                                // Keep alphabetically sorted
                                [MeterModbusTCPTableID.AlphaESSHybridInverter.toString(), __("meters_modbus_tcp.content.table_alpha_ess_hybrid_inverter")],
                                [MeterModbusTCPTableID.CarloGavazziEM24DIN.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em24_din")],
                                [MeterModbusTCPTableID.CarloGavazziEM24E1.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em24_e1")],
                                [MeterModbusTCPTableID.CarloGavazziEM100.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em100")],
                                [MeterModbusTCPTableID.CarloGavazziET100.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_et100")],
                                [MeterModbusTCPTableID.CarloGavazziEM210.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em210")],
                                [MeterModbusTCPTableID.CarloGavazziEM270.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em270")],
                                [MeterModbusTCPTableID.CarloGavazziEM280.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em280")],
                                [MeterModbusTCPTableID.CarloGavazziEM300.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em300")],
                                [MeterModbusTCPTableID.CarloGavazziET300.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_et300")],
                                [MeterModbusTCPTableID.CarloGavazziEM510.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em510")],
                                [MeterModbusTCPTableID.CarloGavazziEM530.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em530")],
                                [MeterModbusTCPTableID.CarloGavazziEM540.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em540")],
                                [MeterModbusTCPTableID.CarloGavazziEM580.toString(), __("meters_modbus_tcp.content.table_carlo_gavazzi_em580")],
                                [MeterModbusTCPTableID.ChisageESSHybridInverter.toString(), __("meters_modbus_tcp.content.table_chisage_ess_hybrid_inverter")],
                                [MeterModbusTCPTableID.DeyeHybridInverter.toString(), __("meters_modbus_tcp.content.table_deye_hybrid_inverter")],
                                [MeterModbusTCPTableID.E3DCHauskraftwerk.toString(), __("meters_modbus_tcp.content.table_e3dc_hauskraftwerk")],
                                [MeterModbusTCPTableID.EastronSDM630TCP.toString(), __("meters_modbus_tcp.content.table_eastron_sdm630_tcp")],
//#if OPTIONS_PRODUCT_ID_IS_ELTAKO
                                [MeterModbusTCPTableID.TinkerforgeWARPCharger.toString(), __("meters_modbus_tcp.content.table_tinkerforge_warp_charger")],
//#endif
                                [MeterModbusTCPTableID.FoxESSH3AC3HybridInverter.toString(), __("meters_modbus_tcp.content.table_fox_ess_h3_ac3_hybrid_inverter")],
                                [MeterModbusTCPTableID.FoxESSH3SmartHybridInverter.toString(), __("meters_modbus_tcp.content.table_fox_ess_h3_smart_hybrid_inverter")],
                                [MeterModbusTCPTableID.FoxESSH3ProHybridInverter.toString(), __("meters_modbus_tcp.content.table_fox_ess_h3_pro_hybrid_inverter")],
                                [MeterModbusTCPTableID.FroniusGEN24Plus.toString(), __("meters_modbus_tcp.content.table_fronius_gen24_plus")],
                                [MeterModbusTCPTableID.GoodweHybridInverter.toString(), __("meters_modbus_tcp.content.table_goodwe_hybrid_inverter")],
                                [MeterModbusTCPTableID.HaileiHybridInverter.toString(), __("meters_modbus_tcp.content.table_hailei_hybrid_inverter")],
                                [MeterModbusTCPTableID.HuaweiEMMA.toString(), __("meters_modbus_tcp.content.table_huawei_emma")],
                                [MeterModbusTCPTableID.HuaweiSmartLogger3000.toString(), __("meters_modbus_tcp.content.table_huawei_smart_logger_3000")],
                                [MeterModbusTCPTableID.HuaweiSUN2000.toString(), __("meters_modbus_tcp.content.table_huawei_sun2000")],
                                [MeterModbusTCPTableID.HuaweiSUN2000SmartDongle.toString(), __("meters_modbus_tcp.content.table_huawei_sun2000_smart_dongle")],
                                [MeterModbusTCPTableID.Janitza.toString(), __("meters_modbus_tcp.content.table_janitza")],
                                [MeterModbusTCPTableID.SAXPowerHomeBasicMode.toString(), __("meters_modbus_tcp.content.table_sax_power_home_basic_mode")],
                                [MeterModbusTCPTableID.SAXPowerHomeExtendedMode.toString(), __("meters_modbus_tcp.content.table_sax_power_home_extended_mode")],
                                [MeterModbusTCPTableID.ShellyProEM.toString(), __("meters_modbus_tcp.content.table_shelly_pro_em")],
                                [MeterModbusTCPTableID.ShellyPro3EM.toString(), __("meters_modbus_tcp.content.table_shelly_pro_3em")],
                                [MeterModbusTCPTableID.SiemensPAC2200.toString(), __("meters_modbus_tcp.content.table_siemens_pac2200")],
                                [MeterModbusTCPTableID.SiemensPAC3120.toString(), __("meters_modbus_tcp.content.table_siemens_pac3120")],
                                [MeterModbusTCPTableID.SiemensPAC3200.toString(), __("meters_modbus_tcp.content.table_siemens_pac3200")],
                                [MeterModbusTCPTableID.SiemensPAC3220.toString(), __("meters_modbus_tcp.content.table_siemens_pac3220")],
                                [MeterModbusTCPTableID.SiemensPAC4200.toString(), __("meters_modbus_tcp.content.table_siemens_pac4200")],
                                [MeterModbusTCPTableID.SiemensPAC4220.toString(), __("meters_modbus_tcp.content.table_siemens_pac4220")],
                                [MeterModbusTCPTableID.SMAHybridInverter.toString(), __("meters_modbus_tcp.content.table_sma_hybrid_inverter")],
                                [MeterModbusTCPTableID.SolaredgeInverter.toString(), __("meters_modbus_tcp.content.table_solaredge_inverter")],
                                [MeterModbusTCPTableID.SolarmaxMaxStorage.toString(), __("meters_modbus_tcp.content.table_solarmax_max_storage")],
                                [MeterModbusTCPTableID.SolaxHybridInverter.toString(), __("meters_modbus_tcp.content.table_solax_hybrid_inverter")],
                                [MeterModbusTCPTableID.SolaxStringInverter.toString(), __("meters_modbus_tcp.content.table_solax_string_inverter")],
                                [MeterModbusTCPTableID.SungrowHybridInverter.toString(), __("meters_modbus_tcp.content.table_sungrow_hybrid_inverter")],
                                [MeterModbusTCPTableID.SungrowStringInverter.toString(), __("meters_modbus_tcp.content.table_sungrow_string_inverter")],
//#ifn OPTIONS_PRODUCT_ID_IS_ELTAKO
                                [MeterModbusTCPTableID.TinkerforgeWARPCharger.toString(), __("meters_modbus_tcp.content.table_tinkerforge_warp_charger")],
//#endif
                                [MeterModbusTCPTableID.VARTAElement.toString(), __("meters_modbus_tcp.content.table_varta_element")],
                                [MeterModbusTCPTableID.VARTAFlex.toString(), __("meters_modbus_tcp.content.table_varta_flex")],
                                [MeterModbusTCPTableID.VictronEnergyGX.toString(), __("meters_modbus_tcp.content.table_victron_energy_gx")],
                                [MeterModbusTCPTableID.Custom.toString(), __("meters_modbus_tcp.content.table_custom")],
                            ]}
                            placeholder={__("select")}
                            value={util.hasValue(config[1].table) ? config[1].table[0].toString() : undefined}
                            onValue={(v) => {
                                let table = parseInt(v);
                                let location = MeterLocation.Unknown;

                                if (table == MeterModbusTCPTableID.TinkerforgeWARPCharger) {
                                    location = MeterLocation.Load; // FIXME: maybe use MeterLocation.Charger in the future?
                                }

                                on_config(util.get_updated_union(config, {location: location, table: new_table_config(table)}));
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
                  || config[1].table[0] == MeterModbusTCPTableID.GoodweHybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.SolaxHybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.FroniusGEN24Plus
                  || config[1].table[0] == MeterModbusTCPTableID.HaileiHybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.FoxESSH3AC3HybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.SiemensPAC2200
                  || config[1].table[0] == MeterModbusTCPTableID.SiemensPAC3120
                  || config[1].table[0] == MeterModbusTCPTableID.SiemensPAC3200
                  || config[1].table[0] == MeterModbusTCPTableID.SiemensPAC3220
                  || config[1].table[0] == MeterModbusTCPTableID.SiemensPAC4200
                  || config[1].table[0] == MeterModbusTCPTableID.SiemensPAC4220
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM24DIN
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM24E1
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM100
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziET100
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM210
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM270
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM280
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM300
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziET300
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM510
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM530
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM540
                  || config[1].table[0] == MeterModbusTCPTableID.SolaredgeInverter
                  || config[1].table[0] == MeterModbusTCPTableID.EastronSDM630TCP
                  || config[1].table[0] == MeterModbusTCPTableID.TinkerforgeWARPCharger
                  || config[1].table[0] == MeterModbusTCPTableID.SAXPowerHomeBasicMode
                  || config[1].table[0] == MeterModbusTCPTableID.SAXPowerHomeExtendedMode
                  || config[1].table[0] == MeterModbusTCPTableID.E3DCHauskraftwerk
                  || config[1].table[0] == MeterModbusTCPTableID.HuaweiSUN2000
                  || config[1].table[0] == MeterModbusTCPTableID.HuaweiSUN2000SmartDongle
                  || config[1].table[0] == MeterModbusTCPTableID.HuaweiEMMA
                  || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM580
                  || config[1].table[0] == MeterModbusTCPTableID.SolaxStringInverter
                  || config[1].table[0] == MeterModbusTCPTableID.FoxESSH3SmartHybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.FoxESSH3ProHybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.SMAHybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.VARTAElement
                  || config[1].table[0] == MeterModbusTCPTableID.VARTAFlex
                  || config[1].table[0] == MeterModbusTCPTableID.ChisageESSHybridInverter
                  || config[1].table[0] == MeterModbusTCPTableID.Janitza
                  || config[1].table[0] == MeterModbusTCPTableID.HuaweiSmartLogger3000)) {
                    let default_device_address = get_default_device_address(config[1].table[0]);

                    if (config[1].table[0] != MeterModbusTCPTableID.TinkerforgeWARPCharger) {
                        edit_children.push(
                            <FormRow label={__("meters_modbus_tcp.content.device_address")} label_muted={default_device_address !== undefined ? __("meters_modbus_tcp.content.device_address_muted")(default_device_address) : undefined}>
                                <InputNumber
                                    required
                                    min={0}
                                    max={255}
                                    value={config[1].table[1].device_address}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {device_address: v})}));
                                    }} />
                            </FormRow>);
                    }

                    let virtual_meter_items: [string, string][] = get_virtual_meter_items(config[1].table[0]);
                    let virtual_meter: number = undefined; // undefined: there are no virtual meters, null: virtual meter is not known yet
                    let default_location: MeterLocation = undefined; // undefined: there is no default location, null: default location is not known yet

                    if (virtual_meter_items.length > 0) {
                        virtual_meter = util.hasValue(config[1].table[1]) && util.hasValue((config[1].table[1] as any).virtual_meter) ? (config[1].table[1] as any).virtual_meter : null;

                        edit_children.push(
                            <FormRow label={__("meters_modbus_tcp.content.virtual_meter")}>
                                <InputSelect
                                    required
                                    items={virtual_meter_items}
                                    placeholder={__("select")}
                                    value={virtual_meter !== null ? virtual_meter.toString() : null}
                                    onValue={(v) => {
                                        let location = config[1].location;

                                        if (util.hasValue(get_default_location)) {
                                            location = get_default_location(config[1].table[0], parseInt(v));
                                        }

                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {virtual_meter: parseInt(v)}), location: location}));
                                    }} />
                            </FormRow>);
                    }

                    if (virtual_meter === null) {
                        default_location = null;
                    }
                    else {
                        default_location = get_default_location(config[1].table[0], virtual_meter);
                    }

                    if (default_location === undefined) {
                        edit_children.push(
                            <FormRow label={__("meters_modbus_tcp.content.location")}>
                                <InputSelect
                                    required
                                    items={get_meter_location_items()}
                                    placeholder={__("select")}
                                    value={config[1].location.toString()}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {location: parseInt(v)}));
                                    }} />
                            </FormRow>);
                    }
                    else {
                        let enable_location_override = default_location !== null && default_location != config[1].location;

                        edit_children.push(
                            <FormRow label={__("meters_modbus_tcp.content.location")}>
                                <SwitchableInputSelect
                                    required
                                    disabled={default_location === null}
                                    items={get_meter_location_items()}
                                    placeholder={__("select")}
                                    value={config[1].location.toString()}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {location: parseInt(v)}));
                                    }}
                                    checked={enable_location_override}
                                    onSwitch={() => {
                                        on_config(util.get_updated_union(config, {location: (enable_location_override ? default_location : MeterLocation.Unknown)}));
                                    }}
                                    switch_label_active={__("meters_modbus_tcp.content.location_different")}
                                    switch_label_inactive={__("meters_modbus_tcp.content.location_matching")}
                                    />
                            </FormRow>);
                    }

                    if (config[1].table[0] == MeterModbusTCPTableID.ShellyPro3EM) {
                        edit_children.push(
                            <FormRow label={__("meters_modbus_tcp.content.shelly_pro_3em_device_profile")}>
                                <InputSelect
                                    required
                                    items={[
                                        [ShellyPro3EMDeviceProfile.Triphase.toString(), __("meters_modbus_tcp.content.shelly_pro_3em_device_profile_triphase")],
                                        [ShellyPro3EMDeviceProfile.Monophase.toString(), __("meters_modbus_tcp.content.shelly_pro_3em_device_profile_monophase")],
                                    ]}
                                    placeholder={__("select")}
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
                                    placeholder={__("select")}
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
                                    placeholder={__("select")}
                                    value={util.hasValue(config[1].table[1]) && util.hasValue((config[1].table[1] as any).monophase_mapping) ? (config[1].table[1] as any).monophase_mapping.toString() : undefined}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {monophase_mapping: parseInt(v)})}));
                                    }} />
                            </FormRow>);
                    }

                    if (config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM100
                     || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziET100
                     || config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM510) {
                        edit_children.push(
                            <FormRow label={__("meters_modbus_tcp.content.carlo_gavazzi_phase")}>
                                <InputSelect
                                    required
                                    items={[
                                        [CarloGavazziPhase.L1.toString(), __("meters_modbus_tcp.content.carlo_gavazzi_phase_l1")],
                                        [CarloGavazziPhase.L2.toString(), __("meters_modbus_tcp.content.carlo_gavazzi_phase_l2")],
                                        [CarloGavazziPhase.L3.toString(), __("meters_modbus_tcp.content.carlo_gavazzi_phase_l3")],
                                    ]}
                                    placeholder={__("select")}
                                    value={util.hasValue(config[1].table[1]) && util.hasValue((config[1].table[1] as any).phase) ? (config[1].table[1] as any).phase.toString() : undefined}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {phase: parseInt(v)})}));
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
                                min={0}
                                max={255}
                                value={config[1].table[1].device_address}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {device_address: v})}));
                                }} />
                        </FormRow>,
                        <FormRow label={__("meters_modbus_tcp.content.location")}>
                            <InputSelect
                                required
                                items={get_meter_location_items()}
                                placeholder={__("select")}
                                value={config[1].location.toString()}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {location: parseInt(v)}));
                                }} />
                        </FormRow>,
                        <FormRow label={__("meters_modbus_tcp.content.register_address_mode")}>
                            <InputSelect
                                required
                                items={[
                                    [ModbusRegisterAddressMode.Address.toString(), __("meters_modbus_tcp.content.register_address_mode_address")],
                                    [ModbusRegisterAddressMode.Number.toString(), __("meters_modbus_tcp.content.register_address_mode_number")]
                                ]}
                                placeholder={__("select")}
                                value={util.hasValue(config[1].table[1].register_address_mode) ? config[1].table[1].register_address_mode.toString() : undefined}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {register_address_mode: parseInt(v)})}));
                                }} />
                        </FormRow>,
                        <FormRow label={__("meters_modbus_tcp.content.registers")}>
                            <RegisterEditor
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
