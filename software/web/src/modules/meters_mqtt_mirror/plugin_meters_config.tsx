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
import { MeterLocation } from "../meters/meter_location.enum";
import { get_meter_location_items } from "../meters/meter_location";
import { MeterConfig } from "../meters/types";
import { InputText } from "../../ts/components/input_text";
import { FormRow } from "../../ts/components/form_row";
import * as API from "../../ts/api";
import { Switch          } from "../../ts/components/switch";
import { InputSelect     } from "../../ts/components/input_select";

export type MqttMirrorMetersConfig = [
    MeterClassID.MqttMirror,
    {
        display_name: string;
        location: number;
        auto: boolean;
        meter_path: string;
        value_ids: string;
    },
];

export function init() {
    return {
        [MeterClassID.MqttMirror]: {
            name: () => __("meters_mqtt_mirror.content.meter_class"),
            new_config: () => [MeterClassID.MqttMirror, {display_name: "", location: MeterLocation.Unknown, auto: true, meter_path: "source/abc/meters/0", value_ids: ""}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: MqttMirrorMetersConfig, on_config: (config: MqttMirrorMetersConfig) => void): ComponentChildren => {
                return [
                    <FormRow label={__("meters_mqtt_mirror.content.config_display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {display_name: v}));
                            }}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_mqtt_mirror.content.config_location")}>
                        <InputSelect
                            required
                            items={get_meter_location_items()}
                            placeholder={__("select")}
                            value={config[1].location.toString()}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {location: parseInt(v)}));
                            }} />
                    </FormRow>,
                    <FormRow label={__("meters_mqtt_mirror.content.auto")}>
                        <Switch desc={__("meters_mqtt_mirror.content.auto_desc")}
                            checked={config[1].auto}
                            disabled={true}
                            onClick={() => {
                                on_config(util.get_updated_union(config, {auto: !config[1].auto}));
                            }}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_mqtt_mirror.content.meter_path")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].meter_path}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {meter_path: v}));
                            }}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_mqtt_mirror.content.value_ids")}>
                        <InputText
                            disabled={true}
                            maxLength={64}
                            value={config[1].value_ids}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {value_ids: v}));
                            }}
                        />
                    </FormRow>,
                ];
            },
        },
    };
}
