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

import { h, Component, createContext, Context, VNode, cloneElement, toChildArray, Fragment, ComponentChildren } from "preact";
import { Button, Collapse } from "react-bootstrap";
import { HelpCircle } from "react-feather";

export interface FormRowProps {
    label: ComponentChildren
    label_muted?: ComponentChildren
    // Don't use ComponentChildren here: We want to pass in the idContext. This only works on VNodes.
    children: VNode | VNode[]
    labelColClasses?: string
    contentColClasses?: string
    hidden?: boolean
    label_prefix ?: VNode
    label_infix ?: VNode
    label_suffix ?: VNode
    help?: ComponentChildren
}

let id_counter = 0;

export class FormRow extends Component<FormRowProps, {help_expanded: boolean}> {
    idContext: Context<string>;
    id: string;

    constructor() {
        super();
        this.id = "formrow" + id_counter;
        this.idContext = createContext(this.id);
        ++id_counter;
    }

    render(props: FormRowProps, state: {help_expanded: boolean}) {
        let inner = <>{(toChildArray(props.children) as VNode[]).map(c => cloneElement(c, {idContext: this.idContext}))}</>;
        if (props.contentColClasses === undefined || props.contentColClasses !== "")
            inner = <div class={props.contentColClasses === undefined ? "col-lg-9" : props.contentColClasses}>
                {inner}
                {props.help ? <Collapse in={state.help_expanded}>
                                <div>{/*Empty div to fix choppy animation. See https://react-bootstrap-v4.netlify.app/utilities/transitions/#collapse*/}
                                    <div class="card">
                                        <div class="card-body p-3">
                                            {props.help}
                                        </div>
                                    </div>
                                </div>
                              </Collapse>
                            : <></>}
            </div>


        return (
            <div class="form-group row" hidden={props.hidden == undefined ? false : props.hidden}>
                <label for={this.id} class={"col-form-label " + (props.labelColClasses === undefined ? "col-lg-3" : props.labelColClasses)}>
                    <div class="row mx-lg-0">
                        <div class="col px-lg-0">
                    {props.label_prefix ? props.label_prefix : <></>}
                    {props.label ? <span class={"form-label" + (props.label_muted && !props.label_infix ? " pr-2" : "")}>{props.label}</span> : ""}
                    {props.label_infix ? props.label_infix : <></>}
                    {props.label_muted ? <span class="text-muted">{props.label_muted}</span> : ""}
                    {props.label_suffix ? props.label_suffix : <></>}
                    </div>
                    {props.help ? <span class="col-auto px-lg-0" onClick={() => this.setState({help_expanded: !state.help_expanded})}><HelpCircle {...{class:"btn-outline-secondary", style:"border-radius: 50%;"} as any}/></span> : <></>}
                    </div>
                </label>
                {inner}
            </div>
        );
    }
}
