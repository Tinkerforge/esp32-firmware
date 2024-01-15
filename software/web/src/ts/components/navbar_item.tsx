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

import * as API from "../api";
import * as util from "../util";
import { h, Component } from "preact";

interface NavbarItemProps {
    name: string;
    module?: string;
    title: string;
    symbol: h.JSX.Element;
    hidden?: boolean;
}

interface NavbarItemState {
    hidden?: boolean;
}

export class NavbarItem extends Component<NavbarItemProps, NavbarItemState> {
    constructor(props: NavbarItemProps) {
        super(props);

        this.state = {
            hidden: undefined
        } as any;

        if (props.module !== undefined) {
            util.addApiEventListener("info/modules", () => {
                this.setState({hidden: !API.hasModule(props.module)});
            });
        }
    }

    render() {
        // look at module state first, if module was given
        let hidden: boolean = this.state.hidden;

        if (this.props.hidden === true) {
            // forced hide overrides module state
            hidden = true;
        }
        else if (this.props.hidden === false && hidden === undefined) {
            // forced show does not override module state
            hidden = false;
        }

        if (hidden === undefined) {
            // hidden by default
            hidden = true;
        }

        return (
            <li class="nav-item">
                <a id={`sidebar-${this.props.name}`} class="nav-link" data-toggle="tab" role="tab" aria-controls={this.props.name} aria-selected="true" href={`#${this.props.name}`} hidden={hidden}>
                    {this.props.symbol}
                    <span style="margin-left: 8px;">{this.props.title}</span>
                </a>
            </li>
        );
    }
}
