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
import { MeterClassID } from "../meters/generated/meter_class_id.enum";
import { MeterLocation } from "../meters/generated/meter_location.enum";
import { get_meter_location_items, translate_meter_location } from "../meters/meter_location";
import { MeterConfig } from "../meters/types";
import { InputText } from "../../ts/components/input_text";
import { InputHost } from "../../ts/components/input_host";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { VirtualMeter } from "./generated/virtual_meter.enum";

export type RCTPowerMetersConfig = [
    MeterClassID.RCTPower,
    {
        display_name: string;
        location: number;
        host: string;
        port: number;
        virtual_meter: number;
    },
];

function get_fixed_location(virtual_meter: number) {
    if (util.hasValue(virtual_meter)) {
        switch (virtual_meter) {
        case VirtualMeter.Inverter: return MeterLocation.Inverter;
        case VirtualMeter.Grid: return MeterLocation.Grid;
        case VirtualMeter.Battery: return MeterLocation.Battery;
        case VirtualMeter.Load: return MeterLocation.Load;
        case VirtualMeter.PV: return MeterLocation.PV;
        }
    }

    return MeterLocation.Unknown;
}

export function pre_init() {
    return {
        [MeterClassID.RCTPower]: {
            name: () => __("meters_rct_power.content.meter_class"),
            new_config: () => [MeterClassID.RCTPower, {display_name: "", location: MeterLocation.Unknown, host: "", port: 8899, virtual_meter: null}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: RCTPowerMetersConfig, on_config: (config: RCTPowerMetersConfig) => void): ComponentChildren => {
                let virtual_meter_items: [string, string][] = [
                    [VirtualMeter.Inverter.toString(), __("meters_rct_power.content.virtual_meter_inverter")],
                    [VirtualMeter.Grid.toString(), __("meters_rct_power.content.virtual_meter_grid")],
                    [VirtualMeter.Battery.toString(), __("meters_rct_power.content.virtual_meter_battery")],
                    [VirtualMeter.Load.toString(), __("meters_rct_power.content.virtual_meter_load")],
                    [VirtualMeter.PV.toString(), __("meters_rct_power.content.virtual_meter_pv")],
                ];

                let edit_children = [
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
                        <InputHost
                            required
                            value={config[1].host}
                            onValue={(v) => on_config(util.get_updated_union(config, {host: v}))} />
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
                            items={virtual_meter_items}
                            placeholder={__("select")}
                            value={util.hasValue(config[1].virtual_meter) ? config[1].virtual_meter.toString() : null}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {virtual_meter: parseInt(v), location: get_fixed_location(parseInt(v))}));
                            }} />
                    </FormRow>
                ];

                let virtual_meter: number = util.hasValue(config[1].virtual_meter) ? config[1].virtual_meter : null; // null: virtual meter is not known yet
                let fixed_location = MeterLocation.Unknown; // MeterLocation.Unknown: there is no fixed location, null: fixed location is not known yet

                if (virtual_meter === null) {
                    // check if location depends on virtual meter
                    for (let item of virtual_meter_items) {
                        if (get_fixed_location(parseInt(item[0])) != MeterLocation.Unknown) {
                            fixed_location = null;
                            break;
                        }
                    }
                }
                else if (virtual_meter !== undefined) {
                    fixed_location = get_fixed_location(virtual_meter);
                }

                edit_children.push(
                    <FormRow label={__("meters_rct_power.content.location")}>
                        {fixed_location === null ?
                            <InputText value={__("meters_rct_power.content.location_depends_virtual_meter")} /> :
                            (fixed_location == MeterLocation.Unknown ?
                                <InputSelect
                                    required
                                    items={get_meter_location_items()}
                                    placeholder={__("select")}
                                    value={config[1].location.toString()}
                                    onValue={(v) => {
                                        on_config(util.get_updated_union(config, {location: parseInt(v)}));
                                    }} /> :
                                <InputText value={translate_meter_location(fixed_location)} />)}
                    </FormRow>);

                return edit_children;
            },
        },
    };
}

export function init() {
}
