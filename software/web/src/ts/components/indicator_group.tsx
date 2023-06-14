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

import { h, VNode } from "preact";
import { Button, ButtonGroup } from "react-bootstrap";

type variant = "primary" | "secondary" | "success" |  "danger" | "warning" | "info" | "light" | "dark" | "link"

interface IndicatorProps {
    value: number,
    items: [variant, string | VNode][];
    class?: string
    vertical?: boolean
    style?: string
}

export function IndicatorGroup(props: IndicatorProps) {
    return (
        <ButtonGroup style={props.style != undefined ? props.style : ""} vertical={props.vertical} className={props.class !== undefined ? props.class : "flex-wrap w-100"}>
            {props.items.map((v, i) => typeof(v[1]) == "string" ? <Button disabled
                                               key={i}
                                               variant={(i == props.value ? "" : "outline-") + v[0]}
                                               dangerouslySetInnerHTML={{__html: v[1]}}>
                                        </Button> :
                                        <Button disabled
                                        key={i}
                                        variant={(i == props.value ? "" : "outline-") + v[0]}>
                                        {v[1]}
                                 </Button>)}
        </ButtonGroup>
    );
}
