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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { h, Fragment, Component } from "preact";
import { ConfigComponent } from "../../ts/components/config_component";
import { OutputDatetime } from "../../ts/components/output_datetime";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { __ } from "../../ts/translation";
import { ConfigForm } from "../../ts/components/config_form";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Watch } from "react-feather";

export function RtcNavbar() {
    return <NavbarItem name="rtc" title={__("rtc.navbar.rtc")} symbol={<Watch />} hidden={!API.hasFeature("rtc")} />;
}

type RTCTime = API.getType['rtc/time'];

export class Rtc extends ConfigComponent<'rtc/config'> {
    constructor() {
        super('rtc/config',
              __("rtc.script.save_failed"),
              __("rtc.script.reboot_content_changed"));

        window.setTimeout(() => {
                if (util.render_allowed() && API.hasFeature("rtc") && API.get("rtc/config").auto_sync && !API.get("ntp/state").synced) {
                    this.set_current_time();
                }
            },
            1000);
    }

    set_current_time() {
        let date = new Date();
        let time: RTCTime = {
            year: date.getUTCFullYear(),
            month: date.getUTCMonth() + 1,
            day: date.getUTCDate(),
            hour: date.getUTCHours(),
            minute: date.getUTCMinutes(),
            second: date.getUTCSeconds(),
            weekday: date.getUTCDay(),
        };

        API.save("rtc/time", time, __("rtc.script.save_failed"));
    }

    render(props: {}, state: API.getType['rtc/config']) {
        if (!util.render_allowed() || !API.hasFeature("rtc"))
            return <SubPage name="rtc" />;

        const p = (i: number) => util.leftPad(i, 0, 2);
        let t = API.get('rtc/time');
        let date = new Date(
            `20${p(t.year)}-${p(t.month)}-${p(t.day)}T${p(t.hour)}:${p(t.minute)}:${p(t.second)}.000Z`);

        return <SubPage name="rtc">
                    <ConfigForm id="rtc_config_form"
                                title={__("rtc.content.rtc")}
                                isModified={this.isModified()}
                                isDirty={this.isDirty()}
                                onSave={this.save}
                                onReset={this.reset}
                                onDirtyChange={this.setDirty}>
                        <FormRow label={__("rtc.content.enable_auto_sync")}>
                            <Switch desc={__("rtc.content.auto_sync_desc")} checked={state.auto_sync} onClick={() => {
                                    this.setState({"auto_sync": !state.auto_sync})
                            }}/>
                        </FormRow>
                        <FormRow label={__("rtc.content.live_date")}>
                            <OutputDatetime date={date}
                                            onClick={() => this.set_current_time()}
                                            buttonText={__("rtc.content.set_time")}
                                            disabled={this.state.auto_sync}
                                            invalidDateText={__("rtc.content.time_not_set")}/>
                        </FormRow>
                    </ConfigForm>
                </SubPage>
        }
}

export function init() {
}
