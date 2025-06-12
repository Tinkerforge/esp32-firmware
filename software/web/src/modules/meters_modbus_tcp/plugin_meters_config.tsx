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
import { MeterLocation } from "../meters/meter_location.enum";
import { get_meter_location_items } from "../meters/meter_location";
import { MeterConfig } from "../meters/types";
import { MeterValueIDSelector, get_meter_value_id_name } from "../meters_api/plugin_meters_config";
import { MeterModbusTCPTableID } from "./meter_modbus_tcp_table_id.enum";
import { ModbusRegisterType } from "../modbus_tcp_client/modbus_register_type.enum";
import { ModbusRegisterAddressMode } from "../modbus_tcp_client/modbus_register_address_mode.enum";
import { ModbusValueType } from "../modbus_tcp_client/modbus_value_type.enum";
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
import { SolaxHybridInverterVirtualMeter } from "./solax_hybrid_inverter_virtual_meter.enum";
import { FroniusGEN24PlusVirtualMeter } from "./fronius_gen24_plus_virtual_meter.enum";
import { HaileiHybridInverterVirtualMeter } from "./hailei_hybrid_inverter_virtual_meter.enum";
import { FoxESSH3HybridInverterVirtualMeter } from "./fox_ess_h3_hybrid_inverter_virtual_meter.enum";
import { CarloGavazziPhase } from "./carlo_gavazzi_phase.enum";
import { CarloGavazziEM270VirtualMeter } from "./carlo_gavazzi_em270_virtual_meter.enum";
import { CarloGavazziEM280VirtualMeter } from "./carlo_gavazzi_em280_virtual_meter.enum";
import { SolaredgeVirtualMeter } from "./solaredge_virtual_meter.enum";
import { SAXPowerVirtualMeter } from "./sax_power_virtual_meter.enum";
import { E3DCVirtualMeter } from "./e3dc_virtual_meter.enum";
import { HuaweiSUN2000VirtualMeter } from "./huawei_sun2000_virtual_meter.enum";
import { HuaweiSUN2000SmartDongleVirtualMeter } from "./huawei_sun2000_smart_dongle_virtual_meter.enum";
import { HuaweiEMMAVirtualMeter } from "./huawei_emma_virtual_meter.enum";
import { InputText } from "../../ts/components/input_text";
import { InputHost } from "../../ts/components/input_host";
import { InputNumber } from "../../ts/components/input_number";
import { InputAnyFloat } from "../../ts/components/input_any_float";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { SwitchableInputSelect } from "../../ts/components/switchable_input_select";
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

