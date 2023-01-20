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

import { h, Context, Fragment } from "preact";
import {useContext, useRef} from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";
import { Button } from "react-bootstrap";
import { Minus, Plus } from "react-feather";

import * as util from "../util";

interface InputNumberProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput"> {
    idContext?: Context<string>
    value: number
    onValue?: (value: number) => void
    unit?: string
}

export function InputNumber(props: InputNumberProps) {

    const input = useRef<HTMLInputElement>();

    return (
        <div class="input-group">
            <input class="form-control no-spin"
                       ref={input}
                       id={props.idContext ? useContext(props.idContext) : undefined}
                       type="number"
                       disabled={props.onValue === undefined}
                       onInput={props.onValue === undefined ? undefined : (e) => props.onValue(parseInt((e.target as HTMLInputElement).value, 10))}
                       {...props}/>
            {props.unit || props.onValue ? <div class="input-group-append">
                {props.unit ? <div class="form-control input-group-text">{this.props.unit}</div> : undefined}
                {props.onValue ? <>
                <Button variant="primary"
                        className="form-control px-1"
                        style="margin-right: .125rem !important;"
                        onClick={() => {
                            props.onValue(util.clamp(props.min as number, props.value - 1, props.max as number));
                            input.current.parentNode.dispatchEvent(new Event('input', {bubbles: true}));
                        }}>
                    <Minus/>
                </Button>
                <Button variant="primary"
                        className="form-control px-1 rounded-right"
                        onClick={() => {
                            props.onValue(util.clamp(props.min as number, props.value + 1, props.max as number));
                            input.current.parentNode.dispatchEvent(new Event('input', {bubbles: true}));
                        }}>
                    <Plus/>
                </Button> </> : null}
            </div> : null }
        </div>
    );
}
