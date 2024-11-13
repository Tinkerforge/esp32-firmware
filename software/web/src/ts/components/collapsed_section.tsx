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
import { Button, Collapse } from "react-bootstrap";
import { __ } from "../../ts/translation";

interface CollapsedSectionProps {
    heading?: string;
    children: ComponentChildren;
}

interface CollapsedSectionState {
    show: boolean;
}

export class CollapsedSection extends Component<CollapsedSectionProps, CollapsedSectionState> {
    render(props: CollapsedSectionProps, state: Readonly<CollapsedSectionState>) {
        return (
            <>
            <div class="row mb-3 pt-3">
                <div class="col border-bottom d-flex justify-content-between">
                    <span class="h3">{props.heading || __("component.collapsed_section.heading")}</span>
                    <Button variant="primary"
                            className="mb-2"
                            onClick={() => this.setState({show: !state.show})}>
                                {state.show ? __("component.collapsed_section.hide") : __("component.collapsed_section.show")}
                    </Button>
                </div>
            </div>
            <Collapse in={state.show}>
                <div>
                    {props.children}
                </div>
            </Collapse>
            </>
        );
    }
}