type TableConfigSolaxHybridInverter = [
    MeterModbusTCPTableID.SolaxHybridInverter,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigFroniusGEN24Plus = [
    MeterModbusTCPTableID.FroniusGEN24Plus,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigHaileiHybridInverter = [
    MeterModbusTCPTableID.HaileiHybridInverter,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigFoxESSH3HybridInverter = [
    MeterModbusTCPTableID.FoxESSH3HybridInverter,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigSiemensPAC2200 = [
    MeterModbusTCPTableID.SiemensPAC2200,
    {
        device_address: number;
    },
];

type TableConfigSiemensPAC3120 = [
    MeterModbusTCPTableID.SiemensPAC3120,
    {
        device_address: number;
    },
];

type TableConfigSiemensPAC3200 = [
    MeterModbusTCPTableID.SiemensPAC3200,
    {
        device_address: number;
    },
];

type TableConfigSiemensPAC3220 = [
    MeterModbusTCPTableID.SiemensPAC3220,
    {
        device_address: number;
    },
];

type TableConfigSiemensPAC4200 = [
    MeterModbusTCPTableID.SiemensPAC4200,
    {
        device_address: number;
    },
];

type TableConfigSiemensPAC4220 = [
    MeterModbusTCPTableID.SiemensPAC4220,
    {
        device_address: number;
    },
];

type TableConfigCarloGavazziEM24DIN = [
    MeterModbusTCPTableID.CarloGavazziEM24DIN,
    {
        device_address: number;
    },
];

type TableConfigCarloGavazziEM24E1 = [
    MeterModbusTCPTableID.CarloGavazziEM24E1,
    {
        device_address: number;
    },
];

type TableConfigCarloGavazziEM100 = [
    MeterModbusTCPTableID.CarloGavazziEM100,
    {
        device_address: number;
        phase: number;
    },
];

type TableConfigCarloGavazziET100 = [
    MeterModbusTCPTableID.CarloGavazziET100,
    {
        device_address: number;
        phase: number;
    },
];

type TableConfigCarloGavazziEM210 = [
    MeterModbusTCPTableID.CarloGavazziEM210,
    {
        device_address: number;
    },
];

type TableConfigCarloGavazziEM270 = [
    MeterModbusTCPTableID.CarloGavazziEM270,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigCarloGavazziEM280 = [
    MeterModbusTCPTableID.CarloGavazziEM280,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigCarloGavazziEM300 = [
    MeterModbusTCPTableID.CarloGavazziEM300,
    {
        device_address: number;
    },
];

type TableConfigCarloGavazziET300 = [
    MeterModbusTCPTableID.CarloGavazziET300,
    {
        device_address: number;
    },
];

type TableConfigCarloGavazziEM510 = [
    MeterModbusTCPTableID.CarloGavazziEM510,
    {
        device_address: number;
        phase: number;
    },
];

type TableConfigCarloGavazziEM530 = [
    MeterModbusTCPTableID.CarloGavazziEM530,
    {
        device_address: number;
    },
];

type TableConfigCarloGavazziEM540 = [
    MeterModbusTCPTableID.CarloGavazziEM540,
    {
        device_address: number;
    },
];

type TableConfigSolaredge = [
    MeterModbusTCPTableID.Solaredge,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigEastronSDM630TCP = [
    MeterModbusTCPTableID.EastronSDM630TCP,
    {
        device_address: number;
    },
];

type TableConfigTinkerforgeWARPCharger = [
    MeterModbusTCPTableID.TinkerforgeWARPCharger,
    {},
];

type TableConfigSAXPowerHomeBasicMode = [
    MeterModbusTCPTableID.SAXPowerHomeBasicMode,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigSAXPowerHomeExtendedMode = [
    MeterModbusTCPTableID.SAXPowerHomeExtendedMode,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigE3DC = [
    MeterModbusTCPTableID.E3DC,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigHuaweiSUN2000 = [
    MeterModbusTCPTableID.HuaweiSUN2000,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigHuaweiSUN2000SmartDongle = [
    MeterModbusTCPTableID.HuaweiSUN2000SmartDongle,
    {
        virtual_meter: number;
        device_address: number;
    },
];

type TableConfigHuaweiEMMA = [
    MeterModbusTCPTableID.HuaweiEMMA,
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
                   TableConfigGoodweHybridInverter |
                   TableConfigSolaxHybridInverter |
                   TableConfigFroniusGEN24Plus |
                   TableConfigHaileiHybridInverter |
                   TableConfigFoxESSH3HybridInverter |
                   TableConfigSiemensPAC2200 |
                   TableConfigSiemensPAC3120 |
                   TableConfigSiemensPAC3200 |
                   TableConfigSiemensPAC3220 |
                   TableConfigSiemensPAC4200 |
                   TableConfigSiemensPAC4220 |
                   TableConfigCarloGavazziEM24DIN |
                   TableConfigCarloGavazziEM24E1 |
                   TableConfigCarloGavazziEM100 |
                   TableConfigCarloGavazziET100 |
                   TableConfigCarloGavazziEM210 |
                   TableConfigCarloGavazziEM270 |
                   TableConfigCarloGavazziEM280 |
                   TableConfigCarloGavazziEM300 |
                   TableConfigCarloGavazziET300 |
                   TableConfigCarloGavazziEM510 |
                   TableConfigCarloGavazziEM530 |
                   TableConfigCarloGavazziEM540 |
                   TableConfigSolaredge |
                   TableConfigEastronSDM630TCP |
                   TableConfigTinkerforgeWARPCharger |
                   TableConfigSAXPowerHomeBasicMode |
                   TableConfigSAXPowerHomeExtendedMode |
                   TableConfigE3DC |
                   TableConfigHuaweiSUN2000 |
                   TableConfigHuaweiSUN2000SmartDongle |
                   TableConfigHuaweiEMMA;

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

        case MeterModbusTCPTableID.SolaxHybridInverter:
            return [MeterModbusTCPTableID.SolaxHybridInverter, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.FroniusGEN24Plus:
            return [MeterModbusTCPTableID.FroniusGEN24Plus, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.HaileiHybridInverter:
            return [MeterModbusTCPTableID.HaileiHybridInverter, {virtual_meter: null, device_address: 85}];

        case MeterModbusTCPTableID.FoxESSH3HybridInverter:
            return [MeterModbusTCPTableID.FoxESSH3HybridInverter, {virtual_meter: null, device_address: 247}];

        case MeterModbusTCPTableID.SiemensPAC2200:
            return [MeterModbusTCPTableID.SiemensPAC2200, {device_address: 1}];

        case MeterModbusTCPTableID.SiemensPAC3120:
            return [MeterModbusTCPTableID.SiemensPAC3120, {device_address: 1}];

        case MeterModbusTCPTableID.SiemensPAC3200:
            return [MeterModbusTCPTableID.SiemensPAC3200, {device_address: 1}];

        case MeterModbusTCPTableID.SiemensPAC3220:
            return [MeterModbusTCPTableID.SiemensPAC3220, {device_address: 1}];

        case MeterModbusTCPTableID.SiemensPAC4200:
            return [MeterModbusTCPTableID.SiemensPAC4200, {device_address: 1}];

        case MeterModbusTCPTableID.SiemensPAC4220:
            return [MeterModbusTCPTableID.SiemensPAC4220, {device_address: 1}];

        case MeterModbusTCPTableID.CarloGavazziEM24DIN:
            return [MeterModbusTCPTableID.CarloGavazziEM24DIN, {device_address: 1}];

        case MeterModbusTCPTableID.CarloGavazziEM24E1:
            return [MeterModbusTCPTableID.CarloGavazziEM24E1, {device_address: 1}];

        case MeterModbusTCPTableID.CarloGavazziEM100:
            return [MeterModbusTCPTableID.CarloGavazziEM100, {device_address: 1, phase: null}];

        case MeterModbusTCPTableID.CarloGavazziET100:
            return [MeterModbusTCPTableID.CarloGavazziET100, {device_address: 1, phase: null}];

        case MeterModbusTCPTableID.CarloGavazziEM210:
            return [MeterModbusTCPTableID.CarloGavazziEM210, {device_address: 1}];

        case MeterModbusTCPTableID.CarloGavazziEM270:
            return [MeterModbusTCPTableID.CarloGavazziEM270, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.CarloGavazziEM280:
            return [MeterModbusTCPTableID.CarloGavazziEM280, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.CarloGavazziEM300:
            return [MeterModbusTCPTableID.CarloGavazziEM300, {device_address: 1}];

        case MeterModbusTCPTableID.CarloGavazziET300:
            return [MeterModbusTCPTableID.CarloGavazziET300, {device_address: 1}];

        case MeterModbusTCPTableID.CarloGavazziEM510:
            return [MeterModbusTCPTableID.CarloGavazziEM510, {device_address: 1, phase: null}];

        case MeterModbusTCPTableID.CarloGavazziEM530:
            return [MeterModbusTCPTableID.CarloGavazziEM530, {device_address: 1}];

        case MeterModbusTCPTableID.CarloGavazziEM540:
            return [MeterModbusTCPTableID.CarloGavazziEM540, {device_address: 1}];

        case MeterModbusTCPTableID.Solaredge:
            return [MeterModbusTCPTableID.Solaredge, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.EastronSDM630TCP:
            return [MeterModbusTCPTableID.EastronSDM630TCP, {device_address: 1}];

        case MeterModbusTCPTableID.TinkerforgeWARPCharger:
            return [MeterModbusTCPTableID.TinkerforgeWARPCharger, null];

        case MeterModbusTCPTableID.SAXPowerHomeBasicMode:
            return [MeterModbusTCPTableID.SAXPowerHomeBasicMode, {virtual_meter: null, device_address: 64}];

        case MeterModbusTCPTableID.SAXPowerHomeExtendedMode:
            return [MeterModbusTCPTableID.SAXPowerHomeExtendedMode, {virtual_meter: null, device_address: 40}];

        case MeterModbusTCPTableID.E3DC:
            return [MeterModbusTCPTableID.E3DC, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.HuaweiSUN2000:
            return [MeterModbusTCPTableID.HuaweiSUN2000, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.HuaweiSUN2000SmartDongle:
            return [MeterModbusTCPTableID.HuaweiSUN2000SmartDongle, {virtual_meter: null, device_address: 1}];

        case MeterModbusTCPTableID.HuaweiEMMA:
            return [MeterModbusTCPTableID.HuaweiEMMA, {virtual_meter: null, device_address: 0}];

        default:
            return [MeterModbusTCPTableID.None, null];
    }
}

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

                switch (this.state.register.rtype) {
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
                    ? __("meters_modbus_tcp.content.registers_add_message")(this.props.table[1].registers.length, MAX_CUSTOM_REGISTERS)
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
                                [MeterModbusTCPTableID.DeyeHybridInverter.toString(), __("meters_modbus_tcp.content.table_deye_hybrid_inverter")],
                                [MeterModbusTCPTableID.E3DC.toString(), __("meters_modbus_tcp.content.table_e3dc")],
                                [MeterModbusTCPTableID.EastronSDM630TCP.toString(), __("meters_modbus_tcp.content.table_eastron_sdm630_tcp")],
                                [MeterModbusTCPTableID.FoxESSH3HybridInverter.toString(), __("meters_modbus_tcp.content.table_fox_ess_h3_hybrid_inverter")],
                                [MeterModbusTCPTableID.FroniusGEN24Plus.toString(), __("meters_modbus_tcp.content.table_fronius_gen24_plus")],
                                [MeterModbusTCPTableID.GoodweHybridInverter.toString(), __("meters_modbus_tcp.content.table_goodwe_hybrid_inverter")],
                                [MeterModbusTCPTableID.HaileiHybridInverter.toString(), __("meters_modbus_tcp.content.table_hailei_hybrid_inverter")],
                                [MeterModbusTCPTableID.HuaweiEMMA.toString(), __("meters_modbus_tcp.content.table_huawei_emma")],
                                [MeterModbusTCPTableID.HuaweiSUN2000.toString(), __("meters_modbus_tcp.content.table_huawei_sun2000")],
                                [MeterModbusTCPTableID.HuaweiSUN2000SmartDongle.toString(), __("meters_modbus_tcp.content.table_huawei_sun2000_smart_dongle")],
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
                                [MeterModbusTCPTableID.Solaredge.toString(), __("meters_modbus_tcp.content.table_solaredge")],
                                [MeterModbusTCPTableID.SolarmaxMaxStorage.toString(), __("meters_modbus_tcp.content.table_solarmax_max_storage")],
                                [MeterModbusTCPTableID.SolaxHybridInverter.toString(), __("meters_modbus_tcp.content.table_solax_hybrid_inverter")],
                                [MeterModbusTCPTableID.SungrowHybridInverter.toString(), __("meters_modbus_tcp.content.table_sungrow_hybrid_inverter")],
                                [MeterModbusTCPTableID.SungrowStringInverter.toString(), __("meters_modbus_tcp.content.table_sungrow_string_inverter")],
                                [MeterModbusTCPTableID.TinkerforgeWARPCharger.toString(), __("meters_modbus_tcp.content.table_tinkerforge_warp_charger")],
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
                  || config[1].table[0] == MeterModbusTCPTableID.FoxESSH3HybridInverter
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
                  || config[1].table[0] == MeterModbusTCPTableID.Solaredge
                  || config[1].table[0] == MeterModbusTCPTableID.EastronSDM630TCP
                  || config[1].table[0] == MeterModbusTCPTableID.TinkerforgeWARPCharger
                  || config[1].table[0] == MeterModbusTCPTableID.SAXPowerHomeBasicMode
                  || config[1].table[0] == MeterModbusTCPTableID.SAXPowerHomeExtendedMode
                  || config[1].table[0] == MeterModbusTCPTableID.E3DC
                  || config[1].table[0] == MeterModbusTCPTableID.HuaweiSUN2000
                  || config[1].table[0] == MeterModbusTCPTableID.HuaweiSUN2000SmartDongle
                  || config[1].table[0] == MeterModbusTCPTableID.HuaweiEMMA)) {
                    let virtual_meter_items: [string, string][] = [];
                    let default_location: MeterLocation = undefined; // undefined: there is no default location, null: default location is not known yet
                    let get_default_location: (virtual_meter: number) => MeterLocation = undefined; // undefined: there is no default location
                    let default_device_address: number = 1;

                    if (config[1].table[0] == MeterModbusTCPTableID.SungrowHybridInverter) {
                        virtual_meter_items = [
                            [SungrowHybridInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [SungrowHybridInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SungrowHybridInverterVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [SungrowHybridInverterVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case SungrowHybridInverterVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case SungrowHybridInverterVirtualMeter.Grid: return MeterLocation.Grid;
                            case SungrowHybridInverterVirtualMeter.Battery: return MeterLocation.Battery;
                            case SungrowHybridInverterVirtualMeter.Load: return MeterLocation.Load;
                            }

                            return MeterLocation.Unknown;
                        }
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SungrowStringInverter) {
                        virtual_meter_items = [
                            [SungrowStringInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [SungrowStringInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SungrowStringInverterVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case SungrowStringInverterVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case SungrowStringInverterVirtualMeter.Grid: return MeterLocation.Grid;
                            case SungrowStringInverterVirtualMeter.Load: return MeterLocation.Load;
                            }

                            return MeterLocation.Unknown;
                        }
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SolarmaxMaxStorage) {
                        virtual_meter_items = [
                            [SolarmaxMaxStorageVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [SolarmaxMaxStorageVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SolarmaxMaxStorageVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case SolarmaxMaxStorageVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case SolarmaxMaxStorageVirtualMeter.Grid: return MeterLocation.Grid;
                            case SolarmaxMaxStorageVirtualMeter.Battery: return MeterLocation.Battery;
                            }

                            return MeterLocation.Unknown;
                        }
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.VictronEnergyGX) {
                        virtual_meter_items = [
                            [VictronEnergyGXVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [VictronEnergyGXVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [VictronEnergyGXVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [VictronEnergyGXVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case VictronEnergyGXVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case VictronEnergyGXVirtualMeter.Grid: return MeterLocation.Grid;
                            case VictronEnergyGXVirtualMeter.Battery: return MeterLocation.Battery;
                            case VictronEnergyGXVirtualMeter.Load: return MeterLocation.Load;
                            }

                            return MeterLocation.Unknown;
                        }

                        default_device_address = 100;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.DeyeHybridInverter) {
                        virtual_meter_items = [
                            [DeyeHybridInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [DeyeHybridInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [DeyeHybridInverterVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [DeyeHybridInverterVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                            [DeyeHybridInverterVirtualMeter.PV.toString(), __("meters_modbus_tcp.content.virtual_meter_pv")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case DeyeHybridInverterVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case DeyeHybridInverterVirtualMeter.Grid: return MeterLocation.Grid;
                            case DeyeHybridInverterVirtualMeter.Battery: return MeterLocation.Battery;
                            case DeyeHybridInverterVirtualMeter.Load: return MeterLocation.Load;
                            case DeyeHybridInverterVirtualMeter.PV: return MeterLocation.PV;
                            }

                            return MeterLocation.Unknown;
                        }
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.AlphaESSHybridInverter) {
                        virtual_meter_items = [
                            [AlphaESSHybridInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [AlphaESSHybridInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [AlphaESSHybridInverterVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [AlphaESSHybridInverterVirtualMeter.PV.toString(), __("meters_modbus_tcp.content.virtual_meter_pv")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case AlphaESSHybridInverterVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case AlphaESSHybridInverterVirtualMeter.Grid: return MeterLocation.Grid;
                            case AlphaESSHybridInverterVirtualMeter.Battery: return MeterLocation.Battery;
                            case AlphaESSHybridInverterVirtualMeter.PV: return MeterLocation.PV;
                            }

                            return MeterLocation.Unknown;
                        }

                        default_device_address = 85;
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

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case GoodweHybridInverterVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case GoodweHybridInverterVirtualMeter.Grid: return MeterLocation.Grid;
                            case GoodweHybridInverterVirtualMeter.Battery: return MeterLocation.Battery;
                            case GoodweHybridInverterVirtualMeter.Load: return MeterLocation.Load;
                            case GoodweHybridInverterVirtualMeter.BackupLoad: return MeterLocation.Load;
                            case GoodweHybridInverterVirtualMeter.Meter: return MeterLocation.Other;
                            }

                            return MeterLocation.Unknown;
                        }

                        default_device_address = 247;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SolaxHybridInverter) {
                        virtual_meter_items = [
                            [SolaxHybridInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [SolaxHybridInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SolaxHybridInverterVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case SolaxHybridInverterVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case SolaxHybridInverterVirtualMeter.Grid: return MeterLocation.Grid;
                            case SolaxHybridInverterVirtualMeter.Battery: return MeterLocation.Battery;
                            }

                            return MeterLocation.Unknown;
                        }
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.FroniusGEN24Plus) {
                        virtual_meter_items = [
                            [FroniusGEN24PlusVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case FroniusGEN24PlusVirtualMeter.Battery: return MeterLocation.Battery;
                            }

                            return MeterLocation.Unknown;
                        }
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.HaileiHybridInverter) {
                        virtual_meter_items = [
                            [HaileiHybridInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [HaileiHybridInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [HaileiHybridInverterVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [HaileiHybridInverterVirtualMeter.PV.toString(), __("meters_modbus_tcp.content.virtual_meter_pv")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case HaileiHybridInverterVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case HaileiHybridInverterVirtualMeter.Grid: return MeterLocation.Grid;
                            case HaileiHybridInverterVirtualMeter.Battery: return MeterLocation.Battery;
                            case HaileiHybridInverterVirtualMeter.PV: return MeterLocation.PV;
                            }

                            return MeterLocation.Unknown;
                        }

                        default_device_address = 85;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.FoxESSH3HybridInverter) {
                        virtual_meter_items = [
                            [FoxESSH3HybridInverterVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [FoxESSH3HybridInverterVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [FoxESSH3HybridInverterVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case FoxESSH3HybridInverterVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case FoxESSH3HybridInverterVirtualMeter.Grid: return MeterLocation.Grid;
                            case FoxESSH3HybridInverterVirtualMeter.Battery: return MeterLocation.Battery;
                            }

                            return MeterLocation.Unknown;
                        }

                        default_device_address = 247;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM270) {
                        virtual_meter_items = [
                            [CarloGavazziEM270VirtualMeter.Meter.toString(), __("meters_modbus_tcp.content.virtual_meter_meter")],
                            [CarloGavazziEM270VirtualMeter.CurrentTransformer1.toString(), __("meters_modbus_tcp.content.virtual_meter_current_transformer_1")],
                            [CarloGavazziEM270VirtualMeter.CurrentTransformer2.toString(), __("meters_modbus_tcp.content.virtual_meter_current_transformer_2")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.CarloGavazziEM280) {
                        virtual_meter_items = [
                            [CarloGavazziEM280VirtualMeter.Meter.toString(), __("meters_modbus_tcp.content.virtual_meter_meter")],
                            [CarloGavazziEM280VirtualMeter.CurrentTransformer1.toString(), __("meters_modbus_tcp.content.virtual_meter_current_transformer_1")],
                            [CarloGavazziEM280VirtualMeter.CurrentTransformer2.toString(), __("meters_modbus_tcp.content.virtual_meter_current_transformer_2")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.Solaredge) {
                        virtual_meter_items = [
                            [SolaredgeVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case SolaredgeVirtualMeter.Battery: return MeterLocation.Battery;
                            }

                            return MeterLocation.Unknown;
                        }
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.TinkerforgeWARPCharger) {
                        default_location = MeterLocation.Load;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SAXPowerHomeBasicMode) {
                        virtual_meter_items = [
                            [SAXPowerVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SAXPowerVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case SAXPowerVirtualMeter.Grid: return MeterLocation.Grid;
                            case SAXPowerVirtualMeter.Battery: return MeterLocation.Battery;
                            }

                            return MeterLocation.Unknown;
                        }

                        default_device_address = 40;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SAXPowerHomeExtendedMode) {
                        virtual_meter_items = [
                            [SAXPowerVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [SAXPowerVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case SAXPowerVirtualMeter.Grid: return MeterLocation.Grid;
                            case SAXPowerVirtualMeter.Battery: return MeterLocation.Battery;
                            }

                            return MeterLocation.Unknown;
                        }

                        default_device_address = 64;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.E3DC) {
                        virtual_meter_items = [
                            [E3DCVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [E3DCVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [E3DCVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                            [E3DCVirtualMeter.PV.toString(), __("meters_modbus_tcp.content.virtual_meter_pv")],
                            [E3DCVirtualMeter.AdditionalGeneration.toString(), __("meters_modbus_tcp.content.virtual_meter_additional_generation")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case E3DCVirtualMeter.Grid: return MeterLocation.Grid;
                            case E3DCVirtualMeter.Battery: return MeterLocation.Battery;
                            case E3DCVirtualMeter.Load: return MeterLocation.Load;
                            case E3DCVirtualMeter.PV: return MeterLocation.PV;
                            case E3DCVirtualMeter.AdditionalGeneration: return MeterLocation.PV;
                            }

                            return MeterLocation.Unknown;
                        }
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.HuaweiSUN2000) {
                        virtual_meter_items = [
                            [HuaweiSUN2000VirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [HuaweiSUN2000VirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [HuaweiSUN2000VirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [HuaweiSUN2000VirtualMeter.PV.toString(), __("meters_modbus_tcp.content.virtual_meter_pv")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case HuaweiSUN2000VirtualMeter.Inverter: return MeterLocation.Inverter;
                            case HuaweiSUN2000VirtualMeter.Grid: return MeterLocation.Grid;
                            case HuaweiSUN2000VirtualMeter.Battery: return MeterLocation.Battery;
                            case HuaweiSUN2000VirtualMeter.PV: return MeterLocation.PV;
                            }

                            return MeterLocation.Unknown;
                        }
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.HuaweiSUN2000SmartDongle) {
                        virtual_meter_items = [
                            [HuaweiSUN2000SmartDongleVirtualMeter.Grid.toString(), __("meters_modbus_tcp.content.virtual_meter_grid")],
                            [HuaweiSUN2000SmartDongleVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [HuaweiSUN2000SmartDongleVirtualMeter.PV.toString(), __("meters_modbus_tcp.content.virtual_meter_pv")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case HuaweiSUN2000SmartDongleVirtualMeter.Grid: return MeterLocation.Grid;
                            case HuaweiSUN2000SmartDongleVirtualMeter.Battery: return MeterLocation.Battery;
                            case HuaweiSUN2000SmartDongleVirtualMeter.PV: return MeterLocation.PV;
                            }

                            return MeterLocation.Unknown;
                        }
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.HuaweiEMMA) {
                        virtual_meter_items = [
                            [HuaweiEMMAVirtualMeter.Inverter.toString(), __("meters_modbus_tcp.content.virtual_meter_inverter")],
                            [HuaweiEMMAVirtualMeter.GridInternalSensor.toString(), __("meters_modbus_tcp.content.virtual_meter_grid_internal_sensor")],
                            [HuaweiEMMAVirtualMeter.GridExternalSensor.toString(), __("meters_modbus_tcp.content.virtual_meter_grid_external_sensor")],
                            [HuaweiEMMAVirtualMeter.Battery.toString(), __("meters_modbus_tcp.content.virtual_meter_battery")],
                            [HuaweiEMMAVirtualMeter.PV.toString(), __("meters_modbus_tcp.content.virtual_meter_pv")],
                            [HuaweiEMMAVirtualMeter.Load.toString(), __("meters_modbus_tcp.content.virtual_meter_load")],
                        ];

                        get_default_location = (virtual_meter: number) => {
                            switch (virtual_meter) {
                            case HuaweiEMMAVirtualMeter.Inverter: return MeterLocation.Inverter;
                            case HuaweiEMMAVirtualMeter.GridInternalSensor: return MeterLocation.Grid;
                            case HuaweiEMMAVirtualMeter.GridExternalSensor: return MeterLocation.Grid;
                            case HuaweiEMMAVirtualMeter.Battery: return MeterLocation.Battery;
                            case HuaweiEMMAVirtualMeter.Load: return MeterLocation.Load;
                            case HuaweiEMMAVirtualMeter.PV: return MeterLocation.PV;
                            }

                            return MeterLocation.Unknown;
                        }

                        default_device_address = 0;
                    }

                    if (virtual_meter_items.length > 0) {
                        let virtual_meter = util.hasValue(config[1].table[1]) && util.hasValue((config[1].table[1] as any).virtual_meter) ? (config[1].table[1] as any).virtual_meter : null;

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
                                            location = get_default_location(parseInt(v));
                                        }

                                        on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {virtual_meter: parseInt(v)}), location: location}));
                                    }} />
                            </FormRow>);

                        if (!util.hasValue(get_default_location)) {
                            default_location = undefined;
                        }
                        else if (virtual_meter === null) {
                            default_location = null;
                        }
                        else {
                            default_location = get_default_location(virtual_meter);
                        }
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

                    if (config[1].table[0] != MeterModbusTCPTableID.TinkerforgeWARPCharger) {
                        edit_children.push(
                            <FormRow label={__("meters_modbus_tcp.content.device_address")} label_muted={__("meters_modbus_tcp.content.device_address_muted")(default_device_address)}>
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
