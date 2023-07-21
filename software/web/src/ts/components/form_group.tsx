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

import { h, Component, createContext, Context, VNode, cloneElement, toChildArray } from "preact";

export interface FormGroupProps {
    label: string
    label_muted?: string
    // Don't use ComponentChildren here: We want to pass in the idContext. This only works on VNodes.
    children: VNode | VNode[]
    valueClassList?: string
}

let id_counter = 0;

export class FormGroup extends Component<FormGroupProps, any> {
    idContext: Context<string>;
    id: string;

    constructor() {
        super();
        this.id = "formgroup" + id_counter;
        this.idContext = createContext(this.id);
        ++id_counter;
    }

    render(props: FormGroupProps) {
        return (
            <div class="form-group">
                <label for={this.id}>
                    <span class={"form-label" + (props.label_muted ? " pr-2" : "")} dangerouslySetInnerHTML={{__html: props.label}}></span>
                    {props.label_muted ? <span class="text-muted" dangerouslySetInnerHTML={{__html: props.label_muted}}></span> : ""}
                </label>
                <div class={props.valueClassList}>
                    {(toChildArray(props.children) as VNode[]).map(c => cloneElement(c, {idContext: this.idContext}))}
                </div>
            </div>
        );
    }
}
