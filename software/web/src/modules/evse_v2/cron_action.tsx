export interface EvseGpOutputCronAction {
    0: 9,
    1: {
        state: number
    }
}

import { __ } from "src/ts/translation";
import { cron_action, cron_action_components} from "../cron/api";
import { h } from 'preact'
import { InputSelect } from "src/ts/components/input_select";
import { Cron } from "../cron/main";

function EvseGpioOutputCronActionComponent(cron: cron_action) {
    const state = (cron as any as EvseGpOutputCronAction)[1];
    return state.state ? __("evse.content.gpio_out_high") : __("evse.content.gpio_out_low");
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
        9 as any,
        {
            state: 0
        }
    ];
}

cron_action_components[9] = {
    config_builder: EvseGpioOutputCronActionConfigFactory,
    config_component: EvseGpioOutputCronActionConfigComponent,
    table_row: EvseGpioOutputCronActionComponent,
    name: __("evse.content.gpio_out")
}
