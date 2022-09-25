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
import { __ } from "../translation";

interface InputPasswordProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput" | "value" | "disabled"> {
    idContext?: Context<string>
    value: string | null
    onValue: (value: string | null) => void
    hideClear?: boolean
    placeholder?: string
}

interface InputPasswordState {
    clear: boolean
    show: boolean
}

export class InputPassword extends Component<InputPasswordProps, InputPasswordState> {
    constructor() {
        super();
        this.state = {clear: false, show: false}
    }

    toggleClear() {
        let clear = !this.state.clear
        this.setState({clear: clear})
        if (clear)
            this.props.onValue("")
    }

    render(props: InputPasswordProps, state: Readonly<InputPasswordState>) {
        let id = useContext(props.idContext);
        return (
            <div class="input-group">
                <input class="form-control"
                    id={id}
                    type={state.show ? "text" : "password"}
                    placeholder={this.state.clear ? __("component.input_password.to_be_cleared") : (props.placeholder ?? __("component.input_password.unchanged"))}
                    onInput={(e) => {
                        let value: string = (e.target as HTMLInputElement).value;
                        props.onValue(value.length > 0 ? value : null)}
                    }
                    disabled={this.state.clear}
                    {...props} />
                <div class="input-group-append">
                    <div class="input-group-text custom-control custom-switch" style="padding-left: 2.5rem;">
                        <input id={id+"-show"} type="checkbox" class="custom-control-input" aria-label="Show password" onClick={() => this.setState({show: !this.state.show})} />
                        <label class="custom-control-label" for={id+"-show"} style="line-height: 20px;"><span data-feather="eye"></span></label>
                    </div>
                    { props.hideClear !== true ?
                        <div class="input-group-text custom-control custom-switch" style="padding-left: 2.5rem;">
                            <input id={id+"-clear"} type="checkbox" class="custom-control-input" aria-label="Clear password"  onClick={() => this.toggleClear()} />
                            <label class="custom-control-label" for={id+"-clear"} style="line-height: 20px;"><span data-feather="trash-2"></span></label>
                        </div>
                        : ""
                    }
                </div>
            </div>
        );
    }
}
