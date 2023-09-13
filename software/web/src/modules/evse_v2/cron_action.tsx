import { CronActionID } from "../cron/cron_defs";

export type EvseGpOutputCronAction = [
    CronActionID.EVSEGPOutput,
    {
        state: number
    }
];

import { __ } from "../../ts/translation";
import { CronComponent, CronAction, cron_action_components} from "../cron/api";
import { h } from 'preact'
import { InputSelect } from "../../ts/components/input_select";
import { Cron } from "../cron/main";

function EvseGpioOutputCronActionComponent(action: CronAction): CronComponent {
    const value = (action as EvseGpOutputCronAction)[1];
    return {
        text: value.state ? __("evse.content.gpio_out_high") : __("evse.content.gpio_out_low"),
        fieldNames: [],
        fieldValues: [
            value.state ? __("evse.content.gpio_out_high") : __("evse.content.gpio_out_low")
        ]
    }
}

function EvseGpioOutputCronActionConfigComponent(cron: Cron, action: CronAction) {
    const value = (action as EvseGpOutputCronAction)[1];
    return [
        {
            name: __("evse.content.gpio_out"),
            value: <InputSelect
                items={[
                    ["0", __("evse.content.gpio_out_low")],
                    ["1", __("evse.content.gpio_out_high")]
                ]}
                value={value.state}
                onValue={(v) => {
                    value.state = Number(v);
                    cron.setActionFromComponent(action);
                }}/>
        }
    ]
}

function EvseGpioOutputCronActionConfigFactory(): CronAction {
    return [
        CronActionID.EVSEGPOutput,
        {
            state: 0
        }
    ];
}

export function init() {
    cron_action_components[CronActionID.EVSEGPOutput] = {
        config_builder: EvseGpioOutputCronActionConfigFactory,
        config_component: EvseGpioOutputCronActionConfigComponent,
        table_row: EvseGpioOutputCronActionComponent,
        name: __("evse.content.gpio_out"),
        require_feature: "button_configuration"
    };
}
