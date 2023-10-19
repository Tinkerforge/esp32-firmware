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

import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { MeterClassID } from "../meters/meters_defs";
import { MeterConfig } from "../meters/types";
import { TableModalRow } from "../../ts/components/table";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";

export type SunSpecMetersConfig = [
    MeterClassID.SunSpec,
    {
        display_name: string
        host: string,
        port: number,
        device_address: number,
        model_id: number,
    },
];

export function init() {
    return {
        [MeterClassID.SunSpec]: {
            name: __("meters_sun_spec.content.meter_class"),
            init: () => [MeterClassID.SunSpec, {display_name: "", host: "", port: 502, device_address: 0, model_id: 0}] as MeterConfig,
            clone: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_rows: (config: SunSpecMetersConfig, on_value: (config: SunSpecMetersConfig) => void): TableModalRow[] => {
                return [
                    {
                        name: __("meters_sun_spec.content.config_display_name"),
                        value: <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                config[1].display_name = v;
                                on_value(config);
                            }}/>
                    },
                    {
                        name: __("meters_sun_spec.content.config_host"),
                        value: <InputText
                            required
                            maxLength={64}
                            pattern="^[a-zA-Z0-9\-\.]+$"
                            value={config[1].host}
                            onValue={(v) => {
                                config[1].host = v;
                                on_value(config);
                            }}
                            invalidFeedback={__("meters_sun_spec.content.config_host_invalid")}/>
                    },
                    {
                        name: __("meters_sun_spec.content.config_port"),
                        value: <InputNumber
                            required
                            min={1}
                            max={65535}
                            value={config[1].port}
                            onValue={(v) => {
                                config[1].port = v;
                                on_value(config);
                            }}/>
                    },
                    {
                        name: __("meters_sun_spec.content.config_device_address"),
                        value: <InputNumber
                            required
                            min={0}
                            max={247}
                            value={config[1].device_address}
                            onValue={(v) => {
                                config[1].device_address = v;
                                on_value(config);
                            }}/>
                    },
                    {
                        name: __("meters_sun_spec.content.config_model_id"),
                        value: <InputNumber
                            required
                            min={0}
                            max={65535}
                            value={config[1].model_id}
                            onValue={(v) => {
                                config[1].model_id = v;
                                on_value(config);
                            }}/>
                    },
                ];
            },
        },
    };
}
