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
import { MeterModbusTCPPreset } from "./meters_modbus_tcp_defs";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";

export type ModbusTCPMetersConfig = [
    MeterClassID.ModbusTCP,
    {
        display_name: string;
        host: string;
        port: number;
        device_address: number;
        preset: number;
    },
];

export function init() {
    return {
        [MeterClassID.ModbusTCP]: {
            name: __("meters_modbus_tcp.content.meter_class"),
            new_config: () => [MeterClassID.ModbusTCP, {display_name: "", host: "", port: 502, device_address: 1, preset: null}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: ModbusTCPMetersConfig, on_config: (config: ModbusTCPMetersConfig) => void): ComponentChildren => {
                let model_ids: [string, string][] = [
                    [MeterModbusTCPPreset.SungrowHybridInverter.toString(), __("meters_modbus_tcp.content.preset_sungrow_hybrid_inverter")],
                    [MeterModbusTCPPreset.SungrowHybridInverterGrid.toString(), __("meters_modbus_tcp.content.preset_sungrow_hybrid_inverter_grid")],
                    [MeterModbusTCPPreset.SungrowHybridInverterBattery.toString(), __("meters_modbus_tcp.content.preset_sungrow_hybrid_inverter_battery")],
                    [MeterModbusTCPPreset.SungrowHybridInverterLoad.toString(), __("meters_modbus_tcp.content.preset_sungrow_hybrid_inverter_load")],
                ];

                return [<>
                    <FormRow label={__("meters_modbus_tcp.content.config_display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {display_name: v}));
                            }} />
                    </FormRow>
                    <FormRow label={__("meters_modbus_tcp.content.config_host")}>
                        <InputText
                            required
                            maxLength={64}
                            pattern="^[a-zA-Z0-9\-\.]+$"
                            value={config[1].host}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {host: v}));
                            }}
                            invalidFeedback={__("meters_sun_spec.content.config_host_invalid")} />
                    </FormRow>
                    <FormRow label={__("meters_modbus_tcp.content.config_port")}>
                        <InputNumber
                            required
                            min={1}
                            max={65535}
                            value={config[1].port}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {port: v}));
                            }} />
                    </FormRow>
                    <FormRow label={__("meters_modbus_tcp.content.config_device_address")}>
                        <InputNumber
                            required
                            min={1}
                            max={247}
                            value={config[1].device_address}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {device_address: v}));
                            }} />
                    </FormRow>
                    <FormRow label={__("meters_modbus_tcp.content.config_preset")}>
                        <InputSelect
                            required
                            items={model_ids}
                            placeholder={__("meters_modbus_tcp.content.config_preset_select")}
                            value={util.hasValue(config[1].preset) ? config[1].preset.toString() : config[1].preset}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {preset: parseInt(v)}));
                            }} />
                    </FormRow>
                </>];
            },
        },
    };
}
