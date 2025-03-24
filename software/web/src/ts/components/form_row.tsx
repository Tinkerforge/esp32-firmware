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

import { h, Component, createContext, Context, VNode, toChildArray, Fragment, ComponentChildren } from "preact";
import { Collapse } from "react-bootstrap";
import { HelpCircle } from "react-feather";
import { useMemo } from "preact/hooks";

export interface FormRowProps {
    label: ComponentChildren;
    label_muted?: ComponentChildren;
    // Don't use ComponentChildren here: We want to pass in the idContext. This only works on VNodes.
    children: VNode | VNode[];
    labelColClasses?: string;
    contentColClasses?: string;
    hidden?: boolean;
    label_prefix?: VNode;
    label_infix?: VNode;
    label_suffix?: VNode;
    help?: ComponentChildren;
    error?: ComponentChildren;
    show_error?: boolean;
    warning?: ComponentChildren;
    show_warning?: boolean;
    small?: boolean;
    symbol?: h.JSX.Element;
    class?: string;
}

let id_counter = 0;

let component_types_using_id_context: any = [];

export function register_id_context_component_type(component_type: any) {
    component_types_using_id_context.push(component_type);
}

function walk(children: ComponentChildren): VNode<any> | null {
    for (let c of toChildArray(children)) {
        if (typeof(c) == "string" || typeof(c) == "number")
            continue;

        if (c.type == FormRow)
            continue;

        if (component_types_using_id_context.indexOf(c.type) >= 0)
            return c;

        let sub_result = walk(c.props.children)
        if (sub_result != null)
            return sub_result;
    }
    return null;
}

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
        let child_using_id_context = useMemo(() => walk(props.children), [props.children]);
        if (child_using_id_context != null) {
            child_using_id_context.props["idContext"] = this.idContext;
        }
        let inner = props.children;

        if (props.contentColClasses === undefined || props.contentColClasses !== "") {
            inner = <div class={props.contentColClasses === undefined ? "col-lg-8" : props.contentColClasses}>
                {inner}
                {props.error ? <Collapse in={props.show_error} ><div><div class="alert alert-danger mt-2 mb-0">{props.error}</div></div></Collapse> : <></>}
                {props.warning ? <Collapse in={props.show_warning} ><div><div class="alert alert-warning mt-2 mb-0">{props.warning}</div></div></Collapse> : <></>}
                {props.help ? <Collapse in={state.help_expanded} >
                                <div>{/*Empty div to fix choppy animation. See https://react-bootstrap-v4.netlify.app/utilities/transitions/#collapse*/}
                                    <div class="card mt-2">
                                        <div class="card-body p-3 form-row-help" style="background: #ffffe7;">
                                            {props.help}
                                        </div>
                                    </div>
                                </div>
                              </Collapse>
                            : <></>}
            </div>
        }

        let label_content = <div class="row mx-lg-0">
                                <div class={"col px-lg-0" + (props.symbol ? " d-flex-ni align-items-center" : "")}>
                                    {props.label_prefix ? props.label_prefix : undefined}
                                    {props.symbol ? <span class="col-auto px-1">{props.symbol}</span> : undefined}
                                    {props.label ? <span class={"form-label" + (props.small ? " form-label-sm" : "") + (props.label_muted && !props.label_infix ? " pr-2" : "") + (props.symbol ? " col px-1" : "")}>{props.label}</span> : undefined}
                                    {props.label_infix ? props.label_infix : undefined}
                                    {props.label_muted ? <span class={"text-muted" + (props.small ? " text-muted-sm" : "")}>{props.label_muted}</span> : undefined}
                                    {props.label_suffix ? props.label_suffix : undefined}
                                </div>
                            </div>

        let label = null;
        if (child_using_id_context != null) {
            label = <label for={this.id} class={"col col-form-label " + (props.small ? "col-form-label-sm " : "") + "pt-0 pt-lg-col-form-label"}>
                            {label_content}
                    </label>
        } else {
            label = <div class={"col col-form-label " + (props.small ? "col-form-label-sm " : "") + "pt-0 pt-lg-col-form-label"}>
                            {label_content}
                    </div>
        }

        return (
            <div class={"form-group row " + (props.class === undefined ? "" : props.class)} hidden={props.hidden == undefined ? false : props.hidden}>
                <div class={(props.labelColClasses === undefined ? "col-lg-4" : props.labelColClasses)}>
                    <div class="row">
                        {label}
                        {props.help ? <span class={"col-auto pt-lg-col-form-label"} onClick={() => this.setState({help_expanded: !state.help_expanded})}><HelpCircle {...{class:(state.help_expanded ? "btn-dark" : "btn-outline-secondary"), style:"border-radius: 50%; transition: .35s;"} as any}/></span> : undefined}
                    </div>
                </div>
                {inner}
            </div>
        );
    }
}
