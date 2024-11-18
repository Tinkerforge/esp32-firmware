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

import { h, Component, ComponentChildren } from "preact";
import { Collapse } from "react-bootstrap";
import { HelpCircle } from "react-feather";

interface FormSeparatorProps {
    heading?: string;
    colClasses?: string;
    first?: boolean;
    extraClasses?: string;
    children?: ComponentChildren;
    help?: ComponentChildren;
}

export class FormSeparator extends Component<FormSeparatorProps, {}> {
    render(props: FormSeparatorProps, state: {help_expanded: boolean}) {
        return (
            <div>
            <div class={"row " + (state.help_expanded ? "mb-1 " : "mb-3 ") + (!props.first ? "pt-3" : "pt-0") + " " + (props.extraClasses === undefined ? "" : props.extraClasses)}>
                <div class={((!state.help_expanded) ? "border-bottom " : "")  + (props.colClasses === undefined ? "col" : props.colClasses)}>
                    <div class="row align-items-center">
                        {props.heading && <span class="h3 col-auto">{props.heading}</span>}
                        {props.help && <span class="col" style="margin-bottom: 0.5rem;" onClick={() => this.setState({help_expanded: !state.help_expanded})}><HelpCircle {...{class: (state.help_expanded ? "btn-dark" : "btn-outline-secondary"), style:"border-radius: 50%; transition: .35s; vertical-align:"} as any}/></span>}
                        {props.children}
                    </div>
                </div>
            </div>
            <Collapse in={state.help_expanded}>
                <div>{/*Empty div to fix choppy animation. See https://react-bootstrap-v4.netlify.app/utilities/transitions/#collapse*/}
                    <div class="pb-3">
                        <div class="card">
                            <div class="card-body p-3 form-row-help" style="background: #ffffe7;">
                                {props.help}
                            </div>
                        </div>
                    </div>
                </div>
            </Collapse>
            </div>
        );
    }
}
