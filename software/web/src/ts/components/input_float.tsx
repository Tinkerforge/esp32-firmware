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

import { h, Context, Fragment } from "preact";
import { useContext, useRef, useState } from "preact/hooks";
import { Button, ButtonGroup } from "react-bootstrap";
import { Minus, Plus } from "react-feather";

interface InputFloatReadonlyProps {
    idContext?: Context<string>
    value: number
    digits: number
    unit: string
    class?: string
}

interface InputFloatProps extends InputFloatReadonlyProps {
    onValue: (value: number) => void
    min: number
    max: number
    showMinMax?: boolean
}

export function InputFloat(props: InputFloatProps | InputFloatReadonlyProps) {
    const id = !props.idContext ? util.useId() : useContext(props.idContext);

    let pow10 = Math.pow(10, props.digits);

    const input = useRef<HTMLInputElement>();

    const [inputInFlight, setInputInFlight] = useState<string | null>(null);

    const setTarget = 'onValue' in props ? (target: number) => {
        target = util.clamp(props.min, Math.round(target), props.max);
        input.current.parentNode.dispatchEvent(new Event('input', {bubbles: true}));
        props.onValue(target)
    } : (target: number) => {};

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

    const sendInFlight = () => {
        if (inputInFlight === null)
            return;

        let target = parseFloat(inputInFlight);
        if (isNaN(target))
            return;

        setTarget(target * pow10);
        setInputInFlight(null);
    }

    let floatMin = 'min' in props ? props.min / pow10 : 0;
    let floatMax = 'max' in props ? props.max / pow10 : 0;

    return (
        <div class={"input-group " + (props.class ? props.class : "")}>
            <input class="form-control no-spin"
                       id={id}
                       type="number"
                       ref={input}
                       step={1/pow10}
                       // Don't set min/max: this is already enforced in setTarget.
                       //min={'min' in props ? floatMin : undefined}
                       //max={'max' in props ? floatMax : undefined}
                       onInput={'onValue' in props ? (e) => setInputInFlight((e.target as HTMLInputElement).value) : undefined}
                       // onfocusout is not triggered if a user submits the form by pressing enter
                       onKeyDown={(e: KeyboardEvent) => {
                        if (e.key == 'Enter')
                            sendInFlight();
                       }}
                       onfocusout={'onValue' in props ? () => sendInFlight() : undefined}
                       value={value}
                       disabled={!('onValue' in props)}
                       inputMode="decimal"
                       style="min-width: 5em;"/>
            <div class="input-group-append">
                <div class={"form-control input-group-text" + ('showMinMax' in props ? " d-none d-sm-block" : "")}>
                    {this.props.unit}
                </div>
                {'onValue' in props ?
                    <>
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
                    </>
                    : <></>
                }
            </div>
            {!('onValue' in props) || !props.showMinMax ? null :
                <ButtonGroup className="flex-wrap">
                    <Button variant="primary"
                            className="ml-2"
                            style="margin-right: .125rem !important;"
                            onClick={() => {
                                setTarget(props.min);
                            }}
                            >
                        {((floatMin - Math.trunc(floatMin) < Math.pow(10, -props.digits)) ? Math.trunc(floatMin) : util.toLocaleFixed(floatMin, props.digits)) + " " + props.unit}
                    </Button>
                    <Button variant="primary" onClick={() => {
                                setTarget(props.max);
                            }}
                            >
                        {((floatMax - Math.trunc(floatMax) < Math.pow(10, -props.digits)) ? Math.trunc(floatMax) : util.toLocaleFixed(floatMax, props.digits)) + " " + props.unit}
                    </Button>
                </ButtonGroup>
            }
        </div>
    );
}
