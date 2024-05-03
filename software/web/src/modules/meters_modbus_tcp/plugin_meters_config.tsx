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
import { h, Fragment, ComponentChildren } from "preact";
import { __ } from "../../ts/translation";
import { MeterClassID } from "../meters/meters_defs";
import { MeterConfig } from "../meters/types";
import { MeterModbusTCPTableID,
         SungrowHybridInverterVirtualMeterID,
         SungrowStringInverterVirtualMeterID,
         SolarmaxMaxStorageVirtualMeterID,
         VictronEnergyColorControlGXVirtualMeterID } from "./meters_modbus_tcp_defs";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";

type TableConfigNone = [
    MeterModbusTCPTableID.None,
    {},
];

type TableConfigSungrowHybridInverter = [
    MeterModbusTCPTableID.SungrowHybridInverter,
    {
        virtual_meter: number;
    },
];

type TableConfigSungrowStringInverter = [
    MeterModbusTCPTableID.SungrowStringInverter,
    {
        virtual_meter: number;
    },
];

type TableConfigSolarmaxMaxStorage = [
    MeterModbusTCPTableID.SolarmaxMaxStorage,
    {
        virtual_meter: number;
    },
];

type TableConfigVictronEnergyColorControlGX = [
    MeterModbusTCPTableID.VictronEnergyColorControlGX,
    {
        virtual_meter: number;
    },
];

type TableConfig = TableConfigNone |
                   TableConfigSungrowHybridInverter |
                   TableConfigSungrowStringInverter |
                   TableConfigSolarmaxMaxStorage |
                   TableConfigVictronEnergyColorControlGX;

export type ModbusTCPMetersConfig = [
    MeterClassID.ModbusTCP,
    {
        display_name: string;
        host: string;
        port: number;
        device_address: number;
        table: TableConfig;
    },
];

function new_table_config(table: MeterModbusTCPTableID): TableConfig {
    switch (table) {
        case MeterModbusTCPTableID.SungrowHybridInverter:
            return [MeterModbusTCPTableID.SungrowHybridInverter, {virtual_meter: null}];

        case MeterModbusTCPTableID.SungrowStringInverter:
            return [MeterModbusTCPTableID.SungrowStringInverter, {virtual_meter: null}];

        case MeterModbusTCPTableID.SolarmaxMaxStorage:
            return [MeterModbusTCPTableID.SolarmaxMaxStorage, {virtual_meter: null}];

        case MeterModbusTCPTableID.VictronEnergyColorControlGX:
            return [MeterModbusTCPTableID.VictronEnergyColorControlGX, {virtual_meter: null}];

        default:
            return [MeterModbusTCPTableID.None, {}];
    }
}

