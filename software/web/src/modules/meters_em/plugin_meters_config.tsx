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

import { MeterClassID } from "../meters/meters_defs";
import { MeterConfig } from "../meters/types";
import { TableModalRow } from "../../ts/components/table";

export type EMMetersConfig = [
    MeterClassID.EM,
    {
        display_name: string;
    },
];

export function init() {
    return {
        [MeterClassID.EM]: {
            clone: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_rows: (config: MeterConfig, on_value: (key: string, value: any) => void): TableModalRow[] => {
                return []
            },
            get_add_rows: (config: MeterConfig, on_value: (key: string, value: any) => void): TableModalRow[] => {
                return []
            },
        },
    };
}
