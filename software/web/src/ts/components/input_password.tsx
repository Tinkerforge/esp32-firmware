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

import { h, Component, Context } from "preact";
import {useContext} from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";
import { Eye, Trash2 } from "react-feather";
import { __ } from "../translation";

interface InputPasswordProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput" | "value" | "disabled"> {
    idContext?: Context<string>
    value: string | null
    onValue: (value: string | null) => void
    hideClear?: boolean
    placeholder?: string
    showAlways?: boolean
    clearPlaceholder?: string
    clearSymbol?: h.JSX.Element
    allowAPIClear?: boolean
}

interface InputPasswordState {
    show: boolean
    clearSelected: boolean
}

export class InputPassword extends Component<InputPasswordProps, InputPasswordState> {
    constructor() {
        super();
        this.state = {show: false, clearSelected: false}
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

    render(props: InputPasswordProps, state: Readonly<InputPasswordState>) {
        let id = useContext(props.idContext);

        const toBeCleared = props.value === "" && (props.allowAPIClear || state.clearSelected);

        return (
            <div class="input-group">
                <input class="form-control"
                    id={id}
                    type={state.show || props.showAlways ? "text" : "password"}
                    value={props.value ?? ""}
                    placeholder={toBeCleared ? (props.clearPlaceholder ?? __("component.input_password.to_be_cleared"))
                                                    : (props.placeholder ?? __("component.input_password.unchanged"))}
                    onInput={(e) => {
                        let value: string = (e.target as HTMLInputElement).value;
                        props.onValue(value.length > 0 ? value : null)}
                    }
                    disabled={toBeCleared}
                    {...props} />
                <div class="input-group-append">
                    { props.showAlways ? null :
                        <div class="input-group-text custom-control custom-switch" style="padding-left: 2.5rem;">
                            <input id={id+"-show"} type="checkbox" class="custom-control-input" aria-label="Show password" onClick={() => this.setState({show: !state.show})} />
                            <label class="custom-control-label" for={id+"-show"} style="line-height: 20px;"><Eye/></label>
                        </div>
                    }
                    { props.hideClear ? null :
                        <div class="input-group-text custom-control custom-switch" style="padding-left: 2.5rem;">
                            <input id={id+"-clear"} type="checkbox" class="custom-control-input" aria-label="Clear password" onClick={() => this.toggleClear()} checked={toBeCleared}/>
                            <label class="custom-control-label" for={id+"-clear"} style="line-height: 20px;">{props.clearSymbol ?? <Trash2/>}</label>
                        </div>
                    }
                </div>
            </div>
        );
    }
}
