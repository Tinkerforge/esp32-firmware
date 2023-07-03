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

import { h } from "preact";
import { InputSelect, InputSelectProps } from "./input_select";

import { unparseIP } from "../util";
import { useMemo } from "preact/hooks";

export interface InputSubnetProps extends Omit<InputSelectProps, "items"> {
    minPrefixLength?: number
    maxPrefixLength?: number
}

export function InputSubnet(props: InputSubnetProps) {
    let {minPrefixLength = 8, maxPrefixLength = 31, ...rest} = props;

    const items = useMemo(() => {
            let result: [string, string][] = [];
            for(let i = maxPrefixLength; i >= minPrefixLength; --i) {
                let x = i == 32 ? 0xFFFFFFFF : ~(0xFFFFFFFF >>> (i));
                result.push([unparseIP(x), "/" + i + " (" + unparseIP(x) + ")"]);
            }
            return result;
        }, [minPrefixLength, maxPrefixLength]);

    return (
        <InputSelect items={items}
                    {...rest}
                />);
}
