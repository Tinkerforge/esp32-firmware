import { CronTrigger } from "../cron/cron_defs";

export interface ChargeLimitsCronTrigger {
    0: CronTrigger.ChargeLimits,
    1: {}
}

import { __ } from "../../ts/translation";
import { Cron } from "../cron/main";
import { CronComponent, cron_trigger, cron_trigger_components } from "../cron/api";

// TODO: Think about a nice explanation why this does not need a config
function ChargeLimitsCronTriggerComponent(_: cron_trigger): CronComponent {
    return {
        text: "",
        fieldNames: [],
        fieldValues: []
    };
}

function ChargeLimitsCronTriggerConfig(_: Cron, __: cron_trigger): any[] {
    return []
}

function ChargeLimitsCronTriggerFactory(): cron_trigger {
    return [CronTrigger.ChargeLimits as any, {}];
}

export function init() {
    cron_trigger_components[CronTrigger.ChargeLimits] = {
        config_builder: ChargeLimitsCronTriggerFactory,
        config_component: ChargeLimitsCronTriggerConfig,
        table_row: ChargeLimitsCronTriggerComponent,
        name: __("charge_limits.content.charge_limits_expiration")
    };
}
