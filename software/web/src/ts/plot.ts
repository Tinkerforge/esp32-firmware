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

import * as util from "./util";

// https://seaborn.pydata.org/tutorial/color_palettes.html#qualitative-color-palettes
// sns.color_palette("tab10")
const strokes = [
    'rgb(  0, 123, 255)', // use bootstrap blue instead of tab10 blue, to avoid subtle conflict between plot and button blue
    'rgb(255, 127,  14)',
    'rgb( 44, 160,  44)',
    'rgb(214,  39,  40)',
    'rgb(148, 103, 189)',
    'rgb(140,  86,  75)',
    'rgb(227, 119, 194)',
    'rgb(127, 127, 127)',
    'rgb(188, 189,  34)',
    'rgb( 23, 190, 207)',
];

const fills = [
    'rgb(  0, 123, 255, 0.1)', // use bootstrap blue instead of tab10 blue, to avoid subtle conflict between plot and button blue
    'rgb(255, 127,  14, 0.1)',
    'rgb( 44, 160,  44, 0.1)',
    'rgb(214,  39,  40, 0.1)',
    'rgb(148, 103, 189, 0.1)',
    'rgb(140,  86,  75, 0.1)',
    'rgb(227, 119, 194, 0.1)',
    'rgb(127, 127, 127, 0.1)',
    'rgb(188, 189,  34, 0.1)',
    'rgb( 23, 190, 207, 0.1)',
];

let color_cache: {[id: string]: {stroke: string, fill: string}} = {};
let color_cache_next: {[id: string]: number} = {};

export function get_color(group: string, name: string)
{
    let key = group + '-' + name;

    if (!color_cache[key]) {
        if (!util.hasValue(color_cache_next[group])) {
            color_cache_next[group] = 0;
        }

        color_cache[key] = {
            stroke: strokes[color_cache_next[group] % strokes.length],
            fill: fills[color_cache_next[group] % fills.length],
        };

        color_cache_next[group]++;
    }

    return color_cache[key];
}
