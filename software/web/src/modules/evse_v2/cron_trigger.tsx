import { CronComponent, cron_trigger, cron_trigger_components } from "../cron/api"
import { h } from 'preact'
import { Cron } from "../cron/main"
import { __ } from "src/ts/translation"
import { InputSelect } from "src/ts/components/input_select"

export interface EvseSdCronTrigger {
    0: 7,
    1: {
        high: boolean
    }
}

export interface EvseGpioCronTrigger {
    0: 8,
    1: {
        high: boolean
    }
}

export interface EvseButtonCronTrigger {
    0: 4,
    1: {
        button_pressed: boolean
    }
}

function EvseButtonCronTriggerComponent(cron: cron_trigger): CronComponent {
    const props = (cron as any as EvseButtonCronTrigger)[1];
    return {
        text: props.button_pressed ? __("evse.content.button_pressed") : __("evse.content.button_released"),
        fieldNames: [

        ],
        fieldValues: [
            props.button_pressed ? __("evse.content.button_pressed") : __("evse.content.button_released")
        ]
    }
}

function EvseButtonCronTriggerConfig(cron_object: Cron, props: cron_trigger) {
    let state = props as any as EvseButtonCronTrigger;
    if (state[1] == undefined) {
        state = EvseButtonCronTriggerFactory() as any;
    }
    return [
        {
            name: __("evse.content.button_configuration"),
            value: <InputSelect
                items={[
                    ["0", __("evse.content.button_released")],
                    ["1", __("evse.content.button_pressed")]
                ]}
            value={state[1].button_pressed ? "1": "0"}
            onValue={(v) => {
                state[1].button_pressed = v == "1";
                cron_object.setTriggerFromComponent(state as any);
            }}/>
        }
    ]
}

function EvseButtonCronTriggerFactory(): cron_trigger {
    return [
        4 as any,
        {
            button_pressed: true
        }
    ]
}

cron_trigger_components[4] = {
    config_builder: EvseButtonCronTriggerFactory,
    config_component: EvseButtonCronTriggerConfig,
    table_row: EvseButtonCronTriggerComponent,
    name: __("evse.content.button_configuration"),
    require_feature: "button_configuration"
};

function EvseShutdownTriggerComponent(cron: cron_trigger): CronComponent {
    const props = (cron as any as EvseSdCronTrigger)[1];
    return {
        text: props.high ? __("evse.content.active_high") : __("evse.content.active_low"),
        fieldNames: [
            __("evse.content.gpio_state")
        ],
        fieldValues: [
            props.high ? __("evse.content.active_high") : __("evse.content.active_low")
        ]
    }
}

function EvseShutdownTriggerConfig(cron_object: Cron, props: cron_trigger) {
    const state = props as any as EvseSdCronTrigger;
    return [
        {
            name: __("evse.content.gpio_shutdown"),
            value: <InputSelect
                items={[
                    ["0", __("evse.content.active_low")],
                    ["1", __("evse.content.active_high")]
                ]}
                value={state[1].high ? "1" : "0"}
                onValue={(v) => {
                    state[1].high = v == "1";
                    cron_object.setTriggerFromComponent(state as any);
                }} />
        }
    ]
}

function EvseShutdownTriggerFactory(): cron_trigger {
    return [
        7 as any,
        {
            high: true
        }
    ];
}

cron_trigger_components[7] = {
    config_component: EvseShutdownTriggerConfig,
    table_row: EvseShutdownTriggerComponent,
    config_builder: EvseShutdownTriggerFactory,
    name: __("evse.content.gpio_shutdown"),
    require_feature: "button_configuration"
};

function EvseGpioInputCronTriggerComponent(cron: cron_trigger): CronComponent {
    const props = (cron as any as EvseGpioCronTrigger)[1];
    return {
        text: props.high ? __("evse.content.active_high") : __("evse.content.active_low"),
        fieldNames: [
            __("evse.content.gpio_state")
        ],
        fieldValues: [
            props.high ? __("evse.content.active_high") : __("evse.content.active_low")
        ]
    }
}

function EvseGpioInputCrontTriggerConfigComponent(cron_object: Cron, props: cron_trigger) {
    const state = props as any as EvseGpioCronTrigger;
    return [
        {
            name: __("evse.content.gpio_in"),
            value: <InputSelect
                items={[
                    ["0", __("evse.content.active_low")],
                    ["1", __("evse.content.active_high")]
                ]}
                value={state[1].high ? "1" : "0"}
                onValue={(v) => {
                    state[1].high = v == "1";
                    cron_object.setTriggerFromComponent(state as any);
                }} />
        }
    ]
}

function EvseGpioInputCronTriggerConfigFactory(): cron_trigger {
    return [
        8 as any,
        {
            high: true
        }
    ]
}

cron_trigger_components[8] = {
    config_builder: EvseGpioInputCronTriggerConfigFactory,
    config_component: EvseGpioInputCrontTriggerConfigComponent,
    table_row: EvseGpioInputCronTriggerComponent,
    name: __("evse.content.gpio_in"),
    require_feature: "button_configuration"
}
