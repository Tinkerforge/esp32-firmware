/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
import { AutomationActionID } from "../automation/generated/automation_action_id.enum";
import { AutomationAction, PreInitResult } from "../automation/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { Collapse } from "react-bootstrap";
import { MeterLocation } from "../meters/generated/meter_location.enum";
import { MeterValueID } from "../meters/generated/meter_value_id";
import * as options from "../../options";

export type ChargeLimitsAutomationAction = [
    AutomationActionID.ChargeLimits,
    {
        restart: boolean;
        duration: number;
        energy_wh: number;
        soc_target_pct?: number;
    },
];

function get_charge_limits_table_children(action: ChargeLimitsAutomationAction) {
    const durations = [
        __("charge_limits.automation.unlimited"),
        __("charge_limits.automation.min15"),
        __("charge_limits.automation.min30"),
        __("charge_limits.automation.min45"),
        __("charge_limits.automation.h1"),
        __("charge_limits.automation.h2"),
        __("charge_limits.automation.h3"),
        __("charge_limits.automation.h4"),
        __("charge_limits.automation.h6"),
        __("charge_limits.automation.h8"),
        __("charge_limits.automation.h12"),
    ];

    return __("charge_limits.automation.automation_action_text")(durations[action[1].duration], action[1].energy_wh, options.PRODUCT_ID_IS_WARP4 ? action[1].soc_target_pct : 0, action[1].restart);
}

function get_charge_limits_edit_children(action: ChargeLimitsAutomationAction, on_action: (action: AutomationAction) => void) {
    const energy_items: [string, string][] = [
        ["-1", __("charge_limits.automation.unchanged")],
        ["0", __("charge_limits.automation.unlimited")],
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
        ["-1", __("charge_limits.automation.unchanged")],
        ["0", __("charge_limits.automation.unlimited")],
        ["1", __("charge_limits.automation.min15")],
        ["2", __("charge_limits.automation.min30")],
        ["3", __("charge_limits.automation.min45")],
        ["4", __("charge_limits.automation.h1")],
        ["5", __("charge_limits.automation.h2")],
        ["6", __("charge_limits.automation.h3")],
        ["7", __("charge_limits.automation.h4")],
        ["8", __("charge_limits.automation.h6")],
        ["9", __("charge_limits.automation.h8")],
        ["10", __("charge_limits.automation.h12")],
    ];

    let ev_has_soc = false;
    let soc_items: [string, string][] = [];
    if (options.PRODUCT_ID_IS_WARP4) {
        soc_items = [
            ["-1", __("charge_limits.automation.unchanged")],
            ["0",  __("charge_limits.automation.unlimited")],
            ...Array.from({length: 19}, (_, i) => [(10 + i * 5).toString(), (10 + i * 5) + " %"] as [string, string]),
        ];

        for (let i = 0; i < options.METERS_MAX_SLOTS; i++) {
            try {
                const meter_config = API.get_unchecked(`meters/${i}/config`);
                if (meter_config[1]?.location !== MeterLocation.EV)
                    continue;
                const value_ids = API.get_unchecked(`meters/${i}/value_ids`);
                if (Array.isArray(value_ids) && value_ids.indexOf(MeterValueID.StateOfCharge) >= 0) {
                    ev_has_soc = true;
                    break;
                }
            } catch {
                continue;
            }
        }
    }

    const meter_entry = API.hasFeature("meter") ? [
        <FormRow label={__("charge_limits.automation.energy")}>
            <InputSelect
                items={energy_items}
                value={action[1].energy_wh.toString()}
                onValue={(v) => {
                    const restart = v === "0" && action[1].duration === 0 && (!options.PRODUCT_ID_IS_WARP4 || action[1].soc_target_pct === 0) ? false : action[1].restart;
                    on_action(util.get_updated_union(action, {energy_wh: parseInt(v), restart: restart}));
                }} />
        </FormRow>,
    ] : [];

    const soc_entry = ev_has_soc ? [
        <FormRow label={__("charge_limits.automation.soc_target")}>
            <InputSelect
                items={soc_items}
                value={action[1].soc_target_pct.toString()}
                onValue={(v) => {
                    const restart = v === "0" && action[1].duration === 0 && action[1].energy_wh === 0 ? false : action[1].restart;
                    on_action(util.get_updated_union(action, {soc_target_pct: parseInt(v), restart: restart}));
                }} />
        </FormRow>,
    ] : [];

    const reset = [
        <Collapse in={action[1].energy_wh !== 0 || action[1].duration !== 0 || !!(options.PRODUCT_ID_IS_WARP4 && action[1].soc_target_pct !== 0)}>
            <div>
                <FormRow label={__("charge_limits.automation.restart")}>
                    <Switch
                        checked={action[1].restart}
                        onClick={(v) => {
                            on_action(util.get_updated_union(action, {restart: !action[1].restart}));
                        }} />
                </FormRow>
            </div>
        </Collapse>,
    ]

    return [
        <FormRow label={__("charge_limits.automation.duration")}>
            <InputSelect
                items={duration_items}
                value={action[1].duration.toString()}
                onValue={(v) => {
                    const restart = v === "0" && action[1].energy_wh === 0 && (!options.PRODUCT_ID_IS_WARP4 || action[1].soc_target_pct === 0) ? false : action[1].restart;
                    on_action(util.get_updated_union(action, {duration: parseInt(v), restart: restart}));
                }} />
        </FormRow>,
    ].concat(meter_entry).concat(soc_entry).concat(reset);
}

function new_charge_limits_config(): AutomationAction {
    return [
        AutomationActionID.ChargeLimits,
        {
            restart: false,
            duration: 0,
            energy_wh: 0,
            ...(options.PRODUCT_ID_IS_WARP4 ? {soc_target_pct: 0} : {}),
        },
    ];
}

export function pre_init(): PreInitResult {
    return {
        action_components: {
            [AutomationActionID.ChargeLimits]: {
                translation_name: () => __("charge_limits.automation.charge_limits"),
                new_config: new_charge_limits_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_charge_limits_edit_children,
                get_table_children: get_charge_limits_table_children,
            },
        },
    };
}

export function init() {
}
