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

import { h, Context, Fragment, ComponentChildren } from "preact";
import { useId, useContext } from "preact/hooks";
import { JSX } from 'preact';
import { __ } from "../translation";
import { register_id_context_component_type } from "./form_row";

import * as util from "../util";

interface InputTextProps extends Omit<JSX.InputHTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput" | "pattern" | "className"> {
    idContext?: Context<string>
    onValue?: (value: string) => void
    invalidFeedback?: string
    class?: string
    children?: ComponentChildren
    prefixChildren?: ComponentChildren
}

interface InputTextPatternedProps extends Omit<JSX.InputHTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput" | "className"> {
    idContext?: Context<string>
    onValue?: (value: string) => void
    invalidFeedback: string
    pattern: string
    class?: string
    children?: ComponentChildren
    prefixChildren?: ComponentChildren
}

interface InputTextInternalProps extends JSX.InputHTMLAttributes<HTMLInputElement> {
    idContext?: Context<string>
    onValue?: (value: string) => void
    invalidFeedback?: string
    prefixChildren?: ComponentChildren
}

function InputTextInternal<T extends InputTextInternalProps>(props: util.NoExtraProperties<InputTextInternalProps, T>) {
    let {
        idContext,
        onValue,
        invalidFeedback,
        prefixChildren,
        ...p
    } = props;

    const id = !idContext ? useId() : useContext(idContext);

    const invalidFeedbackDiv = invalidFeedback !== undefined ? <div class="invalid-feedback">{invalidFeedback}</div> : undefined

    let inner = <input {...p}
                    class={"form-control " + (props.class ?? "")}
                    id={id}
                    type="text"
                    onInput={onValue ? (e) => {
                        if ((props.maxLength != undefined && new Blob([(e.target as HTMLInputElement).value]).size <= (props.maxLength as any)) ||
                                props.maxLength == undefined)
                            onValue((e.target as HTMLInputElement).value);
                        else
                            onValue(props.value as string);
                    } : undefined}
                    readonly={!onValue || props.readonly}/>

    if (prefixChildren || props.children) {
        return <div class="input-group">
                {prefixChildren}
                {inner}
                {props.children}
                {invalidFeedbackDiv}
            </div>
    }

    return (
        <>
            {inner}
            {invalidFeedbackDiv}
        </>
    );
}

export function InputText<T extends InputTextProps>(props: util.NoExtraProperties<InputTextProps, T>) {
    let invalidFeedback = undefined;
    if ("invalidFeedback" in props && props.invalidFeedback)
        invalidFeedback = props.invalidFeedback;
    else if (props.required && !props.value)
        invalidFeedback = __("component.input_text.required");
    else if ("minLength" in props && !("maxLength" in props) && props.minLength)
        invalidFeedback = __("component.input_text.min_only")(props.minLength.toString());
    else if (!("minLength" in props) && "maxLength" in props && props.maxLength)
        invalidFeedback = __("component.input_text.max_only")(props.maxLength.toString());
    else if ("minLength" in props && "maxLength" in props && props.minLength && props.maxLength)
        invalidFeedback = __("component.input_text.min_max")(props.minLength.toString(), props.maxLength.toString());

    return <InputTextInternal {...props} invalidFeedback={invalidFeedback} />
}

register_id_context_component_type(InputText);

// Passing a pattern to an input type="text" also requires passing invalidFeedback for a readable error message
export function InputTextPatterned<T extends InputTextPatternedProps>(props: util.NoExtraProperties<InputTextPatternedProps, T>) {
    return <InputTextInternal {...props} />
}

register_id_context_component_type(InputTextPatterned);
