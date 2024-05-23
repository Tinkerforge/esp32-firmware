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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { __ } from "../../ts/translation";
import { METERS_SLOTS } from "../../build";

import { MeterClassID } from "../meters/meter_class_id.enum";
import { MeterConfig  } from "../meters/types";

import { h, ComponentChildren } from "preact";
import { Collapse    } from "react-bootstrap";
import { FormRow     } from "../../ts/components/form_row";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { InputText   } from "../../ts/components/input_text";

export type MetaMetersConfig = [
    MeterClassID.Meta,
    {
        display_name: string;
        mode: number;
        source_meter_a: number;
        source_meter_b: number;
        constant: number;
    },
];

export function init() {
    return {
        [MeterClassID.Meta]: {
            name: __("meters_meta.content.meter_class"),
            new_config: () => [MeterClassID.Meta, {display_name: "Meta", mode: 0, source_meter_a: 0, source_meter_b: 1, constant: 0}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: MetaMetersConfig, on_config: (config: MetaMetersConfig) => void): ComponentChildren => {
                let meter_names: [string, string][] = [];
                for (let i = 0; i < METERS_SLOTS; i++) {
                    const meter = API.get_unchecked(`meters/${i}/config`);
                    if (meter[1]) {
                        meter_names.push([i.toString(), meter[1].display_name]);
                    }
                }

                return [
                    <FormRow label={__("meters_meta.content.config_display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {on_config(util.get_updated_union(config, {display_name: v}));}}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_meta.content.mode")}>
                        <InputSelect
                            items={[
                                ["0", __("meters_meta.content.mode_sum")],
                                ["1", __("meters_meta.content.mode_diff")],
                                ["2", __("meters_meta.content.mode_add")],
                                ["3", __("meters_meta.content.mode_mul")],
                                ["4", __("meters_meta.content.mode_pf2current")],
                            ]}
                            value={config[1].mode}
                            onValue={(v) => {on_config(util.get_updated_union(config, {mode: parseInt(v)}));}}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_meta.content.source_meter_a")}>
                        <InputSelect
                            items={meter_names}
                            value={config[1].source_meter_a}
                            onValue={(v) => {on_config(util.get_updated_union(config, {source_meter_a: parseInt(v)}));}}
                        />
                    </FormRow>,
                    <Collapse in={config[1].mode == 0 || config[1].mode == 1}>
                        <div>
                            <FormRow label={__("meters_meta.content.source_meter_b")}>
                                <InputSelect
                                    items={meter_names}
                                    value={config[1].source_meter_b}
                                    onValue={(v) => {on_config(util.get_updated_union(config, {source_meter_b: parseInt(v)}));}}
                                />
                            </FormRow>
                        </div>
                    </Collapse>,
                    <Collapse in={config[1].mode == 2 || config[1].mode == 3}>
                        <div>
                            <FormRow label={__("meters_meta.content.constant")}>
                                <InputNumber
                                    required
                                    value={config[1].constant}
                                    onValue={(v) => {on_config(util.get_updated_union(config, {constant: v}));}}
                                />
                            </FormRow>
                        </div>
                    </Collapse>,
                ];
            },
        },
    };
}
