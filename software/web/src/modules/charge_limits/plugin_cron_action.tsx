/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

import { h } from "preact";
import { __ } from "../../ts/translation";
import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { Cron } from "../cron/main";
import { CronActionID } from "../cron/cron_defs";
import { CronAction } from "../cron/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "src/ts/components/switch";

export type ChargeLimitsCronAction = [
    CronActionID.ChargeLimits,
    {
        reset: boolean,
        duration: number,
        energy_wh: number
    }
];

function get_charge_limits_table_children(action: CronAction) {
    const value = (action as ChargeLimitsCronAction)[1];
    const durations = [
        __("charge_limits.cron.unlimited"),
        __("charge_limits.cron.min15"),
        __("charge_limits.cron.min30"),
        __("charge_limits.cron.min45"),
        __("charge_limits.cron.h1"),
        __("charge_limits.cron.h2"),
        __("charge_limits.cron.h3"),
        __("charge_limits.cron.h4"),
        __("charge_limits.cron.h6"),
        __("charge_limits.cron.h8"),
        __("charge_limits.cron.h12"),
    ]

    return __("charge_limits.cron.cron_action_text")(durations[value.duration], value.energy_wh, value.reset);
}

function get_charge_limits_edit_children(cron: Cron, action: CronAction) {
    const value = (action as ChargeLimitsCronAction)[1];
    const energy_items: [string, string][] = [
        ["0", __("charge_limits.cron.unlimited")],
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
        ["100000", util.toLocaleFixed(100, 0) + " kWh"],
    ];

    const duration_items: [string, string][] = [
        ["0", __("charge_limits.cron.unlimited")],
        ["1", __("charge_limits.cron.min15")],
        ["2", __("charge_limits.cron.min30")],
        ["3", __("charge_limits.cron.min45")],
        ["4", __("charge_limits.cron.h1")],
        ["5", __("charge_limits.cron.h2")],
        ["6", __("charge_limits.cron.h3")],
        ["7", __("charge_limits.cron.h4")],
        ["8", __("charge_limits.cron.h6")],
        ["9", __("charge_limits.cron.h8")],
        ["10", __("charge_limits.cron.h12")],
    ];

    const meter_entry = API.hasFeature("meter") ? [
        <FormRow label={__("charge_limits.cron.energy")}>
            <InputSelect
                items={energy_items}
                value={value.energy_wh.toString()}
                onValue={(v) => {
                    value.energy_wh = parseInt(v);
                    cron.setActionFromComponent(action)
                }} />
        </FormRow>
    ] : [];

    return [
        <FormRow label={__("charge_limits.cron.reset")}>
            <Switch
                checked={value.reset}
                onClick={(v) => {
                    value.reset = !value.reset;
                    cron.setActionFromComponent(action);
                }} />
        </FormRow>,
        <FormRow label={__("charge_limits.cron.duration")}>
            <InputSelect
                items={duration_items}
                value={value.duration.toString()}
                onValue={(v) => {
                    value.duration = parseInt(v);
                    cron.setActionFromComponent(action);
                }} />
        </FormRow>
    ].concat(meter_entry);
}

function new_charge_limits_config(): CronAction {
    return [
        CronActionID.ChargeLimits,
        {
            reset: false,
            duration: 0,
            energy_wh: 0,
        },
    ];
}

export function init() {
    return {
        action_components: {
            [CronActionID.ChargeLimits]: {
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                new_config: new_charge_limits_config,
                get_edit_children: get_charge_limits_edit_children,
                get_table_children: get_charge_limits_table_children,
                name: __("charge_limits.cron.charge_limits"),
            },
        },
    };
}
