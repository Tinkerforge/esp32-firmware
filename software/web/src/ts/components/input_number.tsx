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

import { h, Context, Fragment, ComponentChildren, toChildArray } from "preact";
import { useId, useContext, useRef } from "preact/hooks";
import { JSX } from 'preact';
import { Button } from "react-bootstrap";
import { Minus, Plus } from "react-feather";
import { __ } from "../translation";
import { register_id_context_component_type } from "./form_row";

import * as util from "../util";

interface InputNumberProps extends Omit<JSX.InputHTMLAttributes<HTMLInputElement>, "class" | "id" | "type" | "onInput"> {
    idContext?: Context<string>;
    value: number;
    onValue?: (value: number) => void;
    unit?: string;
    invalidFeedback?: string;
    disabled?: boolean;
    readonly?: boolean;
    children?: ComponentChildren;
}

export function InputNumber(props: InputNumberProps) {
    const id = !props.idContext ? useId() : useContext(props.idContext);

    delete props.idContext;

    const input = useRef<HTMLInputElement>();
    const value = parseInt(props.value?.toString(), 10);

    const invalid = isNaN(value) || (props.min !== undefined && value < parseInt(props.min.toString())) || (props.max !== undefined && value > parseInt(props.max.toString()));

    let invalidFeedback = undefined;
    if ("invalidFeedback" in props && props.invalidFeedback) {
        invalidFeedback = <div class="invalid-feedback">{props.invalidFeedback}</div>;
    } else if (invalid) {
        if (props.required && isNaN(value)) {
            invalidFeedback = <div class="invalid-feedback">{__("component.input_number.required")}</div>;
        } else if ("min" in props && !("max" in props)) {
            invalidFeedback = <div class="invalid-feedback">{__("component.input_number.min_only")(props.min.toString(), props.unit ? props.unit : "")}</div>;
        } else if (!("min" in props) && "max" in props) {
            invalidFeedback = <div class="invalid-feedback">{__("component.input_number.max_only")(props.max.toString(), props.unit ? props.unit : "")}</div>;
        } else if ("min" in props && "max" in props) {
            invalidFeedback = <div class="invalid-feedback">{__("component.input_number.min_max")(props.min.toString(), props.max.toString(), props.unit ? props.unit : "")}</div>;
        }
    }

    return <div class="input-group">
        {toChildArray(props.children).length > 0 ? <div class="input-group-prepend">{props.children}</div> : undefined}
        <input class="form-control no-spin"
                ref={input}
                id={id}
                type="number"
                onInput={props.onValue === undefined ? undefined : (e) => {
                        // Chrome prints a console warning if NaN is assigned as an input's value; null works.
                        let value = parseInt((e.target as HTMLInputElement).value, 10);
                        if (isNaN(value))
                            value = null;
                        props.onValue(value);
                    }}
                inputMode="numeric"
                {...props}
                value={util.hasValue(props.value) ? props.value : ""}
                readonly={props.onValue === undefined || props.readonly}
        />
        {props.unit || (props.onValue && !props.readonly) ? <div class="input-group-append">
            {props.unit ? <div class="form-control input-group-text">{props.unit}</div> : undefined}
            {props.onValue && !props.readonly ? <>
            <Button variant="primary"
                    disabled={props.value == props.min || props.disabled || props.readonly}
                    className="form-control px-1"
                    style="margin-right: .125rem !important;"
                    onClick={() => {
                        if (util.hasValue(props.value) && !isNaN(props.value)) {
                            props.onValue(util.clamp(props.min as number, props.value - 1, props.max as number));
                        }
                        else {
                            props.onValue(props.min as number);
                        }

                        input.current.parentNode.dispatchEvent(new Event('input', {bubbles: true}));
                    }}>
                <Minus/>
            </Button>
            <Button variant="primary"
                    disabled={props.value == props.max || props.disabled || props.readonly}
                    className="form-control px-1 rounded-right"
                    onClick={() => {
                        if (util.hasValue(props.value) && !isNaN(props.value)) {
                            props.onValue(util.clamp(props.min as number, props.value + 1, props.max as number));
                        }
                        else {
                            props.onValue(props.max as number);
                        }

                        input.current.parentNode.dispatchEvent(new Event('input', {bubbles: true}));
                    }}>
                <Plus/>
            </Button> </> : null}
        </div> : null }
        {invalidFeedback}
    </div>;
}

register_id_context_component_type(InputNumber);
