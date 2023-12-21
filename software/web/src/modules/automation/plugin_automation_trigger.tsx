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
import { AutomationTriggerID } from "../automation/automation_defs";
import { AutomationTrigger } from "../automation/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import * as util from "../../ts/util";

export type RtcAutomationTrigger = [
    AutomationTriggerID.Automation,
    {
        mday: number;
        wday: number;
        hour: number;
        minute: number;
    },
];

function new_rtc_config(): RtcAutomationTrigger {
    return [
        AutomationTriggerID.Automation,
        {
            mday: -1,
            wday: -1,
            hour: -1,
            minute: -1,
        },
    ];
}

function get_rtc_table_children(trigger: RtcAutomationTrigger) {
    return __("automation.automation.automation_translation_function")(trigger[1].mday, trigger[1].wday, trigger[1].hour, trigger[1].minute);
}

function get_rtc_edit_children(trigger: RtcAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    let hours: [string, string][] = [['-1','*']];
    let minutes: [string, string][] = [['-1','*']];
    let days: [string, string][] = [
        ['-1', __("automation.automation.every")],
        ['1', __("automation.automation.monday")],
        ['2', __("automation.automation.tuesday")],
        ['3', __("automation.automation.wednesday")],
        ['4', __("automation.automation.thursday")],
        ['5', __("automation.automation.friday")],
        ['6', __("automation.automation.saturday")],
        ['0', __("automation.automation.sunday")],
        ['8', __("automation.automation.weekdays")],
        ['9', __("automation.automation.weekends")],
        ['10', __("automation.automation.month_end")]
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
        <FormRow label={__("automation.automation.mday")}>
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
        <FormRow label={__("automation.automation.time")}>
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
            [AutomationTriggerID.Automation]: {
                name: __("automation.automation.clock"),
                new_config: new_rtc_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_edit_children: get_rtc_edit_children,
                get_table_children: get_rtc_table_children,
            },
        },
    };
}
