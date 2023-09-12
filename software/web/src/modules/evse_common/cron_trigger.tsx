import { CronTrigger } from "../cron/cron_defs";

export interface EvseStateCronTrigger {
    0: CronTrigger.IECChange,
    1: {
        charger_state: number
    }
}

import { __ } from "../../ts/translation";
import { Cron } from "../cron/main";
import { CronComponent, cron_trigger, cron_trigger_components } from "../cron/api";
import { InputSelect } from "../../ts/components/input_select";
import { h } from "preact"

export function EvseStateCronComponent(cron: cron_trigger): CronComponent {
    let trigger_props = cron as any as EvseStateCronTrigger;
    const names = [
        [ __("evse.status.not_connected")],
        [__("evse.status.waiting_for_charge_release")],
        [__("evse.status.ready_to_charge")],
        [__("evse.status.charging")],
        [__("evse.status.error")]
    ]

    const fieldNames = [
        __("evse.content.status")
    ];
    const fieldValues = [
        names[trigger_props[1].charger_state]
    ];

    return {
        text: __("evse.content.status") + ": " + names[trigger_props[1].charger_state],
        fieldNames,
        fieldValues
    }
}

function EvseStateCronFactory(): cron_trigger {
    return [
        CronTrigger.IECChange as any,
        {
            charger_state: 0
        }
    ]
}

export function EvseStateCronConfig(cron_object: Cron, state: cron_trigger) {
    let props = state as any as EvseStateCronTrigger;
    if (props[1] == undefined) {
        props = EvseStateCronFactory() as any;
    }
    return [{
        name: __("evse.content.status"),
        value: <InputSelect
                    items={[
                        ["0", __("evse.status.not_connected")],
                        ["1", __("evse.status.waiting_for_charge_release")],
                        ["2",    __("evse.status.ready_to_charge")],
                        ["3", __("evse.status.charging")],
                        ["4",  __("evse.status.error")]
                    ]}
                    value={props[1].charger_state.toString()}
                    onValue={(v) => {
                        props[1].charger_state = Number(v);
                        cron_object.setTriggerFromComponent(props as any as cron_trigger);
                    }}/>
    }]
}

export function init() {
    cron_trigger_components[CronTrigger.IECChange] = {
        config_builder: EvseStateCronFactory,
        config_component: EvseStateCronConfig,
        table_row: EvseStateCronComponent,
        name: __("evse.content.state_change")
    };
}
