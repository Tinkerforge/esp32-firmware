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

import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { CronTriggerID } from "../cron/cron_defs";
import { CronTrigger } from "../cron/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import * as util from "../../ts/util";

export type RtcCronTrigger = [
    CronTriggerID.Cron,
    {
        mday: number,
        wday: number,
        hour: number,
        minute: number
    }
];

function new_rtc_config(): RtcCronTrigger {
    return [
        CronTriggerID.Cron,
        {
            mday: -1,
            wday: -1,
            hour: -1,
            minute: -1,
        },
    ];
}

function get_rtc_table_children(trigger: RtcCronTrigger) {
    return __("rtc.cron.cron_translation_function")(trigger[1].mday, trigger[1].wday, trigger[1].hour, trigger[1].minute);
}

function get_rtc_edit_children(trigger: RtcCronTrigger, on_trigger: (trigger: CronTrigger) => void) {
    let hours: [string, string][] = [['-1','*']];
    let minutes: [string, string][] = [['-1','*']];
    let days: [string, string][] = [
        ['-1', __("rtc.cron.every")],
        ['1', __("rtc.cron.monday")],
        ['2', __("rtc.cron.tuesday")],
        ['3', __("rtc.cron.wednesday")],
        ['4', __("rtc.cron.thursday")],
        ['5', __("rtc.cron.friday")],
        ['6', __("rtc.cron.saturday")],
        ['0', __("rtc.cron.sunday")],
        ['8', __("rtc.cron.weekdays")],
        ['9', __("rtc.cron.weekends")],
        ['10', __("rtc.cron.month_end")]
    ];

    const date = new Date();
    for (let i = 0; i <= 59; i++) {
        const numString = i < 10 ? "0" + i : i.toString();
        minutes.push([String(i), numString]);
        if (i != 0 && i <= 31) {
            days.push([String(i + 10), numString]);
        }
        if (i <= 23) {
            date.setHours(i);
            hours.push([String(i), date.toLocaleTimeString([], { hour: "2-digit" })]);
        }
    }

    const day = trigger[1].mday != -1 ? trigger[1].mday == 32 ? 10 : trigger[1].mday + 10 : trigger[1].wday;

    return [<>
        <FormRow label={__("rtc.cron.mday")}>
            <InputSelect
                items={days}
                value={day.toString()}
                onValue={(v) => {
                    const day = parseInt(v);
                    let mday = -1;
                    let wday = -1;

                    if (day == 10) {
                        mday = 32;
                    } else if (day > 10) {
                        mday = day - 10;
                    } else {
                        wday = day;
                    }

                    on_trigger(util.get_updated_union(trigger, {mday: mday, wday: wday}));
                }} />
        </FormRow>
        <FormRow label={__("rtc.cron.time")}>
            <div class="input-group mb-2">
                <InputSelect
                    items={hours}
                    value={trigger[1].hour.toString()}
                    onValue={(v) => {
                        on_trigger(util.get_updated_union(trigger, {hour: parseInt(v)}));
                    }} />
                <InputSelect
                    items={minutes}
                    value={trigger[1].minute.toString()}
                    onValue={(v) => {
                        on_trigger(util.get_updated_union(trigger, {minute: parseInt(v)}));
                    }} />
            </div>
        </FormRow>
    </>]
}

export function init() {
    return {
        trigger_components: {
            [CronTriggerID.Cron]: {
                name: __("rtc.cron.clock"),
                new_config: new_rtc_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_rtc_edit_children,
                get_table_children: get_rtc_table_children,
            },
        },
    };
}
