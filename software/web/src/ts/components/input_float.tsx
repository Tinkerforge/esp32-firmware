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

import * as util from "../util";

import { h, Context } from "preact";
import {useContext, useRef, useState} from "preact/hooks";
import { JSXInternal } from "preact/src/jsx";
import { Button, ButtonGroup } from "react-bootstrap";
import { Minus, Plus } from "react-feather";

interface InputFloatProps {
    idContext?: Context<string>
    value: number
    onValue: (value: number) => void
    digits: number
    unit: string
    min: number
    max: number
    showMinMax?: boolean
}

export function InputFloat(props: InputFloatProps) {
    let id = useContext(props.idContext);

    let pow10 = Math.pow(10, props.digits);

    const input = useRef<HTMLInputElement>();

    const [inputInFlight, setInputInFlight] = useState<string | null>(null);

    const setTarget = (target: number) => {
        target = util.clamp(props.min, target, props.max);
        input.current.parentNode.dispatchEvent(new Event('input', {bubbles: true}));
        props.onValue(target)
    };

    // Firefox does not localize numbers with a fractional part correctly.
    // OTOH Webkit based browsers (correctly) expect setting the value to a non-localized number.
    // Unfortunately, setting the value to a localized number (i.e. with , instead of . for German)
    // does not raise an exception, instead only a warning on the console is shown.
    // So to make everyone happy, we use user agent detection.
    let propValue = navigator.userAgent.indexOf("Gecko/") >= 0
        ? util.toLocaleFixed(props.value / pow10, props.digits)
        : (props.value / pow10).toFixed(props.digits);

    // If a user is currently typing, we have to preserve the input
    // (even if it does currently not confirm to the number format).
    // Otherwise set value to the given property.
    let value = inputInFlight === null ? propValue : inputInFlight;

    return (
        <div class="input-group">
            <input class="form-control no-spin"
                       id={id}
                       type="number"
                       ref={input}
                       step={1/pow10}
                       onInput={(e) => setInputInFlight((e.target as HTMLInputElement).value)}
                       onfocusout={() => {
                            if (inputInFlight !== null) {
                                let target = parseFloat(inputInFlight) * pow10;
                                target = util.clamp(props.min, target, props.max);
                                setTarget(target);
                            }
                            setInputInFlight(null);
                        }}
                       value={value}/>
            <div class="input-group-append">
                <div class="form-control input-group-text">
                    {this.props.unit}
                </div>
                <Button variant="primary"
                        className="form-control px-1"
                        style="margin-right: .125rem !important;"
                        onClick={() => {
                            let v = props.value;
                            let target = (v % pow10 === 0) ? (v - pow10) : (v - (v % pow10));

                            setTarget(target);
                        }}>
                    <Minus/>
                </Button>
                <Button variant="primary"
                        className="form-control px-1 rounded-right"
                        onClick={() => {
                            let v = props.value;
                            let target = (v - (v % pow10)) + pow10;

                            setTarget(target);
                        }}>
                    <Plus/>
                </Button>
            </div>
            {!props.showMinMax ? null :
                <ButtonGroup className="flex-wrap">
                    <Button variant="primary"
                            className="ml-2"
                            style="margin-right: .125rem !important;"
                            onClick={() => {
                                setTarget(props.min);
                            }}
                            >
                        {(props.min / pow10).toString() + " " + props.unit}
                    </Button>
                    <Button variant="primary" onClick={() => {
                                setTarget(props.max);
                            }}
                            >
                        {(props.max / pow10).toString() + " " + props.unit}
                    </Button>
                </ButtonGroup>
            }
        </div>
    );
}
