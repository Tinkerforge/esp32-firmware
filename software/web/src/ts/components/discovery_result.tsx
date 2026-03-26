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

import { h, Component, ComponentChildren, Fragment, ComponentChild, MouseEventHandler, toChildArray, VNode } from "preact";
import { useRef } from "preact/hooks";
import { Button, ListGroup, ListGroupItem } from "react-bootstrap";
import { Plus } from "react-feather";

export interface DiscoveryResultItemProps {
    key: any;
    type?: "button" | "submit" | "reset";
    title: ComponentChildren;
    children?: ComponentChildren;
    error?: ComponentChildren;
    onClick?: MouseEventHandler<HTMLButtonElement>;
    labelAdd: ComponentChildren;

}

export function DiscoveryResultItem(props: DiscoveryResultItemProps) {
    return <ListGroupItem key={props.key} className="p-2">
        <div class="row">
            <div class="col">
                <div class="mb-2">{props.title}</div>
                {props.children}
            </div>
            <div class="col-auto">
                {props.error ? <div class="d-flex align-items-center h-100">{props.error}</div>
                    : props.onClick ?
                         <Button type={props.type ?? 'button'} variant="primary" className="h-100" onClick={(e) => {
                            if (props.error)
                                e.preventDefault();
                            else if (props.onClick)
                                props.onClick(e);
                            }}>
                                {props.labelAdd}
                            </Button>
                        : null
                }
            </div>
        </div>
    </ListGroupItem>;
}

export function DiscoveryResultGroup(props: {children: VNode<DiscoveryResultItemProps> | VNode<DiscoveryResultItemProps>[]}) {
    if (!Array.isArray(props.children))
        return <ListGroup>{props.children}</ListGroup>

    return <ListGroup>
        {props.children.sort((a, b) => {
            if (!a.props.error && b.props.error)
                return -1;
            if (a.props.error && !b.props.error)
                return 1;
            return 0;
        })}
    </ListGroup>
}
