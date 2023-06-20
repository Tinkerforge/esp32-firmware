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

import { h, Context, VNode, Fragment } from "preact";
import { useContext, useRef } from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";
import { Button } from "react-bootstrap";
import { ArrowLeft, ArrowRight } from "react-feather";

import * as util from "../../ts/util";

interface InputDateProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>, "value" | "class" | "id" | "type" | "onInput"> {
    idContext?: Context<string>
    date: Date
    onDate?: (value: Date) => void
    buttons?: "year"|"month"|"day"
    children?: VNode | VNode[]
    style?: string
}

export function InputDate(props: InputDateProps) {
    const input = useRef<HTMLInputElement>();
    let id = props.idContext === undefined ? "" : useContext(props.idContext);

    const dateToValue = (date: Date) => {
        try {
            return util.leftPad(date.getFullYear(), 0, 4) + "-" + util.leftPad((date.getMonth() + 1), 0, 2) + "-" + util.leftPad(date.getDate(), 0, 2);
        } catch (e) {
            return "";
        }
    };

    const valueToDate = (value: string) => {
        let [y, mIdx, d] = value.split(/-/g).map(x => parseInt(x));

        return new Date(y, mIdx - 1, d);
    }

    let inner =
        <>
            {props.children}
            <input class={"form-control " + (props.className ?? "")}
                   ref={input}
                   id={id}
                   type="date"
                   style={props.style ?? ""}
                   onInput={
                       props.onDate ? (e) => {
                           let timeString = (e.target as HTMLInputElement).value;
                           if (timeString == "")
                               return;

                           props.onDate(valueToDate(timeString));
                       } : undefined
                   }
                   disabled={!props.onDate}
                   value={dateToValue(props.date)} />
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
                            let date = valueToDate(dateToValue(props.date));

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
                        }}>
                    <ArrowLeft/>
                </Button>
                <Button variant="primary"
                        className="form-control px-1 rounded-right"
                        onClick={() => {
                            let date = valueToDate(dateToValue(props.date));

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
                        }}>
                    <ArrowRight/>
                </Button>
            </div>
        </div>
    );
}
