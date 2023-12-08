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
import { useContext } from "preact/hooks";

interface OutputFloatProps {
    idContext?: Context<string>;
    value: number;
    digits: 0 | 1 | 2 | 3;
    scale: number;
    unit: string;
    maxFractionalDigitsOnPage?: number;
    maxUnitLengthOnPage?: number;
    small?: boolean;
}

export function OutputFloat(props: OutputFloatProps) {
    const id = !props.idContext ? util.useId() : useContext(props.idContext);
    let pow10 = Math.pow(10, props.scale);

    let val = "";
    let val_0 = "";

    if (util.hasValue(props.value)) {
        val = util.toLocaleFixed(props.value / pow10, props.digits);
        val_0 = util.toLocaleFixed(props.value / pow10, 0);
    }

    let maxFracDigits = props.maxFractionalDigitsOnPage === undefined ? 3 : props.maxFractionalDigitsOnPage;
    let maxUnitLength = props.maxUnitLengthOnPage === undefined ? (props.small ? 1.75 : 2.5) : props.maxUnitLengthOnPage; // Hand-tuned at the moment to fit kvarh

    let pad_right = "padding-right: min(" +
        "calc(100% - 2px " + // border
                  "- .75rem " + // left padding
                  `- ${maxUnitLength}rem ` + // unit
                  `- ${val_0.length}ch` + // digits before decimal separator
        `), calc(${props.digits == 0 ? (maxFracDigits > 0 ? maxFracDigits + 1 : 0) : (maxFracDigits-props.digits)}ch + .75rem));`;

    return (
        <div class={"input-group" + (props.small ? " input-group-sm" : "")}>
            <input class={"form-control" + (props.small ? " form-control-sm" : "") + " no-spin text-right text-monospace"}
                    style={pad_right}
                       id={id}
                       type="text"
                       disabled
                       value={val}/>
            <div class="input-group-append">
                <div class={"form-control" + (props.small ? " form-control-sm" : "") + " input-group-text"} style={`width: ${maxUnitLength + 1.5}rem;`}>
                    {this.props.unit}
                </div>
            </div>
        </div>
    );
}
