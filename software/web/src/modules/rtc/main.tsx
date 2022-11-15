/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

import $ from "../../ts/jq";

import * as API from "../../ts/api";
import * as util from "../../ts/util";

import { h, render, Fragment} from "preact";

import { ConfigComponent } from "../../ts/components/config_component";
import { OutputDatetime } from "../../ts/components/output_datetime";
import { FormRow } from "src/ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { __ } from "src/ts/translation";
import { ConfigForm } from "src/ts/components/config_form";


type RTCState = API.getType['rtc/state'];
type RTCConfig = API.getType['rtc/config'];

interface time {
    year: number
    month: number
    day: number
    hour: number
    minute: number
    second: number
    centisecond: number
    weekday: number
}

export class Rtc extends ConfigComponent<'rtc/state', {}, RTCConfig> {
    constructor() {
        super('rtc/state');

        util.eventTarget.addEventListener("rtc/state", () =>{
            let time = API.get("rtc/state");
            this.setState({ year: time.year,
                            month: time.month,
                            day: time.day,
                            hour: time.hour,
                            minute: time.minute,
                            second: time.second,
                            centisecond: time.centisecond,
                            weekday: time.weekday});
        });
        util.eventTarget.addEventListener("rtc/config", () => {
            this.setState({sync_enabled: API.get("rtc/config").sync_enabled})
        })
    }

    add_leading_zero(i: number)
    {
        if (i > 9)
            return i.toString();
        return '0' + i.toString();
    }

    set_current_time()
    {
        let date = new Date();
        let time: time = {
            year: date.getUTCFullYear(),
            month: date.getUTCMonth() + 1,
            day: date.getUTCDate(),
            hour: date.getUTCHours(),
            minute: date.getUTCMinutes(),
            second: date.getUTCSeconds(),
            centisecond: Math.round(date.getUTCMilliseconds() / 10),
            weekday: date.getUTCDay()
        };

        API.save("rtc/state", time, __("rtc.script.save_failed"));
    }


    render(props: {}, state: RTCState & RTCConfig) {
        if (!state || !state.year)
            return <></>;

        if (API.get("rtc/config").sync_enabled && !API.get("ntp/state").synced)
            this.set_current_time();

        return <>
                    <ConfigForm id="rtc_config_form"
                                title={__("rtc.content.rtc")}
                                onSave={this.save}
                                onDirtyChange={(d) => this.ignore_updates = d}>
                        <FormRow label={__("rtc.content.live_date")}>
                            <OutputDatetime date={new Date(state.year.toString()+ "-" +
                                                        this.add_leading_zero(state.month) + "-" +
                                                        this.add_leading_zero(state.day) + "T" +
                                                        this.add_leading_zero(state.hour) + ":" +
                                                        this.add_leading_zero(state.minute) + ":" +
                                                        this.add_leading_zero(state.second) + "." +
                                                        this.add_leading_zero(state.centisecond * 10) + "Z")}
                                            onClick={() => this.set_current_time()}
                                            buttonText={__("rtc.content.set_time")}
                                            disabled={this.state.sync_enabled}/>
                        </FormRow>
                        <FormRow label={__("rtc.content.enable_auto_sync")}>
                            <Switch desc={__("rtc.content.auto_sync_desc")} checked={state.sync_enabled} onClick={() => {
                                    API.save("rtc/config", {sync_enabled: !this.state.sync_enabled}, __("rtc.script.save_failed"));
                                    console.log(this.state.sync_enabled);
                            }}/>
                        </FormRow>
                    </ConfigForm>
                </>
        }
}

render(<Rtc/>, $('#rtc')[0])

export function add_event_listeners(source: API.APIEventTarget) {
}

export function init() {
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-rtc').prop('hidden', !module_init.rtc);
}
