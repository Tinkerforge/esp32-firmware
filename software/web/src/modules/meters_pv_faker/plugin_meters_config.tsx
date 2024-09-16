/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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
import { MeterConfig  } from "../meters/types";
import { FormRow     } from "../../ts/components/form_row";
import { InputText   } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";

export type PvFakerMetersConfig = [
    MeterClassID.PvFaker,
    {
        display_name: string;
        topic: string;
        limiter_topic: string;
        peak_power: number;
        zero_at_lux: number;
        peak_at_lux: number;
    },
];

export function init() {
    return {
        [MeterClassID.PvFaker]: {
            name: __("meters_pv_faker.content.meter_class"),
            new_config: () => [MeterClassID.PvFaker, {display_name: "PV Faker", topic: "esp32/UKK/ambient_light/state", peak_power: 30000, zero_at_lux: 100, peak_at_lux: 105000}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: PvFakerMetersConfig, on_config: (config: PvFakerMetersConfig) => void): ComponentChildren => {
                return [
                    <FormRow label={__("meters_pv_faker.content.config_display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {on_config(util.get_updated_union(config, {display_name: v}));}}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_pv_faker.content.topic")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].topic}
                            onValue={(v) => {on_config(util.get_updated_union(config, {topic: v}));}}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_pv_faker.content.limiter_topic")}>
                        <InputText
                            maxLength={32}
                            value={config[1].limiter_topic}
                            onValue={(v) => {on_config(util.get_updated_union(config, {limiter_topic: v}));}}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_pv_faker.content.peak_power")}>
                        <InputNumber
                            required
                            unit="W"
                            value={config[1].peak_power}
                            onValue={(v) => {on_config(util.get_updated_union(config, {peak_power: v}));}}
                            min={1}
                            max={200000}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_pv_faker.content.zero_at_lux")}>
                        <InputNumber
                            required
                            unit="lux"
                            value={config[1].zero_at_lux}
                            onValue={(v) => {on_config(util.get_updated_union(config, {zero_at_lux: v}));}}
                            min={0}
                            max={200000}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_pv_faker.content.peak_at_lux")}>
                        <InputNumber
                            required
                            unit="lux"
                            value={config[1].peak_at_lux}
                            onValue={(v) => {on_config(util.get_updated_union(config, {peak_at_lux: v}));}}
                            min={0}
                            max={200000}
                        />
                    </FormRow>,
                ];
            },
        },
    };
}
