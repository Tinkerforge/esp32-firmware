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
import { InputSelect, InputSelectProps } from "./input_select";
import { unparseIP } from "../util";
import { useMemo } from "preact/hooks";
import { register_id_context_component_type } from "./form_row";

export interface InputSubnetProps extends Omit<InputSelectProps, "items"> {
    idContext?: Context<string>
    minPrefixLength?: number;
    maxPrefixLength?: number;
    ipVersion?: "v4" | "v6";
}

export function InputSubnet(props: InputSubnetProps) {
    let {minPrefixLength = 8, maxPrefixLength = 31, ipVersion = "v4", ...rest} = props;

    if (ipVersion === "v6") {
        if (props.minPrefixLength === undefined)
            minPrefixLength = 0;
        if (props.maxPrefixLength === undefined)
            maxPrefixLength = 128;
    }

    const items = useMemo(() => {
            let result: [string, string][] = [];
            if (ipVersion === "v6") {
                for (let i = maxPrefixLength; i >= minPrefixLength; --i) {
                    result.push(["/" + i, "/" + i]);
                }
            } else {
                for(let i = maxPrefixLength; i >= minPrefixLength; --i) {
                    let x = i == 32 ? 0xFFFFFFFF : ~(0xFFFFFFFF >>> (i));
                    result.push([unparseIP(x), "/" + i + " (" + unparseIP(x) + ")"]);
                }
            }
            return result;
        }, [ipVersion, minPrefixLength, maxPrefixLength]);

    return (
        <InputSelect items={items}
                    {...rest}
                />);
}

register_id_context_component_type(InputSubnet);
