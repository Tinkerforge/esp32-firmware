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

import { h } from 'preact'
import { __, translate_unchecked } from "../../ts/translation";
import { MeterClassID } from "../meters/meters_defs";
import { MeterValueID } from "../meters/meter_value_id";
import { MeterConfig } from "../meters/types";
import { TableModalRow } from "../../ts/components/table";
import { InputText } from "../../ts/components/input_text";

export type PushAPIMetersConfig = [
    MeterClassID.PushAPI,
    {
        display_name: string;
        value_ids: number[];
    },
];

export function init() {
    return {
        [MeterClassID.PushAPI]: {
            name: __("meters_push_api.content.meter_class"),
            init: () => [MeterClassID.PushAPI, {display_name: "", value_ids: new Array<number>()}] as MeterConfig,
            clone: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_rows: (config: PushAPIMetersConfig, on_value: (config: PushAPIMetersConfig) => void): TableModalRow[] => {
                return [
                    {
                        name: __("meters_push_api.content.config_display_name"),
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
                        name: __("meters_push_api.content.config_value_ids"),
                        value: <select class="form-control custom-select" multiple>
                            {config[1].value_ids.map((value_id) => <option>{translate_unchecked(`meters.content.value_${value_id}`)}</option>)}
                        </select>
                    },
                ];
            },
        },
    };
}
