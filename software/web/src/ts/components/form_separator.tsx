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

import { h, Component, ComponentChildren, Fragment } from "preact";
import { Collapse } from "react-bootstrap";
import { HelpCircle } from "react-feather";

interface FormSeparatorProps {
    heading?: string;
    headingClass?: string;
    rowClass?: string;
    help?: ComponentChildren;
    first?: boolean;
    modal?: boolean;
    children?: ComponentChildren;
}

interface FormSeparatorState {
    help_visible: boolean;
}

export class FormSeparator extends Component<FormSeparatorProps, FormSeparatorState> {
    render() {
        return <>
            <div class={"row mb-3 border-bottom" + (this.props.heading && !this.props.first ? " pt-3 " : " pt-0 ") + (this.props.rowClass ? this.props.rowClass : "")}>
                {this.props.heading && <div class="col-auto"><h3 class={this.props.headingClass}>{this.props.heading}</h3></div>}
                {this.props.help && <span class="col-auto" style="padding-top: 5px;" onClick={() => this.setState({help_visible: !this.state.help_visible})}><HelpCircle {...{class: "help-circle" + (this.state.help_visible ? " help-circle-expanded" : ""), style: "cursor: pointer;"} as any}/></span>}
                <div class="col" />
                {this.props.children && <div class="col-auto">
                    {this.props.children}
                </div>}
            </div>
            <Collapse in={this.state.help_visible}>
                <div> {/* Empty div to fix choppy animation. See https://react-bootstrap-v4.netlify.app/utilities/transitions/#collapse */}
                    <div class="mb-3">
                        <div class="card">
                            <div class="card-body p-3 form-row-help">
                                {this.props.help}
                            </div>
                        </div>
                    </div>
                </div>
            </Collapse></>;
    }
}
