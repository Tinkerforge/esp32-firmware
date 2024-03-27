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
import { useId, useContext, useRef, useState } from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";
import { Button } from "react-bootstrap";
import { ArrowLeft, ArrowRight } from "react-feather";

import * as util from "../../ts/util";

interface InputDateProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>, "value" | "class" | "id" | "type" | "onInput"> {
    idContext?: Context<string>
    date: Date
    onDate?: (value: Date) => void
    buttons?: "year"|"month"|"day"
    children?: ComponentChildren
    style?: string
}

export function InputDate(props: InputDateProps) {
    const input = useRef<HTMLInputElement>();
    const id = !props.idContext ? useId() : useContext(props.idContext);

    const [inputInFlight, setInputInFlight] = useState<string | null>(null);
    const [lastEvent, setLastEvent] = useState<'key' | 'click' | null>(null);

    const dateToValue = (date: Date) => isNaN(date.getFullYear()) ? "" : util.toIsoString(date).split("T")[0];

    const valueToDate = (value: string) => {
        if (!value) {
            return new Date(NaN);
        }

        let [y, mIdx, d] = value.split(/-/g).map(x => parseInt(x));

        return new Date(y, mIdx - 1, d);
    };

    // If a user is currently typing, we have to preserve the input
    // (even if it does currently not confirm to the number format).
    // Otherwise set value to the given property.
    let value = inputInFlight === null ? dateToValue(props.date) : inputInFlight;

    const sendInFlight = (override?: string) => {
        let value = override !== undefined ? override : inputInFlight;

        if (value === null) {
            return;
        }

        props.onDate(valueToDate(value));
        setInputInFlight(null);
        input.current.parentNode.dispatchEvent(new Event('input', {bubbles: true}));
    };

    let inner =
        <>
            {props.children}
            <input class={"form-control " + (props.className ?? "")}
                   ref={input}
                   id={id}
                   type="date"
                   style={props.style ?? ""}
                   onInput={props.onDate ? (e) => {
                       let value = (e.target as HTMLInputElement).value;
                       setInputInFlight(value);
                       if (lastEvent == 'click') {
                           // need to pass value directly, because the state
                           // change by setInputInFlight is not immediate
                           sendInFlight(value);
                       }
                   } : undefined}
                   // onfocusout is not triggered if a user submits the form by pressing enter
                   onKeyDown={props.onDate ? (e: KeyboardEvent) => {
                       setLastEvent('key');
                       if (e.key == 'Enter') {
                           sendInFlight();
                       }
                   } : undefined}
                   onClick={() => {
                       setLastEvent('click');
                   }}
                   onfocusout={props.onDate ? () => sendInFlight() : undefined}
                   disabled={!props.onDate}
                   value={value} />
        </>;

    if ((!props.onDate || !props.buttons) && !props.children) {
        return inner;
    }

    return (
        <div class="input-group">
            {inner}
            <div class="input-group-append">
                <Button variant="primary"
                        className="form-control px-1"
                        style="margin-right: .125rem !important;"
                        onClick={() => {
                            let date = valueToDate(value);

                            if (props.buttons == "year") {
                                date.setFullYear(date.getFullYear() - 1);
                            }

                            if (props.buttons == "month") {
                                date.setMonth(date.getMonth() - 1);
                            }

                            if (props.buttons == "day") {
                                date.setDate(date.getDate() - 1);
                            }

                            props.onDate(date);
                            input.current.parentNode.dispatchEvent(new Event('input', {bubbles: true}));
                        }}
                        disabled={!value}>
                    <ArrowLeft/>
                </Button>
                <Button variant="primary"
                        className="form-control px-1 rounded-right"
                        onClick={() => {
                            let date = valueToDate(value);

                            if (props.buttons == "year") {
                                date.setFullYear(date.getFullYear() + 1);
                            }

                            if (props.buttons == "month") {
                                date.setMonth(date.getMonth() + 1);
                            }

                            if (props.buttons == "day") {
                                date.setDate(date.getDate() + 1);
                            }

                            props.onDate(date);
                            input.current.parentNode.dispatchEvent(new Event('input', {bubbles: true}));
                        }}
                        disabled={!value}>
                    <ArrowRight/>
                </Button>
            </div>
        </div>
    );
}
