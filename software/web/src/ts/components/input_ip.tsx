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

import { h, Context, Fragment, ComponentChildren } from "preact";
import { useId, useContext } from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";

interface InputIPProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "minLength" | "maxLength" | "size" | "pattern" | "onInput"> {
    idContext?: Context<string>
    onValue: (value: string) => void
    invalidFeedback: ComponentChildren
    moreClasses?: string[]
}

export function InputIP(props: InputIPProps) {
    const id = !props.idContext ? useId() : useContext(props.idContext);
    return (<>
        <input class={"form-control" + (props.moreClasses? " " + props.moreClasses.join(" ") : "")}
               id={id}
               type="text"
               minLength={7}
               maxLength={15}
               size={15}
               pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
               onInput={(e) => props.onValue((e.target as HTMLInputElement).value)}
               {...props}/>
        <div class="invalid-feedback">{props.invalidFeedback}</div>
    </>);
}
