/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

import { h, Component } from "preact";

interface InputAnyFloatProps {
    value: number,
    onValue: (value: number) => void,
    required?: boolean,
}

interface InputAnyFloatState {
    value_str: string,
}

export class InputAnyFloat extends Component<InputAnyFloatProps, InputAnyFloatState> {
    separator: string;
    regexp: RegExp;
    pattern: string;

    constructor(props: InputAnyFloatProps) {
        super(props);

        if (4.2.toLocaleString().indexOf(",") >= 0) {
            this.separator = ",";
            this.regexp = /^-?[0-9]*,?[0-9]*$/;
            this.pattern = "-?[0-9]+(,[0-9]+)?";
        }
        else {
            this.separator = ".";
            this.regexp = /^-?[0-9]*\.?[0-9]*$/;
            this.pattern = "-?[0-9]+(\.[0-9]+)?";
        }

        this.state = {
            value_str: props.value.toString().replace(".", this.separator),
        } as any;
    }

    parse_input(text: string) {
        let m = text.match(this.regexp);

        if (m == null) {
            this.setState({value_str: this.state.value_str});
            return
        }

        let num = m[0].replace(",", ".");

        if (num.endsWith(".")) {
            num += "0";
        }

        let value = parseFloat(num);

        if (isNaN(value)) {
            value = null;
        }

        this.setState({value_str: m[0]});
        this.props.onValue(value);
    }

    render() {
        return <input
            class="form-control"
            type="text"
            pattern={this.pattern}
            inputMode="decimal"
            required={this.props.required}
            value={this.state.value_str}
            onInput={(e) => {
                this.parse_input((e.target as HTMLInputElement).value);
            }}
            onKeyDown={(e) => {
                this.parse_input((e.target as HTMLInputElement).value);
            }} />;
    }
}