export function init() {
    return {
        [MeterClassID.ModbusTCP]: {
            name: __("meters_modbus_tcp.content.meter_class"),
            new_config: () => [MeterClassID.ModbusTCP, {display_name: "", host: "", port: 502, device_address: 1, table: null}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: ModbusTCPMetersConfig, on_config: (config: ModbusTCPMetersConfig) => void): ComponentChildren => {
                let table_ids: [string, string][] = [
                    [MeterModbusTCPTableID.SungrowHybridInverter.toString(), __("meters_modbus_tcp.content.table_sungrow_hybrid_inverter")],
                    [MeterModbusTCPTableID.SungrowStringInverter.toString(), __("meters_modbus_tcp.content.table_sungrow_string_inverter")],
                    [MeterModbusTCPTableID.SolarmaxMaxStorage.toString(), __("meters_modbus_tcp.content.table_solarmax_max_storage")],
                    [MeterModbusTCPTableID.VictronEnergyColorControlGX.toString(), __("meters_modbus_tcp.content.table_victron_energy_color_control_gx")],
                ];

                return [
                    <FormRow label={__("meters_modbus_tcp.content.config_display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {display_name: v}));
                            }} />
                    </FormRow>,
                    <FormRow label={__("meters_modbus_tcp.content.config_host")}>
                        <InputText
                            required
                            maxLength={64}
                            pattern="^[a-zA-Z0-9\-\.]+$"
                            value={config[1].host}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {host: v}));
                            }}
                            invalidFeedback={__("meters_modbus_tcp.content.config_host_invalid")} />
                    </FormRow>,
                    <FormRow label={__("meters_modbus_tcp.content.config_port")} label_muted={__("meters_modbus_tcp.content.config_port_muted")}>
                        <InputNumber
                            required
                            min={1}
                            max={65535}
                            value={config[1].port}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {port: v}));
                            }} />
                    </FormRow>,
                    <FormRow label={__("meters_modbus_tcp.content.config_device_address")}>
                        <InputNumber
                            required
                            min={1}
                            max={247}
                            value={config[1].device_address}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {device_address: v}));
                            }} />
                    </FormRow>,
                    <FormRow label={__("meters_modbus_tcp.content.config_table")}>
                        <InputSelect
                            required
                            items={table_ids}
                            placeholder={__("meters_modbus_tcp.content.config_table_select")}
                            value={util.hasValue(config[1].table) ? config[1].table[0].toString() : undefined}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {table: new_table_config(parseInt(v))}));
                            }} />
                    </FormRow>,

                    <>{util.hasValue(config[1].table) && config[1].table[0] == MeterModbusTCPTableID.SungrowHybridInverter ?
                        <FormRow label={__("meters_modbus_tcp.content.sungrow_hybrid_inverter_virtual_meter")}>
                            <InputSelect
                                required
                                items={[
                                    [SungrowHybridInverterVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.sungrow_hybrid_inverter_virtual_meter_inverter")],
                                    [SungrowHybridInverterVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.sungrow_hybrid_inverter_virtual_meter_grid")],
                                    [SungrowHybridInverterVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.sungrow_hybrid_inverter_virtual_meter_battery")],
                                    [SungrowHybridInverterVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.sungrow_hybrid_inverter_virtual_meter_load")],
                                ]}
                                placeholder={__("meters_modbus_tcp.content.sungrow_hybrid_inverter_virtual_meter_select")}
                                value={util.hasValue(config[1].table[1]) && util.hasValue(config[1].table[1].virtual_meter) ? config[1].table[1].virtual_meter.toString() : undefined}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {virtual_meter: parseInt(v)})}));
                                }} />
                        </FormRow>
                        : undefined
                    }

                    {util.hasValue(config[1].table) && config[1].table[0] == MeterModbusTCPTableID.SungrowStringInverter ?
                        <FormRow label={__("meters_modbus_tcp.content.sungrow_string_inverter_virtual_meter")}>
                            <InputSelect
                                required
                                items={[
                                    [SungrowStringInverterVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.sungrow_string_inverter_virtual_meter_inverter")],
                                    [SungrowStringInverterVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.sungrow_string_inverter_virtual_meter_grid")],
                                    [SungrowStringInverterVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.sungrow_string_inverter_virtual_meter_load")],
                                ]}
                                placeholder={__("meters_modbus_tcp.content.sungrow_string_inverter_virtual_meter_select")}
                                value={util.hasValue(config[1].table[1]) && util.hasValue(config[1].table[1].virtual_meter) ? config[1].table[1].virtual_meter.toString() : undefined}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {virtual_meter: parseInt(v)})}));
                                }} />
                        </FormRow>
                        : undefined
                    }

                    {util.hasValue(config[1].table) && config[1].table[0] == MeterModbusTCPTableID.SolarmaxMaxStorage ?
                        <FormRow label={__("meters_modbus_tcp.content.solarmax_max_storage_virtual_meter")}>
                            <InputSelect
                                required
                                items={[
                                    [SolarmaxMaxStorageVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.solarmax_max_storage_virtual_meter_inverter")],
                                    [SolarmaxMaxStorageVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.solarmax_max_storage_virtual_meter_grid")],
                                    [SolarmaxMaxStorageVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.solarmax_max_storage_virtual_meter_battery")],
                                ]}
                                placeholder={__("meters_modbus_tcp.content.solarmax_max_storage_virtual_meter_select")}
                                value={util.hasValue(config[1].table[1]) && util.hasValue(config[1].table[1].virtual_meter) ? config[1].table[1].virtual_meter.toString() : undefined}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {virtual_meter: parseInt(v)})}));
                                }} />
                        </FormRow>
                        : undefined
                    }

                    {util.hasValue(config[1].table) && config[1].table[0] == MeterModbusTCPTableID.VictronEnergyColorControlGX ?
                        <FormRow label={__("meters_modbus_tcp.content.victron_energy_color_control_gx_virtual_meter")}>
                            <InputSelect
                                required
                                items={[
                                    [VictronEnergyColorControlGXVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.victron_energy_color_control_gx_virtual_meter_inverter")],
                                    [VictronEnergyColorControlGXVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.victron_energy_color_control_gx_virtual_meter_grid")],
                                    [VictronEnergyColorControlGXVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.victron_energy_color_control_gx_virtual_meter_battery")],
                                    [VictronEnergyColorControlGXVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.victron_energy_color_control_gx_virtual_meter_load")],
                                ]}
                                placeholder={__("meters_modbus_tcp.content.victron_energy_color_control_gx_virtual_meter_select")}
                                value={util.hasValue(config[1].table[1]) && util.hasValue(config[1].table[1].virtual_meter) ? config[1].table[1].virtual_meter.toString() : undefined}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {virtual_meter: parseInt(v)})}));
                                }} />
                        </FormRow>
                        : undefined
                    }</>
                ];
            },
        },
    };
}
