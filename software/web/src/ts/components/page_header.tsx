/* esp32-firmware
 * Copyright (C) 2022 Matthias Bolte <matthias@tinkerforge.com>
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
import * as util from "../util";

export interface PageHeaderProps {
    children?: ComponentChildren;
    title: ComponentChildren;
    titleClass?: string;
    childrenClass?: string;
}

export class PageHeader extends Component<PageHeaderProps, any> {
    render() {
        return (
            // this row/col combination is necessary here to create a div that is
            // full width to cover the page header shadow after the sm breakpoint
            <div class={"row mb-3 pt-3 " + (util.is_native_median_app() ? "sticky-top-app" : "sticky-under-top")}>
                <div class="col">
                    <div class="row d-flex flex-wrap justify-content-between pb-2 border-bottom tab-header-shadow">
                        <h1 class={"page-header " + (this.props.titleClass ? this.props.titleClass : "col")}>{this.props.title}</h1>
                        <div class={this.props.childrenClass ? this.props.childrenClass : "col-auto"}>
                            {this.props.children}
                        </div>
                    </div>
                </div>
            </div>
        );
    }
}
