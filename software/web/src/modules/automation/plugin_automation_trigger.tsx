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

import { h } from "preact";
import { __ } from "../../ts/translation";
import { AutomationTriggerID } from "../automation/automation_trigger_id.enum";
import { AutomationTrigger, InitResult } from "../automation/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { InputText } from "src/ts/components/input_text";

export type CronAutomationTrigger = [
    AutomationTriggerID.Cron,
    {
        mday: number;
        wday: number;
        hour: number;
        minute: number;
    },
];

function new_cron_config(): CronAutomationTrigger {
    return [
        AutomationTriggerID.Cron,
        {
            mday: -1,
            wday: -1,
            hour: -1,
            minute: -1,
        },
    ];
}

function get_cron_table_children(trigger: CronAutomationTrigger) {
    return __("automation.automation.cron_translation_function")(trigger[1].mday, trigger[1].wday, trigger[1].hour, trigger[1].minute);
}

function get_cron_edit_children(trigger: CronAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    let hours: [string, string][] = [['-1',__("automation.automation.cron_every_hour")]];
    let minutes: [string, string][] = [['-1',__("automation.automation.cron_every_minute")]];
    let days: [string, string][] = [
        ['-1', __("automation.automation.cron_every_day")],
        ['1', __("automation.automation.cron_monday")],
        ['2', __("automation.automation.cron_tuesday")],
        ['3', __("automation.automation.cron_wednesday")],
        ['4', __("automation.automation.cron_thursday")],
        ['5', __("automation.automation.cron_friday")],
        ['6', __("automation.automation.cron_saturday")],
        ['0', __("automation.automation.cron_sunday")],
        ['8', __("automation.automation.cron_weekdays")],
        ['9', __("automation.automation.cron_weekends")],
        ['10', __("automation.automation.cron_month_end")]
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

    return [
        <FormRow label={__("automation.automation.cron_mday")}>
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
        </FormRow>,
        <FormRow label={__("automation.automation.cron_time")}>
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
        </FormRow>,
    ];
}

const enum HttpTriggerMethod {
    GET = 0,
    POST,
    PUT,
    POST_PUT,
    GET_POST_PUT
};

export type HTTPAutomationTrigger = [
    AutomationTriggerID.HTTP,
    {
        method: HttpTriggerMethod,
        url_suffix: string,
        payload: string
    }
];

function new_http_config(): HTTPAutomationTrigger {
    return [
        AutomationTriggerID.HTTP,
        {
            method: HttpTriggerMethod.GET_POST_PUT,
            url_suffix: "",
            payload: ""
        },
    ];
}

function suffix_to_url(url_suffix: string) {
    return "http://" + API.get("network/config").hostname + "/automation_trigger/" + url_suffix
}

function get_http_table_children(trigger: HTTPAutomationTrigger) {
    return __("automation.automation.http_translation_function")(trigger[1].method, suffix_to_url(trigger[1].url_suffix), trigger[1].payload);
}

function get_http_edit_children(trigger: HTTPAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("automation.automation.http_method")}>
            <InputSelect
                items={[
                    ['0', __("automation.automation.http_get")],
                    ['1', __("automation.automation.http_post")],
                    ['2', __("automation.automation.http_put")],
                    ['3', __("automation.automation.http_post_put")],
                    ['4', __("automation.automation.http_get_post_put")],
                ]}
                value={trigger[1].method.toString()}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {method: parseInt(v)}))} />
        </FormRow>,
        <FormRow label={__("automation.automation.http_url_suffix")}>
            <InputText
                required
                value={trigger[1].url_suffix}
                maxLength={32}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {url_suffix: v}))} />
        </FormRow>,
        <FormRow label={__("automation.automation.http_payload")}>
            <InputText
                placeholder={__("automation.automation.http_match_any")}
                maxLength={32}
                value={trigger[1].payload}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {payload: v}))} />
        </FormRow>,
    ];
}

export function init(): InitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.Cron]: {
                name: __("automation.automation.cron"),
                new_config: new_cron_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_edit_children: get_cron_edit_children,
                get_table_children: get_cron_table_children,
            },
            [AutomationTriggerID.HTTP]: {
                name: __("automation.automation.http"),
                new_config: new_http_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_edit_children: get_http_edit_children,
                get_table_children: get_http_table_children,
            },
        },
    };
}
