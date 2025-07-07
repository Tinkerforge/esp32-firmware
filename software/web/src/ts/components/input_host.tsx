/* esp32-firmware
 * Copyright (C) 2025 Matthias Bolte <matthias@tinkerforge.com>
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

import * as util from "../util";
import { Context, h } from "preact";
import { __ } from "../translation";
import { InputTextPatterned } from "./input_text";
import { register_id_context_component_type } from "./form_row";

interface InputHostProps {
    class?: string
    required?: boolean
    disabled?: boolean
    value: string
    onValue?: (value: string) => void
    invalidFeedback?: string
    idContext?: Context<string>
};

export function InputHost(props: InputHostProps) {
    let invalidFeedback = props.invalidFeedback;

    if (invalidFeedback === undefined) {
        invalidFeedback = __("component.input_host.invalid_feedback");
    }

    return <InputTextPatterned {...props}
                invalidFeedback={invalidFeedback}
                maxLength={64}
                class={props.class + (props.value.endsWith(".localhost") ? " is-invalid" : "")}
                pattern={`^(?:${util.IPV4_ADDRESS_PATTERN}|.*[^0-9\.].*)$`} />
}

register_id_context_component_type(InputHost);
