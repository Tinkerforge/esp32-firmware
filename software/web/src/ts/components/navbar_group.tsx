/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

import { h, Component, ComponentChildren } from "preact";
import { ChevronRight } from "react-feather";
import Nav from 'react-bootstrap/Nav';
import Collapse from 'react-bootstrap/Collapse';

interface NavbarGroupProps {
    children: ComponentChildren;
    title: string;
    symbol: h.JSX.Element;
    hidden?: boolean; // default: true
}

interface NavbarGroupState {
    open: boolean;
}

export class NavbarGroup extends Component<NavbarGroupProps, NavbarGroupState> {
    constructor(props: NavbarGroupProps) {
        super(props);

        this.state = {
            open: false,
        } as any;
    }

    open() {
        this.setState({open: true});
    }

    render() {
        return (
            <Nav.Item as="li">
                <Nav.Link as="div" className="row mx-n1 d-flex-ni align-items-center" role="button" hidden={this.props.hidden === undefined ? true : this.props.hidden} onClick={() => {this.setState({open: !this.state.open});}}>
                    <span class="col-auto px-1">{this.props.symbol}</span><span class="col px-1">{this.props.title}</span><ChevronRight {...{class: "col-auto pl-1 pr-0 unrotated-chevron " + (this.state.open ? "rotated-chevron" : "")} as any} />
                </Nav.Link>
                <Collapse in={this.state.open}>
                    <div class="nav-nested">
                        <Nav as="ul" className="flex-column" style="padding-left: 32px">
                            {this.props.children}
                        </Nav>
                    </div>
                </Collapse>
            </Nav.Item>
        );
    }
}
