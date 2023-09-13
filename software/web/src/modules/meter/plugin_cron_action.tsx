import { CronActionID } from "../cron/cron_defs";
import { TableModalRow } from '../../ts/components/table'

export type MeterCronAction = [
    CronActionID.MeterReset,
    {}
]

import { __ } from "../../ts/translation";
import { Cron } from "../cron/main";
import { CronComponent, CronAction, cron_action_components } from "../cron/api";

function MeterResetCronActionComponent(_: CronAction): CronComponent {
    return {
        text: "",
        fieldNames: [],
        fieldValues: []
    };
}

function MeterResetCronActionConfigComponent(_: Cron, __: CronAction): TableModalRow[] {
    return [];
}

function MeterResetCronActionConfigFactory(): CronAction {
    return [
        CronActionID.MeterReset,
        {}
    ];
}

export function init() {
    cron_action_components[CronActionID.MeterReset] = {
        config_builder: MeterResetCronActionConfigFactory,
        config_component: MeterResetCronActionConfigComponent,
        table_row: MeterResetCronActionComponent,
        name: __("meter.content.meter_reset"),
        require_feature: "meter"
    };
}
