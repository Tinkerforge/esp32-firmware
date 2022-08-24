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

interface InputTextProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput"> {
    idContext?: Context<string>
    onValue: (value: string) => void
}

export function InputText(props: InputTextProps) {
    let id = useContext(props.idContext);
    return (
        <input class="form-control"
               id={id}
               type="text"
               onInput={(e) => props.onValue((e.target as HTMLInputElement).value)}
               {...props}/>
    );
}
