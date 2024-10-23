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

import { h, ComponentChildren } from "preact";
import { __ } from "../../ts/translation";
import * as util from "../../ts/util";
import { MeterClassID } from "../meters/meter_class_id.enum";
import { MeterConfig } from "../meters/types";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { VirtualMeter } from "./virtual_meter.enum";

export type RCTPowerMetersConfig = [
    MeterClassID.RCTPower,
    {
        display_name: string;
        host: string;
        port: number;
        virtual_meter: number;
    },
];

export function init() {
    return {
        [MeterClassID.RCTPower]: {
            name: () => __("meters_rct_power.content.meter_class"),
            new_config: () => [MeterClassID.RCTPower, {display_name: "", host: "", port: 8899, virtual_meter: null}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: RCTPowerMetersConfig, on_config: (config: RCTPowerMetersConfig) => void): ComponentChildren => {
                return [
                    <FormRow label={__("meters_rct_power.content.display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {display_name: v}));
                            }}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_rct_power.content.host")}>
                        <InputText
                            required
                            maxLength={64}
                            pattern="^[a-zA-Z0-9\-\.]+$"
                            value={config[1].host}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {host: v}));
                            }}
                            invalidFeedback={__("meters_rct_power.content.host_invalid")} />
                    </FormRow>,
                    <FormRow label={__("meters_rct_power.content.port")} label_muted={__("meters_rct_power.content.port_muted")}>
                        <InputNumber
                            required
                            min={1}
                            max={65535}
                            value={config[1].port}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {port: v}));
                            }} />
                    </FormRow>,
                    <FormRow label={__("meters_rct_power.content.virtual_meter")}>
                        <InputSelect
                            required
                            items={[
                                [VirtualMeter.Grid.toString(), __("meters_rct_power.content.virtual_meter_grid")],
                                [VirtualMeter.Battery.toString(), __("meters_rct_power.content.virtual_meter_battery")],
                            ]}
                            placeholder={__("meters_rct_power.content.virtual_meter_select")}
                            value={util.hasValue(config[1].virtual_meter) ? config[1].virtual_meter.toString() : undefined}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {virtual_meter: parseInt(v)}));
                            }} />
                    </FormRow>,
                ];
            },
        },
    };
}
