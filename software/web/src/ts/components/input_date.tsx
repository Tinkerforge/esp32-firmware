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

import { h, Context } from "preact";
import {useContext} from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";

import * as util from "../../ts/util";

interface InputDateProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>, "value" | "class" | "id" | "type" | "onInput"> {
    idContext?: Context<string>
    date: Date
    onDate?: (value: Date) => void
}

export function InputDate(props: InputDateProps) {
    let id = props.idContext === undefined ? "" : useContext(props.idContext);

    const dateToValue = (date: Date) => {
        try {
            return util.leftPad(date.getFullYear(), 0, 4) + "-" + util.leftPad((date.getMonth() + 1), 0, 2) + "-" + util.leftPad(date.getDate(), 0, 2);
        } catch (e) {
            return "";
        }
    };

    return (
        <input class={"form-control " + props.className}
               id={id}
               type="date"
               onInput={props.onDate ? (e) => {
                    let timeString = (e.target as HTMLInputElement).value;
                    if (timeString == "")
                        return;

                    let [y, mIdx, d] = timeString.split(/-/g).map(x => parseInt(x));
                    props.onDate(new Date(y, mIdx - 1, d))
                } : undefined
               }
               disabled={!props.onDate}
               value={dateToValue(props.date)}/>
    );
}
