export interface ChargeLimitsCronAction {
    0: 8,
    1: {
        duration: number,
        energy_wh: number
    }
}

import * as util from "../../ts/util";
import * as API from "../../ts/api"
import { h } from "preact"
import { __ } from "src/ts/translation";
import { Cron } from "../cron/main";
import { CronComponent, cron_action, cron_action_components } from "../cron/api";
import { InputSelect } from "src/ts/components/input_select";

function ChargeLimitsCronActionComponent(cron: cron_action): CronComponent {
    const props = (cron as any as ChargeLimitsCronAction)[1];
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
        durations[props.duration]
    ];
    let ret =  __("charge_limits.content.duration") + ": " + durations[props.duration];
    if (API.hasFeature("meter")) {
        fieldNames = fieldNames.concat([__("charge_limits.content.energy")]);
        fieldValues = fieldValues.concat([(props.energy_wh != 0 ? props.energy_wh / 1000 + " kWh" : __("charge_limits.content.unlimited"))]);

        ret += ", " + __("charge_limits.content.energy") + ": " + (props.energy_wh != 0 ? props.energy_wh / 1000 + " kWh" : __("charge_limits.content.unlimited"));
    }

    return {
        text: ret,
        fieldNames: fieldNames,
        fieldValues: fieldValues
    };
}

function ChargeLimitsCronActionConfig(cron_object: Cron, props: cron_action) {
    const state = props as any as ChargeLimitsCronAction;
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
                    value={state[1].energy_wh.toString()}
                    onValue={(v) => {
                        state[1].energy_wh = parseInt(v);
                        cron_object.setActionFromComponent(state as any)
                    }}/>
        }] : [];
    return [{
        name: __("charge_limits.content.duration"),
        value: <InputSelect
            items={duration_items}
            value={state[1].duration.toString()}
            onValue={(v) => {
                state[1].duration = parseInt(v);
                cron_object.setActionFromComponent(state as any);
            }}/>
    }].concat(meter_entry);
}

function ChargeLimitsCronActionFactory(): cron_action {
    return [
        8 as any,
        {
            duration: 0,
            energy_wh: 0
        }
    ];
}

cron_action_components[8] = {
    config_builder: ChargeLimitsCronActionFactory,
    config_component: ChargeLimitsCronActionConfig,
    table_row: ChargeLimitsCronActionComponent,
    name: __("charge_limits.content.charge_limits")
};
