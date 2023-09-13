import { CronTriggerID } from "../cron/cron_defs";

export type EvseStateCronTrigger = [
    CronTriggerID.IECChange,
    {
        charger_state: number
    }
];

import { __ } from "../../ts/translation";
import { Cron } from "../cron/main";
import { CronComponent, CronTrigger, cron_trigger_components } from "../cron/api";
import { InputSelect } from "../../ts/components/input_select";
import { h } from "preact"

export function EvseStateCronComponent(trigger: CronTrigger): CronComponent {
    let value = (trigger as EvseStateCronTrigger)[1];
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
        names[value.charger_state]
    ];

    return {
        text: __("evse.content.status") + ": " + names[value.charger_state],
        fieldNames,
        fieldValues
    }
}

function EvseStateCronFactory(): CronTrigger {
    return [
        CronTriggerID.IECChange,
        {
            charger_state: 0
        }
    ]
}

export function EvseStateCronConfig(cron: Cron, trigger: CronTrigger) {
    let value = (trigger as EvseStateCronTrigger)[1];
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
                    value={value.charger_state.toString()}
                    onValue={(v) => {
                        value.charger_state = Number(v);
                        cron.setTriggerFromComponent(trigger);
                    }}/>
    }]
}

export function init() {
    cron_trigger_components[CronTriggerID.IECChange] = {
        config_builder: EvseStateCronFactory,
        config_component: EvseStateCronConfig,
        table_row: EvseStateCronComponent,
        name: __("evse.content.state_change")
    };
}
