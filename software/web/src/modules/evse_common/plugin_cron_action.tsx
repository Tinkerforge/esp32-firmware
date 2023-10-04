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

import { CronActionID } from "../cron/cron_defs";

export type EvseCronAction = [
    CronActionID.SetCurrent,
    {
        current: number
    }
];

export type EvseLedCronAction = [
    CronActionID.LED,
    {
        state: number,
        duration: number
    }
];

import { __ } from "../../ts/translation"
import { CronComponent, CronAction } from "../cron/types"
import { Cron } from "../cron/main"
import { InputSelect } from "../../ts/components/input_select"
import { InputFloat } from "../../ts/components/input_float"
import { h, Fragment } from 'preact'
import { InputNumber } from "../../ts/components/input_number"

function EvseSetCurrentCronActionComponent(action: CronAction): CronComponent {
    const value = (action as EvseCronAction)[1];
    return {
        text: __("evse.content.cron_action_text")(value.current / 1000),
        fieldNames: [__("evse.content.allowed_charging_current")],
        fieldValues: [value.current / 1000 + " A"]
    };
}

function EvseSetCurrentCronActionConfigComponent(cron: Cron, action: CronAction) {
    const value = (action as EvseCronAction)[1];
    return [
        {
            name: __("evse.content.allowed_charging_current"),
            value: <InputFloat
                digits={3}
                min={0}
                max={32000}
                unit="A"
                value={value.current}
                onValue={(v) => {
                    value.current = v;
                    cron.setActionFromComponent(action);
                }}/>
        }
    ]
}

function EvseSetCurrentCronActionConfigFactory(): CronAction {
    return [
        CronActionID.SetCurrent,
        {
            current: 0
        }
    ]
}

function EvseLedCronActionComponent(action: CronAction): CronComponent {
    const value = (action as EvseLedCronAction)[1];
    const fieldNames = [
        __("evse.content.led_state"),
        __("evse.content.led_duration"),
    ];
    let state = ""
    switch (value.state) {
        case 0:
            state = __("evse.content.led_state_off");
            break;

        case 255:
            state = __("evse.content.led_state_on");
            break;

        case 1001:
            state = __("evse.content.led_state_blinking");
            break;

        case 1002:
            state = __("evse.content.led_state_flickering");
            break;

        case 1003:
            state = __("evse.content.led_state_breathing");
            break;

        case 2001:
            state = __("evse.content.led_state_error");
            break;
    }

    const fieldValues = [
        value.duration + " ms",
        state
    ];
    return {
        text: __("evse.content.cron_led_action_text")(state, value.duration),
        fieldNames: fieldNames,
        fieldValues: fieldValues
    }
}

function EvseLedCronActionConfigComponent(cron: Cron, action: CronAction) {
    const value = (action as EvseLedCronAction)[1];
    return [
        {
            name: __("evse.content.led_state"),
            value: <InputSelect
                items={[
                    // TODO: Add more led-states
                    ["0", __("evse.content.led_state_off")],
                    ["255", __("evse.content.led_state_on")],
                    ["1001", __("evse.content.led_state_blinking")],
                    ["1002", __("evse.content.led_state_flickering")],
                    ["1003", __("evse.content.led_state_breathing")],
                    ["2001", __("evse.content.led_state_error")]
                ]}
                value={value.state.toString()}
                onValue={(v) => {
                    value.state = parseInt(v);
                    cron.setActionFromComponent(action);
                }}/>
        },
        {
            name: __("evse.content.led_duration"),
            value: <> <InputNumber
                value={value.duration}
                unit="ms"
                onValue={(v) => {
                    value.duration = v;
                    cron.setActionFromComponent(action);
                }} />
                <span class="text-muted mt-1">{__("evse.content.api_must_be_enabled")}</span>
            </>
        }
    ]
}

function EvseLedCronActionConfigFactory(): CronAction {
    return [
        CronActionID.LED,
        {
            duration: 0,
            state: 0
        }
    ]
}

export function init() {
    return {
        action_components: {
            [CronActionID.SetCurrent]: {
                clone: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                config_builder: EvseSetCurrentCronActionConfigFactory,
                config_component: EvseSetCurrentCronActionConfigComponent,
                table_row: EvseSetCurrentCronActionComponent,
                name: __("evse.content.allowed_charging_current")
            },
            [CronActionID.LED]: {
                clone: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                config_builder: EvseLedCronActionConfigFactory,
                config_component: EvseLedCronActionConfigComponent,
                table_row: EvseLedCronActionComponent,
                name: __("evse.content.led_state")
            }
        }
    };
}
