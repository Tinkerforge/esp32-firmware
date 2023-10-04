import { CronTriggerID } from "../cron/cron_defs";

export type RtcCronTrigger = [
    CronTriggerID.Cron,
    {
        mday: number,
        wday: number,
        hour: number,
        minute: number
    }
];

import { h, Fragment } from "preact"
import { __ } from "../../ts/translation";
import { CronComponent, CronTrigger, cron_trigger_components } from "../cron/api";
import { Cron } from "../cron/main";
import { InputSelect } from "src/ts/components/input_select";

export function RtcCronTriggerComponent(trigger: CronTrigger): CronComponent {
    const value = (trigger as RtcCronTrigger)[1];

    const fieldNames = [
        __("rtc.content.mday"),
        __("rtc.content.wday"),
        __("rtc.content.hour"),
        __("rtc.content.minute")
    ];

    const wdays: [string, string][] = [
        ['-1','*'],
        ['0', __("rtc.content.sunday")],
        ['1', __("rtc.content.monday")],
        ['2', __("rtc.content.tuesday")],
        ['3', __("rtc.content.wednesday")],
        ['4', __("rtc.content.thursday")],
        ['5', __("rtc.content.friday")],
        ['6', __("rtc.content.saturday")]

    ];
    const fieldValues = [
        value.mday,
        wdays[value.wday + 1][1],
        value.hour,
        value.minute
    ];

    let ret = "";
    fieldNames.map((val, idx) => {
        ret += val + ": " + (fieldValues[idx] == -1 ? "*" : fieldValues[idx]) + (idx != fieldValues.length - 1 ? ", " : "");
    })

    return {
        text: __("rtc.content.cron_translation_function")(value.mday, value.wday, value.hour, value.minute),
        fieldNames: fieldNames,
        fieldValues: fieldValues
    };
}

export function RtcCronTriggerConfigComponent(cron: Cron, trigger: CronTrigger) {
    const value = (trigger as RtcCronTrigger)[1];

    let hours: [string, string][] = [['-1','*']];
    let minutes: [string, string][] = [['-1','*']];
    let days: [string, string][] = [
        ['-1', __("rtc.content.every")],
        ['0', __("rtc.content.sunday")],
        ['1', __("rtc.content.monday")],
        ['2', __("rtc.content.tuesday")],
        ['3', __("rtc.content.wednesday")],
        ['4', __("rtc.content.thursday")],
        ['5', __("rtc.content.friday")],
        ['6', __("rtc.content.saturday")]
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

function RtcCronTriggerFactory(): RtcCronTrigger {
    return [
        CronTriggerID.Cron,
        {
            mday: -1,
            wday: -1,
            hour: -1,
            minute: -1
        }
    ];
}

export function init() {
    cron_trigger_components[CronTriggerID.Cron] = {
        table_row: RtcCronTriggerComponent,
        config_builder: RtcCronTriggerFactory,
        config_component: RtcCronTriggerConfigComponent,
        name: __("rtc.content.clock")
    }
}
