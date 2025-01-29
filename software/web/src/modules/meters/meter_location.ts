/* esp32-firmware
 * Copyright (C) 2025 Matthias Bolte <matthias@tinkerforge.com>
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

import { __ } from "../../ts/translation";
import { MeterLocation } from "./meter_location.enum";

export function translate_meter_location(location: MeterLocation): string {
    switch (location) {
    case MeterLocation.Other:    return __("meters.content.location_other");
    case MeterLocation.Charger:  return __("meters.content.location_charger");
    case MeterLocation.Inverter: return __("meters.content.location_inverter");
    case MeterLocation.Grid:     return __("meters.content.location_grid");
    case MeterLocation.Battery:  return __("meters.content.location_battery");
    case MeterLocation.Load:     return __("meters.content.location_load");
    }

    return "<unknown>";
}

export function get_meter_location_items(): [string, string][] {
    return [
        //[MeterLocation.Charger.toString(), translate_meter_location(MeterLocation.Charger)],
        [MeterLocation.Inverter.toString(), translate_meter_location(MeterLocation.Inverter)],
        [MeterLocation.Grid.toString(), translate_meter_location(MeterLocation.Grid)],
        [MeterLocation.Battery.toString(), translate_meter_location(MeterLocation.Battery)],
        [MeterLocation.Load.toString(), translate_meter_location(MeterLocation.Load)],
        [MeterLocation.Other.toString(), translate_meter_location(MeterLocation.Other)],
    ];
}
