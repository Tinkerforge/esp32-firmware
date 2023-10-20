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
import { __ } from "../../ts/translation";
import { MeterClassID } from "../meters/meters_defs";
import { MeterConfig } from "../meters/types";
import { TableModalRow } from "../../ts/components/table";
import { InputText } from "../../ts/components/input_text";

export type EMMetersConfig = [
    MeterClassID.EnergyManager,
    {
        display_name: string;
    },
];

export function init() {
    return {
        [MeterClassID.EnergyManager]: {
            name: __("meters_em.content.meter_class"),
            init: () => [MeterClassID.EnergyManager, {display_name: ""}] as MeterConfig,
            clone: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_rows: (config: EMMetersConfig, on_value: (config: EMMetersConfig) => void): TableModalRow[] => {
                return [
                    {
                        name: __("meters_em.content.config_display_name"),
                        value: <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                config[1].display_name = v;
                                on_value(config);
                            }}/>
                    },
                ];
            },
        },
    };
}
