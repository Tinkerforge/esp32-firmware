/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

import { h, Component, Fragment } from "preact";
import { Card, Row, Button } from "react-bootstrap"
import { __ } from "../translation"

interface DayCardState {
    day: string
    onValue: (value: number) => void
    configuredTimes: number
}

class DayCard extends Component<DayCardState, any>
{
    times: string[] = [];

    constructor()
    {
        super();

        for (let i = 0; i < 24; i++)
            this.times.push(i.toString());
    }

    hackToAllowSave() {
        document.getElementById("charge_condition_config_form").dispatchEvent(new Event('input'));
    }

    isBitSet(number: number, bit: number): boolean
    {
        bit = 1 << bit;
        let test = number & bit;
        return test != 0 ? true : false;
    }

    xorBit(number: number, bit: number): number
    {
        bit = 1 << bit;
        return number ^ bit
    }

    render(props: DayCardState)
    {
        if (!props || props.configuredTimes === undefined)
            return <></>;

        return <Card>
                <Card.Header>
                    <Card.Title>{props.day}</Card.Title>
                </Card.Header>
                <Card.Body>
                        {this.times.map((val, idx) => {
                            return <Row>
                                        <Button className="mb-1 form-control"
                                            variant={this.isBitSet(props.configuredTimes, idx) ? "danger" : "primary"}
                                            onClick={() => {
                                                    this.hackToAllowSave();
                                                    this.props.onValue(this.xorBit(props.configuredTimes, idx));
                                                }}>
                                            {this.times[idx]}
                                        </Button>
                                    </Row>
                        })}
                </Card.Body>
            </Card>
    }
}

interface DateTimePickerProps
{
    value: number[]
    onValue: (value: number[]) => void
}

export class DateTimePicker extends Component<DateTimePickerProps, any>
{
    handle_change(idx: number, e: number)
    {
        const times = this.props.value;
        times.splice(idx, 1, e)
        this.setState({value: [...times]});
    }

    render(props: DateTimePickerProps)
    {
        if (!props || !props.value)
            return <></>;

        return (<>
                    <Row class="form-control">
                    <div class="col" style="width: 100%">
                        <DayCard day={"Mo"} configuredTimes={props.value[0]} onValue={(v) => {this.handle_change(0, v); props.onValue(props.value);}}/>
                    </div>
                    <div class="col" style="width: 100%">
                        <DayCard day={"Tu"} configuredTimes={props.value[1]} onValue={(v) => {this.handle_change(1, v); props.onValue(props.value);}}/>
                    </div>
                    <div class="col" style="width: 100%">
                        <DayCard day={"We"} configuredTimes={props.value[2]} onValue={(v) => {this.handle_change(2, v); props.onValue(props.value);}}/>
                    </div>
                    <div class="col" style="width: 100%">
                        <DayCard day={"Th"} configuredTimes={props.value[3]} onValue={(v) => {this.handle_change(3, v); props.onValue(props.value);}}/>
                    </div>
                    <div class="col" style="width: 100%">
                        <DayCard day={"Fr"} configuredTimes={props.value[4]} onValue={(v) => {this.handle_change(4, v); props.onValue(props.value);}}/>
                    </div>
                    <div class="col" style="width: 100%">
                        <DayCard day={"Sa"} configuredTimes={props.value[5]} onValue={(v) => {this.handle_change(5, v); props.onValue(props.value);}}/>
                    </div>
                    <div class="col" style="width: 100%">
                        <DayCard day={"Su"} configuredTimes={props.value[6]} onValue={(v) => {this.handle_change(6, v); props.onValue(props.value);}}/>
                    </div>
                    </Row>
                </>
            )
    }
}