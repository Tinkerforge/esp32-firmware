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
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { __ } from "../../ts/translation";
import { ConfigForm } from "../../ts/components/config_form";
import { SubPage } from "../../ts/components/sub_page";


type RTCTime = API.getType['rtc/time'];
type RTCConfig = API.getType['rtc/config'];

interface RtcPageState {
    state: RTCTime
}

export class Rtc extends ConfigComponent<'rtc/config', {}, RtcPageState> {
    constructor() {
        super('rtc/config',
              __("rtc.script.save_failed"),
              __("rtc.script.reboot_content_changed"));

        util.addApiEventListener("rtc/time", () =>{
            let time = API.get("rtc/time");

            if (!this.state.state)
            {
                window.setTimeout(() => {
                    if (API.get("rtc/config").auto_sync && !API.get("ntp/state").synced)
                        this.set_current_time();
                }, 1000);
            }

            this.setState({state:{ year: time.year,
                                    month: time.month,
                                    day: time.day,
                                    hour: time.hour,
                                    minute: time.minute,
                                    second: time.second,
                                    weekday: time.weekday}});
        });
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
        let time: RTCTime = {
            year: date.getUTCFullYear(),
            month: date.getUTCMonth() + 1,
            day: date.getUTCDate(),
            hour: date.getUTCHours(),
            minute: date.getUTCMinutes(),
            second: date.getUTCSeconds(),
            weekday: date.getUTCDay()
        };

        API.save("rtc/time", time, __("rtc.script.save_failed"));
    }


    render(props: {}, state: RTCConfig & RtcPageState) {
        if (!util.render_allowed() || !API.hasFeature("rtc"))
            return <></>

        return <SubPage>
                    <ConfigForm id="rtc_config_form"
                                title={__("rtc.content.rtc")}
                                onSave={this.save}
                                onReset={this.reset}
                                onDirtyChange={(d) => this.ignore_updates = d}
                                isModified={this.isModified()}>
                        <FormRow label={__("rtc.content.enable_auto_sync")}>
                            <Switch desc={__("rtc.content.auto_sync_desc")} checked={state.auto_sync} onClick={() => {
                                    this.setState({"auto_sync": !state.auto_sync})
                            }}/>
                        </FormRow>
                        <FormRow label={__("rtc.content.live_date")}>
                            <OutputDatetime date={new Date(state.state.year.toString()+ "-" +
                                                        this.add_leading_zero(state.state.month) + "-" +
                                                        this.add_leading_zero(state.state.day) + "T" +
                                                        this.add_leading_zero(state.state.hour) + ":" +
                                                        this.add_leading_zero(state.state.minute) + ":" +
                                                        this.add_leading_zero(state.state.second) + ".000Z")}
                                            onClick={() => this.set_current_time()}
                                            buttonText={__("rtc.content.set_time")}
                                            disabled={this.state.auto_sync}/>
                        </FormRow>
                    </ConfigForm>
                </SubPage>
        }
}

render(<Rtc/>, $('#rtc')[0])

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('info/features', () => $('#sidebar-rtc').prop('hidden', !API.hasFeature('rtc')));
}

export function init() {
}

export function update_sidebar_state(module_init: any) {}
