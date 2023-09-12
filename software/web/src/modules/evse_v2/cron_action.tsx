import { CronAction } from "../cron/cron_defs";

export interface EvseGpOutputCronAction {
    0: CronAction.EVSEGPOutput,
    1: {
        state: number
    }
}

import { __ } from "../../ts/translation";
import { CronComponent, cron_action, cron_action_components} from "../cron/api";
import { h } from 'preact'
import { InputSelect } from "../../ts/components/input_select";
import { Cron } from "../cron/main";

function EvseGpioOutputCronActionComponent(cron: cron_action): CronComponent {
    const state = (cron as any as EvseGpOutputCronAction)[1];
    return {
        text: state.state ? __("evse.content.gpio_out_high") : __("evse.content.gpio_out_low"),
        fieldNames: [],
        fieldValues: [
            state.state ? __("evse.content.gpio_out_high") : __("evse.content.gpio_out_low")
        ]
    }
}

function EvseGpioOutputCronActionConfigComponent(cron_object: Cron, props: cron_action) {
    const state = props as any as EvseGpOutputCronAction;
    return [
        {
            name: __("evse.content.gpio_out"),
            value: <InputSelect
                items={[
                    ["0", __("evse.content.gpio_out_low")],
                    ["1", __("evse.content.gpio_out_high")]
                ]}
                value={state[1].state}
                onValue={(v) => {
                    state[1].state = Number(v);
                    cron_object.setActionFromComponent(state as any);
                }}/>
        }
    ]
}

function EvseGpioOutputCronActionConfigFactory(): cron_action {
    return [
        CronAction.EVSEGPOutput as any,
        {
            state: 0
        }
    ];
}

export function init() {
    cron_action_components[CronAction.EVSEGPOutput] = {
        config_builder: EvseGpioOutputCronActionConfigFactory,
        config_component: EvseGpioOutputCronActionConfigComponent,
        table_row: EvseGpioOutputCronActionComponent,
        name: __("evse.content.gpio_out"),
        require_feature: "button_configuration"
    };
}
