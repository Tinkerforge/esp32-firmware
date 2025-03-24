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

import { h, JSX, Context } from "preact";
import { useId, useContext } from "preact/hooks";
import { register_id_context_component_type } from "./form_row";

interface SwitchableInputSelectProps extends Omit<JSX.InputHTMLAttributes<HTMLSelectElement>, "id" | "type" | "onInput"> {
    idContext?: Context<string>
    items: [string, string][];
    onValue: (value: string) => void;
    placeholder?: string;
    className?: string;
    style?: string;
    invalidFeedback?: string;
    checked: boolean;
    onSwitch: JSX.MouseEventHandler<HTMLInputElement>;
    switch_label_active?: string;
    switch_label_inactive?: string;
    switch_label_min_width?: string;
}

export function SwitchableInputSelect(props: SwitchableInputSelectProps) {
    let {idContext, items, value, onValue, placeholder, className, style, checked, onSwitch, switch_label_active, switch_label_inactive, switch_label_min_width, ...p} = props;
    const id = !idContext ? useId() : useContext(idContext);
    const label_desktop_id = id + "-1";
    const label_mobile_id = id + "-2";

    if (placeholder) {
        let found = false;

        for (let i = 0; i < items.length; ++i) {
            if (items[i][0] == value) {
                found = true;
            }
        }

        if (!found) {
            // if value is not found make sure to show the
            // placeholder instead of an empty input field
            value = "";
        } else {
            // if value is found then remove the placeholder
            // because it cannot be selected anyway
            placeholder = undefined;
        }
    }

    const invalidFeedback = props.invalidFeedback ? <div class="invalid-feedback" hidden={props.hidden}>{props.invalidFeedback}</div> : undefined;

    return (
        <div class="row no-gutters input-group rounded">
            <div class="d-none d-sm-block input-group-prepend input-group-text custom-control custom-switch" style={"padding-left: 2.75rem; padding-right: 0.6rem; border-bottom-right-radius: 0; border-top-right-radius: 0; text-align: left; " + (switch_label_min_width ? ("min-width: " + switch_label_min_width) : "")}>
                <input type="checkbox" class="custom-control-input" id={label_desktop_id} checked={checked} onClick={onSwitch} disabled={props.disabled} />
                <label class="custom-control-label" for={label_desktop_id}>{checked ? switch_label_active : switch_label_inactive}</label>
            </div>
            <div class="d-block d-sm-none input-group-prepend input-group-text custom-control custom-switch" style={"padding-left: 2.75rem; padding-right: 0; border-bottom-right-radius: 0; border-top-right-radius: 0;"}>
                <input type="checkbox" class="custom-control-input" id={label_mobile_id} checked={checked} onClick={onSwitch} disabled={props.disabled} />
                <label class="custom-control-label" for={label_mobile_id}></label>
            </div>
            <select
                {...p}
                value={value}
                disabled={!checked || props.disabled}
                class={(className ?? "") + " form-control custom-select"}
                style={style ?? ""}
                id={id}
                onInput={(e) => onValue((e.target as HTMLSelectElement).value)}
                >
                {
                    (placeholder ? [<option value="" disabled selected>{placeholder}</option>] : [])
                        .concat(
                            items.map((k) =>
                                <option value={k[0].endsWith("disabled") ? "" : k[0]} key={k[0]} disabled={k[0].endsWith("disabled")}>{k[1]}</option>))
                }
            </select>
            {invalidFeedback}
        </div>
    );
}

register_id_context_component_type(SwitchableInputSelect);
