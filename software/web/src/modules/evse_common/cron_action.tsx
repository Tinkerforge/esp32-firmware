export interface EvseCronAction {
    0: 3,
    1: {
        current: number
    }
}

export interface EvseLedCronAction {
    0: 4,
    1: {
        state: number,
        duration: number
    }
}

import { __ } from "src/ts/translation"
import { CronComponent, cron_action, cron_action_components } from "../cron/api"
import { Cron } from "../cron/main"
import { InputSelect } from "src/ts/components/input_select"
import { InputFloat } from "src/ts/components/input_float"
import { h } from 'preact'
import { InputNumber } from "src/ts/components/input_number"

function EvseSetCurrentCronActionComponent(cron: cron_action): CronComponent {
    const props = (cron as any as EvseCronAction)[1];
    return {
        text: __("evse.content.allowed_charging_current") + ": " + props.current / 1000 + " A",
        fieldNames: [__("evse.content.allowed_charging_current")],
        fieldValues: [props.current / 1000 + " A"]
    };
}

function EvseSetCurrentCronActionConfigComponent(cron_object: Cron, props: cron_action) {
    const state = props as any as EvseCronAction;
    return [
        {
            name: __("evse.content.allowed_charging_current"),
            value: <InputFloat
                digits={3}
                min={0}
                max={32000}
                unit="A"
                value={state[1].current}
                onValue={(v) => {
                    state[1].current = v;
                    cron_object.setActionFromComponent(state as any);
                }}/>
        }
    ]
}

function EvseSetCurrentCronActionConfigFactory(): cron_action {
    return [
        3 as any,
        {
            current: 0
        }
    ]
}

cron_action_components[3] = {
    config_builder: EvseSetCurrentCronActionConfigFactory,
    config_component: EvseSetCurrentCronActionConfigComponent,
    table_row: EvseSetCurrentCronActionComponent,
    name: __("evse.content.allowed_charging_current")
}

function EvseLedCronActionComponent(cron: cron_action): CronComponent {
    const props = (cron as any as EvseLedCronAction)[1];
    let ret = __("evse.content.led_state") + ": ";
    const fieldNames = [
        __("evse.content.led_state"),
        __("evse.content.led_duration"),
    ];
    let state = ""
    switch (props.state) {
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
        props.duration + " ms",
        state
    ];
    ret = ret + "\n" + __("evse.content.led_duration") + ": " + props.duration + " ms"
    return {
        text: ret,
        fieldNames: fieldNames,
        fieldValues: fieldValues
    }
}

function EvseLedCronActionConfigComponent(cron_object: Cron, props: cron_action) {
    const state = props as any as EvseLedCronAction;
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
                value={state[1].state.toString()}
                onValue={(v) => {
                    state[1].state = parseInt(v);
                    cron_object.setActionFromComponent(state as any);
                }}/>
        },
        {
            name: __("evse.content.led_duration"),
            value: <InputNumber
                value={state[1].duration}
                unit="ms"
                onValue={(v) => {
                    state[1].duration = v;
                    cron_object.setActionFromComponent(state as any);
                }} />
        }
    ]
}

function EvseLedCronActionConfigFactory(): cron_action {
    return [
        4 as any,
        {
            duration: 0,
            state: 0
        }
    ]
}

cron_action_components[4] = {
    config_builder: EvseLedCronActionConfigFactory,
    config_component: EvseLedCronActionConfigComponent,
    table_row: EvseLedCronActionComponent,
    name: __("evse.content.led_state")
};
