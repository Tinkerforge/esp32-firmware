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

import * as util from "../util";
import { h, Context, Fragment, ComponentChildren } from "preact";
import { useId, useContext } from "preact/hooks";
import { JSX } from 'preact';
import { register_id_context_component_type } from "./form_row";

type IpVersion = "v4" | "v6";

interface InputIPProps extends Omit<JSX.InputHTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "minLength" | "maxLength" | "size" | "pattern" | "onInput"> {
    idContext?: Context<string>
    onValue: (value: string) => void
    invalidFeedback: ComponentChildren
    moreClasses?: string[]
    ipVersion?: IpVersion
    allowCidr?: boolean
}

export function InputIP(props: InputIPProps) {
    const id = !props.idContext ? useId() : useContext(props.idContext);
    const ipv6 = props.ipVersion === "v6";
    const allowCidr = props.allowCidr === true;
    
    // Adjust lengths and patterns based on CIDR support
    const minLength = ipv6 ? 2 : 7;
    let maxLength = ipv6 ? 45 : 15;
    let size = ipv6 ? 45 : 15;
    
    // If CIDR is allowed, increase max length to accommodate "/128" or "/32"
    if (allowCidr) {
        maxLength = ipv6 ? 49 : 18;  // IPv6: "xxxx:.../128" (45+4), IPv4: "xxx.xxx.../32" (15+3)
        size = ipv6 ? 49 : 18;
    }
    
    // Build pattern - allow either plain IP or IP with CIDR if allowCidr is true
    let pattern: string;
    if (allowCidr) {
        if (ipv6) {
            pattern = `^(?:${util.IPV6_ADDRESS_PATTERN}|${util.IPV6_CIDR_PATTERN})$`;
        } else {
            pattern = `^(?:${util.IPV4_ADDRESS_PATTERN}|${util.IPV4_CIDR_PATTERN})$`;
        }
    } else {
        pattern = `^${ipv6 ? util.IPV6_ADDRESS_PATTERN : util.IPV4_ADDRESS_PATTERN}$`;
    }
    
    return (<>
        <input class={"form-control" + (props.moreClasses? " " + props.moreClasses.join(" ") : "")}
               id={id}
               type="text"
               minLength={minLength}
               maxLength={maxLength}
               size={size}
               pattern={pattern}
               onInput={(e) => props.onValue((e.target as HTMLInputElement).value)}
               {...props}/>
        <div class="invalid-feedback">{props.invalidFeedback}</div>
    </>);
}

register_id_context_component_type(InputIP);
