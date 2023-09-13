import { CronActionID } from "../cron/cron_defs";

export type ChargeLimitsCronAction = [
    CronActionID.ChargeLimits,
    {
        duration: number,
        energy_wh: number
    }
];

import * as util from "../../ts/util";
import * as API from "../../ts/api"
import { h } from "preact"
import { __ } from "../../ts/translation";
import { Cron } from "../cron/main";
import { CronComponent, CronAction, cron_action_components } from "../cron/api";
import { InputSelect } from "../../ts/components/input_select";

function ChargeLimitsCronActionComponent(action: CronAction): CronComponent {
    const value = (action as ChargeLimitsCronAction)[1];
    const durations = [
        __("charge_limits.content.unlimited"),
        __("charge_limits.content.min15"),
        __("charge_limits.content.min30"),
        __("charge_limits.content.min45"),
        __("charge_limits.content.h1"),
        __("charge_limits.content.h2"),
        __("charge_limits.content.h3"),
        __("charge_limits.content.h4"),
        __("charge_limits.content.h6"),
        __("charge_limits.content.h8"),
        __("charge_limits.content.h12")
    ]

    let fieldNames = [
        __("charge_limits.content.duration")
    ];
    let fieldValues = [
        durations[value.duration]
    ];
    let ret =  __("charge_limits.content.duration") + ": " + durations[value.duration];
    if (API.hasFeature("meter")) {
        fieldNames = fieldNames.concat([__("charge_limits.content.energy")]);
        fieldValues = fieldValues.concat([(value.energy_wh != 0 ? value.energy_wh / 1000 + " kWh" : __("charge_limits.content.unlimited"))]);

        ret += ", " + __("charge_limits.content.energy") + ": " + (value.energy_wh != 0 ? value.energy_wh / 1000 + " kWh" : __("charge_limits.content.unlimited"));
    }

    return {
        text: ret,
        fieldNames: fieldNames,
        fieldValues: fieldValues
    };
}

function ChargeLimitsCronActionConfig(cron: Cron, action: CronAction) {
    const value = (action as ChargeLimitsCronAction)[1];
    const energy_items: [string, string][] = [
        ["0", __("charge_limits.content.unlimited")],
        ["5000", util.toLocaleFixed(5, 0) + " kWh"],
        ["10000", util.toLocaleFixed(10, 0) + " kWh"],
        ["15000", util.toLocaleFixed(15, 0) + " kWh"],
        ["20000", util.toLocaleFixed(20, 0) + " kWh"],
        ["25000", util.toLocaleFixed(25, 0) + " kWh"],
        ["30000", util.toLocaleFixed(30, 0) + " kWh"],
        ["40000", util.toLocaleFixed(40, 0) + " kWh"],
        ["50000", util.toLocaleFixed(50, 0) + " kWh"],
        ["60000", util.toLocaleFixed(60, 0) + " kWh"],
        ["70000", util.toLocaleFixed(70, 0) + " kWh"],
        ["80000", util.toLocaleFixed(80, 0) + " kWh"],
        ["90000", util.toLocaleFixed(90, 0) + " kWh"],
        ["100000", util.toLocaleFixed(100, 0) + " kWh"]
    ];

    const duration_items: [string, string][] = [
        ["0", __("charge_limits.content.unlimited")],
        ["1", __("charge_limits.content.min15")],
        ["2", __("charge_limits.content.min30")],
        ["3", __("charge_limits.content.min45")],
        ["4", __("charge_limits.content.h1")],
        ["5", __("charge_limits.content.h2")],
        ["6", __("charge_limits.content.h3")],
        ["7", __("charge_limits.content.h4")],
        ["8", __("charge_limits.content.h6")],
        ["9", __("charge_limits.content.h8")],
        ["10", __("charge_limits.content.h12")]
    ];

    const meter_entry = API.hasFeature("meter") ? [
        {
            name: __("charge_limits.content.energy"),
            value:  <InputSelect
                    items={energy_items}
                    value={value.energy_wh.toString()}
                    onValue={(v) => {
                        value.energy_wh = parseInt(v);
                        cron.setActionFromComponent(action)
                    }}/>
        }] : [];
    return [{
        name: __("charge_limits.content.duration"),
        value: <InputSelect
            items={duration_items}
            value={value.duration.toString()}
            onValue={(v) => {
                value.duration = parseInt(v);
                cron.setActionFromComponent(action);
            }}/>
    }].concat(meter_entry);
}

function ChargeLimitsCronActionFactory(): CronAction {
    return [
        CronActionID.ChargeLimits,
        {
            duration: 0,
            energy_wh: 0
        }
    ];
}

export function init() {
    cron_action_components[CronActionID.ChargeLimits] = {
        config_builder: ChargeLimitsCronActionFactory,
        config_component: ChargeLimitsCronActionConfig,
        table_row: ChargeLimitsCronActionComponent,
        name: __("charge_limits.content.charge_limits")
    };
}
