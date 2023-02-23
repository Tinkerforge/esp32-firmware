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

import { h, Context, Fragment } from "preact";
import {useContext, useState} from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";

import * as util from "../../ts/util";

interface InputTimeProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>, "id" | "type" | "onInput" | "value"> {
    idContext?: Context<string>
    value: [number, number];
    onValue: (hours: number, minutes: number) => void
}

export function InputTime(props: InputTimeProps) {
    let {idContext, value, onValue, ...p} = props;

    const [inputInFlight, setInputInFlight] = useState<string | null>(null);
    let propValue = util.leftPad(value[0], 0, 2) + ':' + util.leftPad(value[1], 0, 2);
    let v = inputInFlight === null ? propValue : inputInFlight;

    const sendInFlight = () => {
        if (inputInFlight === null)
            return;

        if (inputInFlight === "") {
            return;
        }

        let splt = inputInFlight.split(/:/g).map(x => parseInt(x))

        let h = splt.length >= 1 ? splt[0] : 0;
        let m = splt.length >= 2 ? splt[1] : 0;
        onValue(h, m);
    }

    return (
        <input  min="00:00"
                max="23:59"
                class="form-control"
                {...p}
                type="time"
                value={v}
                onInput={(e) => setInputInFlight((e.target as HTMLInputElement).value)}
                onKeyDown={(e: KeyboardEvent) => {
                    if (e.key == 'Enter')
                        sendInFlight();
                }}
                onfocusout={() => sendInFlight()} />
    );
}
