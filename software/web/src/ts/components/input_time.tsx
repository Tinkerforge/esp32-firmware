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
import { useId, useContext, useRef } from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";
//import { Button } from "react-bootstrap";
//import { ArrowLeft, ArrowRight } from "react-feather";

import * as util from "../../ts/util";

interface InputTimeProps extends Omit<JSXInternal.HTMLAttributes<HTMLInputElement>, "value" | "class" | "id" | "type" | "onInput"> {
    idContext?: Context<string>
    date: Date
    onDate?: (value: Date) => void
    children?: ComponentChildren
    style?: string
    showSeconds?: boolean
}

export function InputTime(props: InputTimeProps) {
    const input = useRef<HTMLInputElement>();
    const id = !props.idContext ? useId() : useContext(props.idContext);

    const dateToHourMinSecValue = (date: Date) => util.toIsoString(date).split("T")[1];
    const dateToHourMinValue    = (date: Date) => util.leftPad(date.getHours(), 0, 2) + ':' + util.leftPad(date.getMinutes(), 0, 2);

    const valueToDate = (value: string) => {
        let [h, m, s] = value.split(/:/g).map(x => parseInt(x));
        // If showSeconds is false, the string is of form hh:mm and s will be undefined
        if (s === undefined) {
            s = 0;
        }

        return new Date(0, 0, 1, h, m, s, 0);
    };

    let inner =
        <>
            {props.children}
            <input class={"form-control " + (props.className ?? "")}
                   ref={input}
                   id={id}
                   type="time"
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
                   // Show seconds if showSeconds is true or undefined (i.e. default is true)
                   value={props.showSeconds === false ? dateToHourMinValue(props.date) : dateToHourMinSecValue(props.date)} />
        </>;

    return inner;
/*
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
*/
}
