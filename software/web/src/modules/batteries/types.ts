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

import * as API from "../../ts/api";
import { ComponentChildren } from "preact";

export type BatteryConfig = API.getType["batteries/0/config"];

export type BatteryConfigPlugin = {
    name: () => string
    new_config: () => BatteryConfig
    clone_config: (config: BatteryConfig) => BatteryConfig
    get_edit_children: (config: BatteryConfig, on_config: (config: BatteryConfig) => void) => ComponentChildren
    get_extra_rows?: (meter_slot: number) => ComponentChildren
    hide?: () => Promise<void>
};
