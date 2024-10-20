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

import * as API from "../../ts/api";
import { ComponentChildren } from "preact";

export type MeterConfig = API.getType["meters/0/config"];

export type MeterConfigPlugin = {
    name: () => string
    new_config: () => MeterConfig
    clone_config: (config: MeterConfig) => MeterConfig
    get_edit_children: (config: MeterConfig, on_config: (config: MeterConfig) => void) => ComponentChildren
    get_extra_rows?: (meter_slot: number) => ComponentChildren
    hide?: () => Promise<void>
};
