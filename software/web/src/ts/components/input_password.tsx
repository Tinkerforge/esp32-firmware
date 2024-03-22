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

import { h, Component, Context, Fragment } from "preact";
import { useId, useContext } from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";
import { Button } from "react-bootstrap";
import { Eye, EyeOff, Trash2 } from "react-feather";
import { __ } from "../translation";

import * as util from "../../ts/util";

interface InputPasswordProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput" | "value" | "disabled"> {
    idContext?: Context<string>
    value: string | null
    onValue: (value: string | null) => void
    hideClear?: boolean // default: false
    placeholder?: string // default: "<unchanged>"
    showAlways?: boolean // default: false
    clearPlaceholder?: string // default: "<to_be_cleared>"
    clearSymbol?: h.JSX.Element
    allowAPIClear?: boolean // default: false
    invalidFeedback?: string // default: ""
}

interface InputPasswordState {
    show: boolean;
    clearSelected: boolean;
    shake: boolean;
}

export class InputPassword extends Component<InputPasswordProps, InputPasswordState> {
    constructor() {
        super();
        this.state = {show: false, clearSelected: false, shake: false};
    }

    toggleClear() {
        if (this.props.value === "") {
            this.props.onValue(null)
            this.setState({clearSelected: false});
        }
        else {
            this.props.onValue("")
            this.setState({clearSelected: true});
        }
    }

    shake() {
        this.setState({shake: true}, () => {
            window.setTimeout(() => {
                this.setState({shake: false});
            }, 500);
        });
    }

    render(props: InputPasswordProps, state: Readonly<InputPasswordState>) {
        const id = !props.idContext ? useId() : useContext(props.idContext);

        let invalidFeedback = undefined;
        if ("invalidFeedback" in props)
            invalidFeedback = <div class="invalid-feedback">{props.invalidFeedback}</div>;
        else if (props.required && !props.value)
            invalidFeedback = <div class="invalid-feedback">{__("component.input_text.required")}</div>;
        else if ("minLength" in props && !("maxLength" in props))
            invalidFeedback = <div class="invalid-feedback">{__("component.input_text.min_only")(props.minLength.toString())}</div>;
        else if (!("minLength" in props) && "maxLength" in props)
            invalidFeedback = <div class="invalid-feedback">{__("component.input_text.max_only")(props.maxLength.toString())}</div>;
        else if ("minLength" in props && "maxLength" in props)
            invalidFeedback = <div class="invalid-feedback">{__("component.input_text.min_max")(props.minLength.toString(), props.maxLength.toString())}</div>;

        const toBeCleared = props.value === "" && (props.allowAPIClear || state.clearSelected);

        return (
            <>
                <div class={"input-group rounded" + (this.state.shake ? " shake" : "")}>
                    <input class={"form-control" + (props.showAlways && props.hideClear ? " rounded-right" : "")}
                        id={id}
                        type={state.show || props.showAlways ? "text" : "password"}
                        value={props.value ?? ""}
                        placeholder={toBeCleared ? (props.clearPlaceholder ?? __("component.input_password.to_be_cleared"))
                                                        : (props.placeholder ?? __("component.input_password.unchanged"))}
                        onInput={(e) => {
                            let value: string = (e.target as HTMLInputElement).value;
                            if ((props.maxLength != undefined && new Blob([(e.target as HTMLInputElement).value]).size <= (props.maxLength as any)) ||
                                    props.maxLength == undefined)
                                props.onValue(value.length > 0 ? value : null);
                            else
                            {
                                let old_val = props.value as string;
                                props.onValue(old_val.length > 0 ? old_val : null);
                            }}
                        }
                        disabled={toBeCleared}
                        {...props}
                    />
                    <div class="input-group-append">
                        { props.showAlways ? null :
                            <Button variant="primary" className={"px-1" + (props.hideClear ? " rounded-right" : "")} style="line-height: 20px;" onClick={() => this.setState({show: !state.show})} disabled={toBeCleared}>{state.show ? <EyeOff/> : <Eye/>}</Button>
                        }
                        { props.hideClear ? null :
                            <div class="input-group-text custom-control custom-switch rounded-right" style="padding-left: 2.75rem; padding-right: 0.5rem;">
                                <input id={id+"-clear"} type="checkbox" class="custom-control-input" aria-label="Clear password" onClick={() => this.toggleClear()} checked={toBeCleared}/>
                                <label class="custom-control-label" for={id+"-clear"} style="line-height: 20px;">{props.clearSymbol ?? <Trash2/>}</label>
                            </div>
                        }
                    </div>
                    {invalidFeedback}
                </div>
                <div class="mt-2" hidden={!util.capsLockActive()}>{__("component.input_password.capslock_enabled")}</div>
            </>
        );
    }
}
