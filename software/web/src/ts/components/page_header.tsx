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

import { h, Component, ComponentChildren, toChildArray } from "preact";
import * as util from "../util";

export interface PageHeaderProps {
    title: ComponentChildren;
    titleColClass?: string;
    childrenColClass?: string;
    rowClass?: string;
    small?: boolean;
    children?: ComponentChildren;
}

export class PageHeader extends Component<PageHeaderProps, {}> {
    render() {
        let has_children = toChildArray(this.props.children).length > 0;
        let row_class = "mb-3 pt-2" + (this.props.small ? " pt-sm-4" : " pt-sm-3");
        let title_class = "text-center text-sm-start" + (this.props.small ? " mt-1" : "");

        return (
            // this row/col combination is necessary here to create a div that is
            // full width to cover the page header shadow after the sm breakpoint
            <div class={"row " + row_class + (util.is_native_median_app() ? " sticky-top-app" : " sticky-under-top")}>
                <div class={"col border-bottom tab-header-shadow" + (this.props.small ? "" : " pb-0 pb-sm-2")}>
                    <div class="row">
                        <div class={this.props.titleColClass ? this.props.titleColClass : "col"}>
                            {this.props.small
                                ? <h3 class={title_class}>{this.props.title}</h3>
                                : <h1 class={"page-header " + title_class}>{this.props.title}</h1>
                            }
                        </div>
                        {has_children ?
                            <div class={(this.props.childrenColClass ? this.props.childrenColClass : "col-auto") + " mb-2"}>
                                {this.props.children}
                            </div> : undefined}
                    </div>
                </div>
            </div>
        );
    }
}
