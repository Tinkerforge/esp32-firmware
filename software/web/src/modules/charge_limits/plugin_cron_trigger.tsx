import { CronTriggerID } from "../cron/cron_defs";
import { TableModalRow } from '../../ts/components/table'

export type ChargeLimitsCronTrigger = [
    CronTriggerID.ChargeLimits,
    {}
];

import { __ } from "../../ts/translation";
import { Cron } from "../cron/main";
import { CronComponent, CronTrigger } from "../cron/types";

// TODO: Think about a nice explanation why this does not need a config
function ChargeLimitsCronTriggerComponent(_: CronTrigger): CronComponent {
    return {
        text: "",
        fieldNames: [],
        fieldValues: []
    };
}

function ChargeLimitsCronTriggerConfig(_: Cron, __: CronTrigger): TableModalRow[] {
    return []
}

function ChargeLimitsCronTriggerFactory(): CronTrigger {
    return [
        CronTriggerID.ChargeLimits,
        {}
    ];
}

export function init() {
    return {
        trigger_components: {
            [CronTriggerID.ChargeLimits]: {
                clone: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                config_builder: ChargeLimitsCronTriggerFactory,
                config_component: ChargeLimitsCronTriggerConfig,
                table_row: ChargeLimitsCronTriggerComponent,
                name: __("charge_limits.content.charge_limits_expiration")
            }
        }
    }
}
