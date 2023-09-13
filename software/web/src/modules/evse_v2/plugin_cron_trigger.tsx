import { CronTriggerID } from "../cron/cron_defs";
import { CronComponent, CronTrigger, cron_trigger_components } from "../cron/api"
import { h } from 'preact'
import { Cron } from "../cron/main"
import { __ } from "../../ts/translation"
import { InputSelect } from "../../ts/components/input_select"

export type EvseSdCronTrigger = [
    CronTriggerID.EVSEShutdownInput,
    {
        high: boolean
    }
];

export type EvseGpioCronTrigger = [
    CronTriggerID.EVSEGPInput,
    {
        high: boolean
    }
];

export type EvseButtonCronTrigger = [
    CronTriggerID.EVSEButton,
    {
        button_pressed: boolean
    }
];

function EvseButtonCronTriggerComponent(trigger: CronTrigger): CronComponent {
    const value = (trigger as EvseButtonCronTrigger)[1];
    return {
        text: value.button_pressed ? __("evse.content.button_pressed") : __("evse.content.button_released"),
        fieldNames: [

        ],
        fieldValues: [
            value.button_pressed ? __("evse.content.button_pressed") : __("evse.content.button_released")
        ]
    }
}

function EvseButtonCronTriggerConfig(cron: Cron, trigger: CronTrigger) {
    let value = (trigger as EvseButtonCronTrigger)[1];
    return [
        {
            name: __("evse.content.button_configuration"),
            value: <InputSelect
                items={[
                    ["0", __("evse.content.button_released")],
                    ["1", __("evse.content.button_pressed")]
                ]}
            value={value.button_pressed ? "1": "0"}
            onValue={(v) => {
                value.button_pressed = v == "1";
                cron.setTriggerFromComponent(trigger);
            }}/>
        }
    ]
}

function EvseButtonCronTriggerFactory(): CronTrigger {
    return [
        CronTriggerID.EVSEButton,
        {
            button_pressed: true
        }
    ]
}

function EvseShutdownTriggerComponent(trigger: CronTrigger): CronComponent {
    const value = (trigger as EvseSdCronTrigger)[1];
    return {
        text: value.high ? __("evse.content.active_high") : __("evse.content.active_low"),
        fieldNames: [
            __("evse.content.gpio_state")
        ],
        fieldValues: [
            value.high ? __("evse.content.active_high") : __("evse.content.active_low")
        ]
    }
}

function EvseShutdownTriggerConfig(cron: Cron, trigger: CronTrigger) {
    const value = (trigger as EvseSdCronTrigger)[1];
    return [
        {
            name: __("evse.content.gpio_shutdown"),
            value: <InputSelect
                items={[
                    ["0", __("evse.content.active_low")],
                    ["1", __("evse.content.active_high")]
                ]}
                value={value.high ? "1" : "0"}
                onValue={(v) => {
                    value.high = v == "1";
                    cron.setTriggerFromComponent(trigger);
                }} />
        }
    ]
}

function EvseShutdownTriggerFactory(): CronTrigger {
    return [
        CronTriggerID.EVSEShutdownInput,
        {
            high: true
        }
    ];
}

function EvseGpioInputCronTriggerComponent(trigger: CronTrigger): CronComponent {
    const value = (trigger as EvseGpioCronTrigger)[1];
    return {
        text: value.high ? __("evse.content.active_high") : __("evse.content.active_low"),
        fieldNames: [
            __("evse.content.gpio_state")
        ],
        fieldValues: [
            value.high ? __("evse.content.active_high") : __("evse.content.active_low")
        ]
    }
}

function EvseGpioInputCrontTriggerConfigComponent(cron: Cron, trigger: CronTrigger) {
    const value = (trigger as EvseGpioCronTrigger)[1];
    return [
        {
            name: __("evse.content.gpio_in"),
            value: <InputSelect
                items={[
                    ["0", __("evse.content.active_low")],
                    ["1", __("evse.content.active_high")]
                ]}
                value={value.high ? "1" : "0"}
                onValue={(v) => {
                    value.high = v == "1";
                    cron.setTriggerFromComponent(trigger);
                }} />
        }
    ]
}

function EvseGpioInputCronTriggerConfigFactory(): CronTrigger {
    return [
        CronTriggerID.EVSEGPInput,
        {
            high: true
        }
    ]
}

export function init() {
    cron_trigger_components[CronTriggerID.EVSEButton] = {
        config_builder: EvseButtonCronTriggerFactory,
        config_component: EvseButtonCronTriggerConfig,
        table_row: EvseButtonCronTriggerComponent,
        name: __("evse.content.button_configuration"),
        require_feature: "button_configuration"
    };

    cron_trigger_components[CronTriggerID.EVSEShutdownInput] = {
        config_component: EvseShutdownTriggerConfig,
        table_row: EvseShutdownTriggerComponent,
        config_builder: EvseShutdownTriggerFactory,
        name: __("evse.content.gpio_shutdown"),
        require_feature: "button_configuration"
    };

    cron_trigger_components[CronTriggerID.EVSEGPInput] = {
        config_builder: EvseGpioInputCronTriggerConfigFactory,
        config_component: EvseGpioInputCrontTriggerConfigComponent,
        table_row: EvseGpioInputCronTriggerComponent,
        name: __("evse.content.gpio_in"),
        require_feature: "button_configuration"
    }
}
