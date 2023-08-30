export interface ChargeManagerCronAction {
    0: 6,
    1: {
        current: number
    }
}

import { h } from "preact"
import { __ } from "src/ts/translation";
import { Cron } from "../cron/main";
import { cron_action, cron_action_components } from "../cron/api";
import { InputFloat } from "src/ts/components/input_float";

export function ChargeManagerCronComponent(cron: cron_action) {
    let action_props = cron as any as ChargeManagerCronAction;
    return __("charge_manager.content.maximum_available_current") + ": " + action_props[1].current / 1000 + " A";
}

export function ChargeManagerCronConfigComponent(cron_object: Cron, state: cron_action) {
    let props = state as any as ChargeManagerCronAction;
    if (props[1] === undefined) {
        props = ChargeManagerCronActionFactory() as any;
    }
    return [{
        name: "Maximaler Strom",
        value: <InputFloat value={props[1].current}
                    onValue={(v) => {
                        props[1].current = v;
                        cron_object.setActionFromComponent(props as any as cron_action);
                    }}
                    min={0}
                    unit="A"
                    digits={3}/>
    }]
}

function ChargeManagerCronActionFactory(): cron_action {
    return [
        6 as any,
        {
            current: 0
        }
    ]
}

cron_action_components[6] = {
    config_builder: ChargeManagerCronActionFactory,
    config_component: ChargeManagerCronConfigComponent,
    table_row: ChargeManagerCronComponent,
    name: __("charge_manager.content.set_charge_manager")
};
