/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

import { h, Component, createRef } from "preact";
import { useId } from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";

interface OutputTextareaProps extends Omit<JSXInternal.HTMLAttributes<HTMLTextAreaElement>, "readonly" | "onScroll" | "ref"> {
    value: string;
    moreClass?: string;
    resize?: "both" | "horizontal" | "vertical" | "none";
}

export class OutputTextarea extends Component<OutputTextareaProps, {}> {
    ref = createRef<HTMLTextAreaElement>();
    auto_scroll: boolean = true;

    override componentDidUpdate() {
        let ta = this.ref.current;

        if (!ta)
            return;

        if (!this.auto_scroll)
            return;

        this.ref.current.scrollTop = this.ref.current.scrollHeight;
    }

    render(props: OutputTextareaProps, state: {}){
        let {rows = 20,
             resize = 'both',
             style=`resize: ${resize}; width: 100%; white-space: pre; line-height: 1.2; text-shadow: none; font-size: 0.875rem;`,
             ...rest} = props;

        return <textarea class={"text-monospace form-control " + (props.moreClass ?? "")}
                         readonly
                         id={useId()}
                         rows={rows}
                         style={style}
                         ref={this.ref}
                         onScroll={(ev) => {
                            let ta = ev.target as HTMLTextAreaElement;
                            this.auto_scroll = Math.ceil(ta.scrollHeight - ta.scrollTop) <= ta.clientHeight + 2 * parseFloat(getComputedStyle(ta).lineHeight);
                         }}
                         {...rest}>
                    {props.value}
                </textarea>

    }
}
