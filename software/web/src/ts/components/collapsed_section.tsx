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

import { h, Component, Fragment, ComponentChildren } from "preact";
import { __ } from "../../ts/translation";
import { Button, Collapse } from "react-bootstrap";
import { FormSeparator } from "./form_separator";

interface CollapsedSectionProps {
    heading?: string;
    modal?: boolean;
    children: ComponentChildren;
}

interface CollapsedSectionState {
    visible: boolean;
}

export class CollapsedSection extends Component<CollapsedSectionProps, CollapsedSectionState> {
    render() {
        return <>
            <FormSeparator heading={this.props.heading || __("component.collapsed_section.heading")} headingClass={this.props.modal ? "mt-2 form-label" : "mt-1"} rowClass={this.props.modal ? "gx-0" : undefined}>
                <Button variant="primary" className="mb-2" onClick={() => this.setState({visible: !this.state.visible})}>
                    {this.state.visible ? __("component.collapsed_section.hide") : __("component.collapsed_section.show")}
                </Button>
            </FormSeparator>
            <Collapse in={this.state.visible}>
                <div> {/* Empty div to fix choppy animation. See https://react-bootstrap-v4.netlify.app/utilities/transitions/#collapse */}
                    {this.props.children}
                </div>
            </Collapse></>;
    }
}
