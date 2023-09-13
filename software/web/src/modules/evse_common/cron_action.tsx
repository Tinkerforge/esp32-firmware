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
import { CronComponent, CronAction, cron_action_components } from "../cron/api"
import { Cron } from "../cron/main"
import { InputSelect } from "../../ts/components/input_select"
import { InputFloat } from "../../ts/components/input_float"
import { h } from 'preact'
import { InputNumber } from "../../ts/components/input_number"

function EvseSetCurrentCronActionComponent(action: CronAction): CronComponent {
    const value = (action as EvseCronAction)[1];
    return {
        text: __("evse.content.allowed_charging_current") + ": " + value.current / 1000 + " A",
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
    let ret = __("evse.content.led_state") + ": ";
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
    }

    const fieldValues = [
        value.duration + " ms",
        state
    ];
    ret = ret + "\n" + __("evse.content.led_duration") + ": " + value.duration + " ms"
    return {
        text: ret,
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
                    ["1003", __("evse.content.led_state_breathing")]
                ]}
                value={value.state.toString()}
                onValue={(v) => {
                    value.state = parseInt(v);
                    cron.setActionFromComponent(action);
                }}/>
        },
        {
            name: __("evse.content.led_duration"),
            value: <InputNumber
                value={value.duration}
                unit="ms"
                onValue={(v) => {
                    value.duration = v;
                    cron.setActionFromComponent(action);
                }} />
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
    cron_action_components[CronActionID.SetCurrent] = {
        config_builder: EvseSetCurrentCronActionConfigFactory,
        config_component: EvseSetCurrentCronActionConfigComponent,
        table_row: EvseSetCurrentCronActionComponent,
        name: __("evse.content.allowed_charging_current")
    };

    cron_action_components[CronActionID.LED] = {
        config_builder: EvseLedCronActionConfigFactory,
        config_component: EvseLedCronActionConfigComponent,
        table_row: EvseLedCronActionComponent,
        name: __("evse.content.led_state")
    };
}
