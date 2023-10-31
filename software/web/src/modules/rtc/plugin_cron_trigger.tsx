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
import { Cron } from "../cron/main";
import { InputSelect } from "../../ts/components/input_select";

export type RtcCronTrigger = [
    CronTriggerID.Cron,
    {
        mday: number,
        wday: number,
        hour: number,
        minute: number
    }
];

function get_rtc_table_children(trigger: CronTrigger) {
    const value = (trigger as RtcCronTrigger)[1];

    return __("rtc.content.cron_translation_function")(value.mday, value.wday, value.hour, value.minute);
}

function get_rtc_edit_children(cron: Cron, trigger: CronTrigger) {
    const value = (trigger as RtcCronTrigger)[1];

    let hours: [string, string][] = [['-1','*']];
    let minutes: [string, string][] = [['-1','*']];
    let days: [string, string][] = [
        ['-1', __("rtc.content.every")],
        ['1', __("rtc.content.monday")],
        ['2', __("rtc.content.tuesday")],
        ['3', __("rtc.content.wednesday")],
        ['4', __("rtc.content.thursday")],
        ['5', __("rtc.content.friday")],
        ['6', __("rtc.content.saturday")],
        ['0', __("rtc.content.sunday")],
    ];

    const date = new Date();
    for (let i = 0; i <= 59; i++) {
        const numString = i < 10 ? "0" + i : i.toString();
        minutes.push([String(i), numString]);
        if (i != 0 && i <= 31) {
            days.push([String(i + 7), numString]);
        }
        if (i <= 23) {
            date.setHours(i);
            hours.push([String(i), date.toLocaleTimeString([], { hour: "2-digit" })]);
        }
    }

    const day = value.mday != -1 ? value.mday + 7 : value.wday;

    return [
        {
            name: __("rtc.content.mday"),
            value: <InputSelect
                    items={days}
                    value={day}
                    onValue={(v) => {
                        const day = Number(v);
                        if (day > 6) {
                            value.mday = day - 7;
                            value.wday = -1;
                        } else {
                            value.mday = -1;
                            value.wday = day;
                        }
                        cron.setTriggerFromComponent(trigger);
                    }} />
        },
        {
            name: __("rtc.content.time"),
            value:  <>
                <div class="input-group mb-2">
                    <InputSelect
                        items={hours}
                        value={value.hour}
                        onValue={(v) => {
                            value.hour = Number(v);
                            cron.setTriggerFromComponent(trigger);
                        }} />

                    <InputSelect
                    items={minutes}
                    value={value.minute}
                    onValue={(v) => {
                        value.minute = Number(v);
                        cron.setTriggerFromComponent(trigger);
                    }} />
                </div>
                <span>{__("rtc.content.cron_translation_function")(value.mday, value.wday, value.hour, value.minute)}</span>
            </>
        }
    ]
}

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

export function init() {
    return {
        trigger_components: {
            [CronTriggerID.Cron]: {
                name: __("rtc.content.clock"),
                new_config: new_rtc_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_rtc_edit_children,
                get_table_children: get_rtc_table_children,
            },
        },
    };
}
