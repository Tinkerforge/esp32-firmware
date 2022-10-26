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
import {useContext} from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";
import { __ } from "../translation";

import * as util from "../util";

interface InputTextProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput" | "pattern"> {
    idContext?: Context<string>
    onValue?: (value: string) => void
}

interface InputTextWithValidationProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>,  "class" | "id" | "type" | "onInput"> {
    idContext?: Context<string>
    onValue?: (value: string) => void
    invalidFeedback: string
}

export function InputText<T extends (InputTextProps | InputTextWithValidationProps)>(props: util.NoExtraProperties<InputTextProps, T> | InputTextWithValidationProps) {
    let id = props.idContext === undefined ? "" : useContext(props.idContext);
    let invalidFeedback = undefined;
    if ("invalidFeedback" in props)
        invalidFeedback = <div class="invalid-feedback">{props.invalidFeedback}</div>;
    else if ("minLength" in props && !("maxLength" in props))
        invalidFeedback = <div class="invalid-feedback">{__("component.input_text.min_only_prefix") + props.minLength.toString() + __("component.input_text.min_only_suffix")}</div>;
    else if (!("minLength" in props) && "maxLength" in props)
        invalidFeedback = <div class="invalid-feedback">{__("component.input_text.max_only_prefix") + props.maxLength.toString() + __("component.input_text.max_only_suffix")}</div>;
    else if ("minLength" in props && "maxLength" in props)
        invalidFeedback = <div class="invalid-feedback">{__("component.input_text.min_max_prefix") + props.minLength.toString() + __("component.input_text.min_max_infix") + props.maxLength.toString() + __("component.input_text.min_max_suffix")}</div>;

    return (
        <>
            <input class="form-control"
                id={id}
                type="text"
                onInput={props.onValue ? (e) => props.onValue((e.target as HTMLInputElement).value) : undefined}
                readonly={!props.onValue}
                {...props}/>
            {invalidFeedback}
        </>
    );
}
