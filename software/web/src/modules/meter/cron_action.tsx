export interface MeterCronAction {
    0: 5,
    1: {}
}

import { __ } from "src/ts/translation";
import { Cron } from "../cron/main";
import { cron_action,cron_action_components } from "../cron/api";
import * as API from "../../ts/api"

function MeterResetCronActionComponent(_: cron_action) {
    return "";
}

function MeterResetCronActionConfigComponent(_: Cron, __: cron_action): any {
    return [];
}

function MeterResetCronActionConfigFactory(): cron_action {
    return [
        5 as any,
        {}
    ];
}

cron_action_components[5] = {
    config_builder: MeterResetCronActionConfigFactory,
    config_component: MeterResetCronActionConfigComponent,
    table_row: MeterResetCronActionComponent,
    name: __("meter.content.meter_reset"),
    require_feature: "meter"
}
