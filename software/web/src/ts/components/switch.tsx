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

import { h, JSX, Context, ComponentChildren } from "preact";
import { useId, useContext } from "preact/hooks";

export interface SwitchProps {
    idContext?: Context<string>;
    checked: boolean;
    desc?: ComponentChildren;
    onClick: JSX.MouseEventHandler<HTMLInputElement>;
    disabled?: boolean;
    className?: string;
}

export function Switch(props: SwitchProps) {
    const id = !props.idContext ? useId() : useContext(props.idContext);

    let inner = <div class={"borderless-form-control custom-control custom-switch "}>
            <input type="checkbox" class="custom-control-input" id={id} checked={props.checked} onClick={props.onClick} disabled={props.disabled}/>
            <label class="custom-control-label" for={id}>{props.desc}</label>
        </div>;

    if (props.className !== undefined && props.className != "")
        inner = <div class={props.className}>
                {inner}
            </div>;

    return inner;
}
