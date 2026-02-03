/* esp32-firmware
 * Copyright (C) 2026 Matthias Bolte <matthias@tinkerforge.com>
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
import { useId, useContext } from "preact/hooks";
import { register_id_context_component_type } from "./form_row";

interface OutputTextProps {
    idContext?: Context<string>;
    value: string;
    suffix?: string;
    small?: boolean;
    class?: string;
}

export function OutputText(props: OutputTextProps) {
    const id = !props.idContext ? useId() : useContext(props.idContext);

    return (
        <div class={"input-group" + (props.small ? " input-group-sm" : "")}>
            <input class={"form-control" + (props.small ? " form-control-sm" : "") + " no-spin text-end font-monospace " + (props.class ? props.class : "")}
                   id={id}
                   type="text"
                   readonly
                   value={props.value}/>
            {props.suffix !== undefined ?
                <span class={"input-group-text" + (props.small ? " input-group-text-sm" : "")}>
                    {props.suffix}
                </span> : undefined}
        </div>
    );
}

register_id_context_component_type(OutputText);
