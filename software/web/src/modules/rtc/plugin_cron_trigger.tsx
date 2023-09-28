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
import { InputNumber } from "src/ts/components/input_number";
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
        text: ret,
        fieldNames: fieldNames,
        fieldValues: fieldValues
    };
}

export function RtcCronTriggerConfigComponent(cron: Cron, trigger: CronTrigger) {
    const value = (trigger as RtcCronTrigger)[1];

    let mdays: [string, string][] = [['-1','*']];
    let hours: [string, string][] = [['-1','*']];
    let minutes: [string, string][] = [['-1','*']];
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

    for (let i = 0; i <= 59; i++) {
        minutes.push([String(i), String(i)]);
        if (i != 0 && i <= 31) {
            mdays.push([String(i), String(i)]);
        }
        if (i <= 23) {
            hours.push([String(i), String(i)]);
        }
    }

    return [
        {
            name: __("rtc.content.mday"),
            value: <InputSelect
                    items={mdays}
                    value={value.mday}
                    onValue={(v) => {
                        value.mday = Number(v);
                        cron.setTriggerFromComponent(trigger);
                    }} />
        },
        {
            name: __("rtc.content.wday"),
            value: <InputSelect
                items={wdays}
                value={value.wday}
                onValue={(v) => {
                    value.wday = Number(v);
                    cron.setTriggerFromComponent(trigger);
                }} />
        },
        {
            name: __("rtc.content.hour"),
            value: <InputSelect
                items={hours}
                value={value.hour}
                onValue={(v) => {
                    value.hour = Number(v);
                    cron.setTriggerFromComponent(trigger);
                }} />
        },
        {
            name: __("rtc.content.minute"),
            value: <InputSelect
                items={minutes}
                value={value.minute}
                onValue={(v) => {
                    value.minute = Number(v);
                    cron.setTriggerFromComponent(trigger);
                }} />
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
        name: __("rtc.content.rtc")
    }
}
