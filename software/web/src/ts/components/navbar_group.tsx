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

import $ from "../jq";
import { h, Component, ComponentChildren, createRef } from "preact";
import { ChevronRight } from "react-feather";

interface NavbarGroupProps {
    children: ComponentChildren;
    name: string;
    title: string;
    symbol: h.JSX.Element;
    hidden?: boolean;
}

export class NavbarGroup extends Component<NavbarGroupProps, {}> {
    chevron_ref = createRef();

    constructor(props: NavbarGroupProps) {
        super(props);

        props.symbol.props.class = "col-auto";
    }

    componentDidMount() {
        // FIXME: Bootstrap 4.x only provides jQuery events. We need to port
        //        to Bootstrap 5.x before we can remove jQuery completly
        //        https://getbootstrap.com/docs/5.0/getting-started/javascript/
        $(`#${this.props.name}-group`).on('hide.bs.collapse', () => this.chevron_ref.current.classList.remove("rotated-chevron"));
        $(`#${this.props.name}-group`).on('show.bs.collapse', () => this.chevron_ref.current.classList.add("rotated-chevron"));
    }

    render() {
        return (
            <li class="nav-item">
                <div id={`sidebar-${this.props.name}-group`} class="nav-link row no-gutters d-flex-ni align-items-center" data-toggle="collapse" href={`#${this.props.name}-group`} role="button" aria-expanded="false" aria-controls={`${this.props.name}-group`} hidden={this.props.hidden === undefined ? true : this.props.hidden}>
                    {this.props.symbol}<span class="col" style="margin-left: 8px;">{this.props.title}</span><ChevronRight ref={this.chevron_ref} {...{class: "unrotated-chevron col-auto", style: "margin-right: 0px;"} as any} />
                </div>
                <div class="collapse nav-nested" id={`${this.props.name}-group`}>
                    <ul class="flex-column nav" style="padding-left: 32px">
                        {this.props.children}
                    </ul>
                </div>
            </li>
        );
    }
}
