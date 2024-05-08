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
         VictronEnergyGXVirtualMeterID,
         DeyeHybridInverterVirtualMeterID } from "./meters_modbus_tcp_defs";
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

export function init() {
    return {
        [MeterClassID.ModbusTCP]: {
            name: __("meters_modbus_tcp.content.meter_class"),
            new_config: () => [MeterClassID.ModbusTCP, {display_name: "", host: "", port: 502, table: null}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: ModbusTCPMetersConfig, on_config: (config: ModbusTCPMetersConfig) => void): ComponentChildren => {
                let table_ids: [string, string][] = [
                    [MeterModbusTCPTableID.SungrowHybridInverter.toString(), __("meters_modbus_tcp.content.config_table_sungrow_hybrid_inverter")],
                    [MeterModbusTCPTableID.SungrowStringInverter.toString(), __("meters_modbus_tcp.content.config_table_sungrow_string_inverter")],
                    [MeterModbusTCPTableID.SolarmaxMaxStorage.toString(), __("meters_modbus_tcp.content.config_table_solarmax_max_storage")],
                    [MeterModbusTCPTableID.VictronEnergyGX.toString(), __("meters_modbus_tcp.content.config_table_victron_energy_gx")],
                    [MeterModbusTCPTableID.DeyeHybridInverter.toString(), __("meters_modbus_tcp.content.config_table_deye_hybrid_inverter")],
                ];

                let edit_children = [
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
                            [SungrowHybridInverterVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.config_virtual_meter_inverter")],
                            [SungrowHybridInverterVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.config_virtual_meter_grid")],
                            [SungrowHybridInverterVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.config_virtual_meter_battery")],
                            [SungrowHybridInverterVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.config_virtual_meter_load")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SungrowStringInverter) {
                        items = [
                            [SungrowStringInverterVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.config_virtual_meter_inverter")],
                            [SungrowStringInverterVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.config_virtual_meter_grid")],
                            [SungrowStringInverterVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.config_virtual_meter_load")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.SolarmaxMaxStorage) {
                        items = [
                            [SolarmaxMaxStorageVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.config_virtual_meter_inverter")],
                            [SolarmaxMaxStorageVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.config_virtual_meter_grid")],
                            [SolarmaxMaxStorageVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.config_virtual_meter_battery")],
                        ];
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.VictronEnergyGX) {
                        items = [
                            [VictronEnergyGXVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.config_virtual_meter_inverter")],
                            [VictronEnergyGXVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.config_virtual_meter_grid")],
                            [VictronEnergyGXVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.config_virtual_meter_battery")],
                            [VictronEnergyGXVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.config_virtual_meter_load")],
                        ];

                        device_address_default = 100;
                    }
                    else if (config[1].table[0] == MeterModbusTCPTableID.DeyeHybridInverter) {
                        items = [
                            [DeyeHybridInverterVirtualMeterID.Inverter.toString(), __("meters_modbus_tcp.content.config_virtual_meter_inverter")],
                            [DeyeHybridInverterVirtualMeterID.Grid.toString(), __("meters_modbus_tcp.content.config_virtual_meter_grid")],
                            [DeyeHybridInverterVirtualMeterID.Battery.toString(), __("meters_modbus_tcp.content.config_virtual_meter_battery")],
                            [DeyeHybridInverterVirtualMeterID.Load.toString(), __("meters_modbus_tcp.content.config_virtual_meter_load")],
                        ];
                    }

                    edit_children.push(
                        <FormRow label={__("meters_modbus_tcp.content.config_virtual_meter")}>
                            <InputSelect
                                required
                                items={items}
                                placeholder={__("meters_modbus_tcp.content.config_virtual_meter_select")}
                                value={util.hasValue(config[1].table[1]) && util.hasValue(config[1].table[1].virtual_meter) ? config[1].table[1].virtual_meter.toString() : undefined}
                                onValue={(v) => {
                                    on_config(util.get_updated_union(config, {table: util.get_updated_union(config[1].table, {virtual_meter: parseInt(v)})}));
                                }} />
                        </FormRow>,
                        <FormRow label={__("meters_modbus_tcp.content.config_device_address")} label_muted={__("meters_modbus_tcp.content.config_device_address_muted")(device_address_default)}>
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

                return edit_children;
            },
        },
    };
}
