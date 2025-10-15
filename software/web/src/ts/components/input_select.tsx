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
import { useId, useContext } from "preact/hooks";
import { JSX } from 'preact';
import { register_id_context_component_type } from "./form_row";

export interface InputSelectProps extends Omit<JSX.InputHTMLAttributes<HTMLSelectElement>, "id" | "type" | "onInput"> {
    idContext?: Context<string>;
    items: ([string, [number | string, string][]]|[number | string, string])[];
    onValue?: (value: string) => void;
    placeholder?: string;
    className?: string;
    style?: string;
    invalidFeedback?: string;
}

function find_value(value: InputSelectProps["value"], items: InputSelectProps["items"]) {
    for (let i = 0; i < items.length; ++i) {
        let item = items[i];
        if (typeof(item[1]) == "string" && item[0] == value) {
            return true;
        } else {
            for (let i = 0; i < item[1].length; ++i) {
                let subitem = item[1][i];
                if (subitem[0] == value)
                    return true;
            }
        }
    }
    return false;
}

function item_to_option(item: [string | number, string]) {
    if (typeof(item[0]) == "string")
        return <option value={item[0].endsWith("disabled") ? "" : item[0]} key={item[0]} disabled={item[0].endsWith("disabled")}>{item[1]}</option>
    else
        return <option value={item[0]} key={item[0]}>{item[1]}</option>
}

export function InputSelect(props: InputSelectProps) {
    let {idContext, items, value, onValue, placeholder, className, style, ...p} = props;

    const id = !idContext ? useId() : useContext(idContext);

    if (placeholder) {
        if (!find_value(value, items)) {
            // if value is not found make sure to show the
            // placeholder instead of an empty input field
            value = "";
        } else {
            // if value is found then remove the placeholder
            // because it cannot be selected anyway
            placeholder = undefined;
        }
    }

    let select_items = placeholder ? [<option value="" disabled selected>{placeholder}</option>] : []

    for (let i = 0; i < items.length; ++i) {
        let item = items[i];
        if (typeof(item[1]) == "string") {
            select_items.push(item_to_option(item as [string | number, string]))
        } else {
            let subitems = item[1].map(k => item_to_option(k))
            select_items.push(<optgroup label={item[0] as string}>{subitems}</optgroup>)
        }
    }

    const invalidFeedback = props.invalidFeedback ? <div class="invalid-feedback" hidden={props.hidden}>{props.invalidFeedback}</div> : undefined;

    return (
        <>
            <select
                {...p}
                disabled={onValue === undefined || p.disabled}
                value={value}
                class={(className ?? "") + " form-control custom-select"}
                style={style ?? ""}
                id={id}
                onInput={onValue === undefined ? undefined : (e) => onValue((e.target as HTMLSelectElement).value)}
                >
                {select_items}
            </select>
            {invalidFeedback}
        </>
    );
}

register_id_context_component_type(InputSelect);
