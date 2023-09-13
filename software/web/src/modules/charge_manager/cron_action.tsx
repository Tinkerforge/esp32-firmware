import { CronActionID } from "../cron/cron_defs";

export type ChargeManagerCronAction = [
    CronActionID.SetManagerCurrent,
    {
        current: number
    }
];

import { h } from "preact"
import { __ } from "../../ts/translation";
import { Cron } from "../cron/main";
import { CronComponent, CronAction, cron_action_components } from "../cron/api";
import { InputFloat } from "../../ts/components/input_float";

export function ChargeManagerCronComponent(action: CronAction): CronComponent {
    let value = (action as ChargeManagerCronAction)[1];
    return {
        text: __("charge_manager.content.maximum_available_current") + ": " + value.current / 1000 + " A",
        fieldNames: [__("charge_manager.content.maximum_available_current")],
        fieldValues: [value.current / 1000 + " A"]
    }
}

export function ChargeManagerCronConfigComponent(cron: Cron, action: CronAction) {
    let value = (action as ChargeManagerCronAction)[1];
    return [{
        name: "Maximaler Strom",
        value: <InputFloat value={value.current}
                    onValue={(v) => {
                        value.current = v;
                        cron.setActionFromComponent(action);
                    }}
                    min={0}
                    unit="A"
                    digits={3}/>
    }]
}

function ChargeManagerCronActionFactory(): CronAction {
    return [
        CronActionID.SetManagerCurrent,
        {
            current: 0
        }
    ]
}

export function init() {
    cron_action_components[CronActionID.SetManagerCurrent] = {
        config_builder: ChargeManagerCronActionFactory,
        config_component: ChargeManagerCronConfigComponent,
        table_row: ChargeManagerCronComponent,
        name: __("charge_manager.content.set_charge_manager")
    };
}
