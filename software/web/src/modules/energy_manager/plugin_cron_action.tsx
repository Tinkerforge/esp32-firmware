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
import { CronActionID } from "../cron/cron_defs";
import { Cron } from "../cron/main";
import { CronAction } from "../cron/types";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "src/ts/components/input_select";

export type EMPhaseSwitchCronAction = [
    CronActionID.EMPhaseSwitch,
    {
        phases_wanted: number;
    },
];

export type EMChargeModeSwitchCronAction = [
    CronActionID.EMChargeModeSwitch,
    {
        mode: number;
    }
]

function get_em_phase_switch_table_children(action: CronAction) {
    let value = (action as EMPhaseSwitchCronAction)[1];
    return __("energy_manager.cron.cron_action_text")(value.phases_wanted);
}

function get_em_phase_switch_edit_children(cron: Cron, action: CronAction) {
    let value = (action as EMPhaseSwitchCronAction)[1];
    const phases: [string, string][] = [
        ['1', __('energy_manager.cron.single_phase')],
        ['3', __('energy_manager.cron.three_phase')],
    ]
    return [
        <FormRow label={__("energy_manager.cron.phases_wanted")}>
            <InputSelect
                value={value.phases_wanted.toString()}
                onValue={(v) => {
                    value.phases_wanted = parseInt(v);
                    cron.setActionFromComponent(action);
                }}
                items={phases}
            />
        </FormRow>
    ];
}

function new_em_phase_switch_config(): CronAction {
    return [
        CronActionID.EMPhaseSwitch,
        {
            phases_wanted: 1,
        },
    ];
}

function get_em_charge_mode_switch_table_children(action: CronAction) {
    let value = (action as EMChargeModeSwitchCronAction)[1];
    return __("energy_manager.cron.charge_mode_switch_action_text")(value.mode);
}

function get_em_charge_mode_switch_edit_children(cron: Cron, action: CronAction) {
    let value = (action as EMChargeModeSwitchCronAction)[1];
    const modes: [string, string][] = [
        ['0', __('energy_manager.cron.fast')],
        ['1', __('energy_manager.cron.disabled')],
        ['2', __('energy_manager.cron.pv_excess')],
        ['3', __('energy_manager.cron.guaranteed_power')],
        ['4', __('energy_manager.cron.charge_mode_default')]
    ]

    return [
        <FormRow label={__("energy_manager.cron.charge_mode")}>
            <InputSelect
                items={modes}
                value={value.mode.toString()}
                onValue={(v) => {
                    value.mode = parseInt(v);
                    cron.setActionFromComponent(action);
                }}
            />
        </FormRow>
    ]
}

function new_em_charge_mode_switch_config(): CronAction {
    return [
        CronActionID.EMChargeModeSwitch,
        {
            mode: 4,
        },
    ];
}

export function init() {
    return {
        action_components: {
            [CronActionID.EMPhaseSwitch]: {
                name: __("energy_manager.cron.set_phases"),
                new_config: new_em_phase_switch_config,
                get_table_children: get_em_phase_switch_table_children,
                get_edit_children: get_em_phase_switch_edit_children,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction
            },
            [CronActionID.EMChargeModeSwitch]: {
                name: __("energy_manager.cron.charge_mode_switch"),
                new_config: new_em_charge_mode_switch_config,
                get_table_children: get_em_charge_mode_switch_table_children,
                get_edit_children: get_em_charge_mode_switch_edit_children,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction
            }
        }
    }
}
