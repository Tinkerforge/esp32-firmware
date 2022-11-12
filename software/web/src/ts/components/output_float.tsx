/* esp32-firmware
 * Copyright (C) 2022 Erik Fleckstein <erik@tinkerforge.com>
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

import * as util from "../util";

import { h, Context } from "preact";
import {useContext} from "preact/hooks";

interface OutputFloatProps {
    idContext?: Context<string>
    value: number
    digits: 0|1|2|3
    scale: number
    unit: string
}

function flt2Input(x: number, digits: number) {
    // Firefox does not localize numbers with a fractional part correctly.
    // OTOH Webkit based browsers (correctly) expect setting the value to a non-localized number.
    // Unfortunately, setting the value to a localized number (i.e. with , instead of . for German)
    // does not raise an exception, instead only a warning on the console is shown.
    // So to make everyone happy, we use user agent detection.
    return navigator.userAgent.indexOf("Gecko/") >= 0
                ? util.toLocaleFixed(x, digits)
                : (x).toFixed(digits);
}

export function OutputFloat(props: OutputFloatProps) {
    let pow10 = Math.pow(10, props.scale);

    let val = flt2Input(props.value / pow10, props.digits);

    let pad_right = "padding-right: min(" +
        "calc(100% - 2px " + // border
                  "- .75rem " + // left padding
                  "- 4rem " + // unit
                  `- ${flt2Input(props.value / pow10, 0).length}ch` + // digits before decimal separator
        `), calc(${props.digits == 0 ? 4 : (3-props.digits)}ch + .75rem));`;

    return (
        <div class="input-group">
            <input class="form-control no-spin text-right text-monospace"
                    style={pad_right}
                       id={props.idContext ? useContext(props.idContext) : undefined}
                       type="text"
                       disabled
                       value={val}/>
            <div class="input-group-append">
                <div class="form-control input-group-text" style="width: 4rem;">
                    {this.props.unit}
                </div>

            </div>
        </div>
    );
}
