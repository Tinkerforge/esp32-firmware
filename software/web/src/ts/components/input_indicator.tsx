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
import { Button } from "react-bootstrap";

type variant = "primary" | "secondary" | "success" | "warning" | "danger" | "light" | "link"

interface InputIndicatorProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput"> {
    idContext?: Context<string>
    onValue?: (value: string) => void
    variant: variant

    onReset?: () => void
    resetVariant?: variant
    resetText?: string
    resetHidden?: boolean
}

export function InputIndicator(props: InputIndicatorProps) {
    const id = !props.idContext ? useId() : useContext(props.idContext);

    let inner = (
        <input class={`form-control input-indicator input-indicator-${props.variant}`}
               id={id}
               type="text"
               onInput={props.onValue ? (e) => props.onValue((e.target as HTMLInputElement).value) : undefined}
               readonly={!props.onValue}
               {...props}/>
    );
    if (!props.onReset || props.resetHidden)
        return inner;

    return <div class="input-group">
        {inner}
        <div class="input-group-append">
            <Button variant={props.resetVariant} className="form-control rounded-right" type="button" onClick={props.onReset}>{props.resetText}</Button>
        </div>
    </div>

}
