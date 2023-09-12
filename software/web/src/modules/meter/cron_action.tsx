import { CronAction } from "../cron/cron_defs";

export interface MeterCronAction {
    0: CronAction.MeterReset,
    1: {}
}

import { __ } from "../../ts/translation";
import { Cron } from "../cron/main";
import { CronComponent, cron_action,cron_action_components } from "../cron/api";

function MeterResetCronActionComponent(_: cron_action): CronComponent {
    return {
        text: "",
        fieldNames: [],
        fieldValues: []
    };
}

function MeterResetCronActionConfigComponent(_: Cron, __: cron_action): any {
    return [];
}

function MeterResetCronActionConfigFactory(): cron_action {
    return [
        CronAction.MeterReset as any,
        {}
    ];
}

export function init() {
    cron_action_components[CronAction.MeterReset] = {
        config_builder: MeterResetCronActionConfigFactory,
        config_component: MeterResetCronActionConfigComponent,
        table_row: MeterResetCronActionComponent,
        name: __("meter.content.meter_reset"),
        require_feature: "meter"
    };
}
