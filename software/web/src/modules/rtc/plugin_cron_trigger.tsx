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
import { FormRow } from "../../ts/components/form_row";

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

    return __("rtc.cron.cron_translation_function")(value.mday, value.wday, value.hour, value.minute);
}

function get_rtc_edit_children(cron: Cron, trigger: CronTrigger) {
    const value = (trigger as RtcCronTrigger)[1];

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

    const day = value.mday != -1 ? value.mday == 32 ? 10 : value.mday + 7 : value.wday;

    console.log(day);

    return [<>
        <FormRow label={__("rtc.cron.mday")}>
            <InputSelect
                items={days}
                value={day}
                onValue={(v) => {
                    const day = Number(v);
                    if (day == 10) {
                        value.mday = 32;
                        value.wday = -1;
                    } else if (day > 9) {
                        value.mday = day - 9;
                        value.wday = -1;
                    } else {
                        value.mday = -1;
                        value.wday = day;
                    }
                    cron.setTriggerFromComponent(trigger);
                }} />
        </FormRow>
        <FormRow label={__("rtc.cron.time")}>
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
        </FormRow>
    </>]
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
                name: __("rtc.cron.clock"),
                new_config: new_rtc_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_rtc_edit_children,
                get_table_children: get_rtc_table_children,
            },
        },
    };
}
