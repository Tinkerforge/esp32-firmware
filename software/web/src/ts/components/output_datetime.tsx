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
import { useId, useContext } from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";

import * as util from "../../ts/util";

interface OutputDatetimeProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>, "value" | "class" | "id" | "type" | "onInput" | "disabled"> {
    idContext?: Context<string>;
    date: Date;
    onClick?: () => void;
    buttonText?: string;
    disabled?: boolean;
    invalidDateText?: string;
}

export function OutputDatetime(props: OutputDatetimeProps) {
    const id = !props.idContext ? useId() : useContext(props.idContext);

    let inner = <input class={"form-control " + props.className}
                    id={id}
                    type="datetime-local"
                    step={1}
                    disabled={true}
                    value={util.toIsoString(props.date)}
                    required
                    />;

    if (isNaN(props.date.getTime()) && props.invalidDateText)
        inner = <input class={"form-control " + props.className}
            id={id}
            type="text"
            disabled={true}
            value={props.invalidDateText}
            required
            />;

    if (!props.onClick)
        return inner;

    return (
        <div class="input-group">
            {inner}
            <div class="input-group-append">
                <button class="btn btn-primary form-control rounded-right" type="button" onClick={props.onClick} disabled={props.disabled}>{props.buttonText}</button>
            </div>
        </div>

    );
}
