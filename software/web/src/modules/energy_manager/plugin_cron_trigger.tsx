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
import { CronTriggerID } from "../cron/cron_defs";
import { Cron } from "../cron/main";
import { CronTrigger } from "../cron/types";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";

export type EMInputThreeCronTrigger = [
    CronTriggerID.EMInputThree,
    {
        state: boolean;
    },
];

export type EMInputFourCronTrigger = [
    CronTriggerID.EMInputFour,
    {
        state: boolean;
    },
];

export type EMPhaseSwitchCronTrigger = [
    CronTriggerID.EMPhaseSwitch,
    {
        phase: number;
    },
];

export type EMContactorMonitoringCronTrigger = [
    CronTriggerID.EMContactorMonitoring,
    {},
];

export type EMPowerAvailableCronTrigger = [
    CronTriggerID.EMPowerAvailable,
    {
        power_available: boolean;
    },
];

export type EMGridPowerDrawCronTrigger = [
    CronTriggerID.EMGridPowerDraw,
    {
        drawing_power: boolean;
    },
];

function get_em_input_three_table_children(trigger: CronTrigger) {
    let value = (trigger as EMInputThreeCronTrigger)[1];
    return __("energy_manager.cron.cron_input_text")(3, value.state);
}

function get_em_input_three_edit_children(cron: Cron, trigger: CronTrigger) {
    let value = (trigger as EMInputThreeCronTrigger)[1];
    const states: [string, string][] = [
        ['0', __('energy_manager.cron.open')],
        ['1', __('energy_manager.cron.closed')],
    ];

    return [
        <FormRow label={__('energy_manager.cron.state')}>
            <InputSelect
                value={value.state == true ? '1' : '0'}
                items = {states}
                onValue={(v) => {
                    value.state = v === '1';
                    cron.setTriggerFromComponent(trigger);
                }}
            />
        </FormRow>,
    ];
}

function new_em_input_three_config(): CronTrigger {
    return [
        CronTriggerID.EMInputThree,
        {
            state: false
        }
    ];
}

function get_em_input_four_table_children(trigger: CronTrigger) {
    let value = (trigger as EMInputFourCronTrigger)[1];
    return __("energy_manager.cron.cron_input_text")(4, value.state);
}

function get_em_input_four_edit_children(cron: Cron, trigger: CronTrigger) {
    let value = (trigger as EMInputFourCronTrigger)[1];
    const states: [string, string][] = [
        ['0', __('energy_manager.cron.open')],
        ['1', __('energy_manager.cron.closed')],
    ];

    return [
        <FormRow label={__('energy_manager.cron.state')}>
            <InputSelect
                value={value.state == true ? '1' : '0'}
                items = {states}
                onValue={(v) => {
                    value.state = v === '1';
                    cron.setTriggerFromComponent(trigger);
                }}
            />
        </FormRow>,
    ];
}

function new_em_input_four_config(): CronTrigger {
    return [
        CronTriggerID.EMInputFour,
        {
            state: false
        }
    ];
}

function get_em_phase_switch_table_children(trigger: CronTrigger) {
    let value = (trigger as EMPhaseSwitchCronTrigger)[1];
    return __("energy_manager.cron.cron_phase_switch_text")(value.phase);
}

function get_em_phase_switch_edit_children(cron: Cron, trigger: CronTrigger) {
    let value = (trigger as EMPhaseSwitchCronTrigger)[1];
    const phases: [string, string][] = [
        ['1', __('energy_manager.cron.single_phase')],
        ['3', __('energy_manager.cron.three_phase')],
    ];

    return [
        <FormRow label={__('energy_manager.cron.phase')}>
            <InputSelect
                value={value.phase.toString()}
                items = {phases}
                onValue={(v) => {
                    value.phase = parseInt(v);
                    cron.setTriggerFromComponent(trigger);
                }}
            />
        </FormRow>,
    ];
}

function new_em_phase_switch_config(): CronTrigger {
    return [
        CronTriggerID.EMPhaseSwitch,
        {
            phase: 1
        }
    ];
}

function get_em_contactor_monitoring_table_children(trigger: CronTrigger) {
    return __('energy_manager.cron.cron_contactor_monitoring_text');
}

function get_em_contactor_monitoring_edit_children(cron: Cron, trigger: CronTrigger): h.JSX.Element[] {
    return [];
}

function new_em_contactor_monitoring_config(): CronTrigger {
    return [
        CronTriggerID.EMContactorMonitoring,
        {}
    ];
}

function get_em_power_available_table_children(trigger: CronTrigger) {
    let value = (trigger as EMPowerAvailableCronTrigger)[1];
    return __('energy_manager.cron.cron_power_available_text')(value.power_available);
}

function get_em_power_available_edit_children(cron: Cron, trigger: CronTrigger): h.JSX.Element[] {
    let value = (trigger as EMPowerAvailableCronTrigger)[1];
    const states: [string, string][] = [
        ['0', __('energy_manager.cron.not_available')],
        ['1', __('energy_manager.cron.available')],
    ];

    return [
        <FormRow label={__('energy_manager.cron.power')}>
            <InputSelect
                value={value.power_available == true ? '1' : '0'}
                items = {states}
                onValue={(v) => {
                    value.power_available = v === '1';
                    cron.setTriggerFromComponent(trigger);
                }}
            />
        </FormRow>
    ]
}

function new_em_power_available_config(): CronTrigger {
    return [
        CronTriggerID.EMPowerAvailable,
        {
            power_available: false
        }
    ];
}

function get_em_grid_power_draw_table_children(trigger: CronTrigger) {
    let value = (trigger as EMGridPowerDrawCronTrigger)[1];
    return __('energy_manager.cron.cron_grid_power_draw_text')(value.drawing_power);
}

function get_em_grid_power_draw_edit_children(cron: Cron, trigger: CronTrigger): h.JSX.Element[] {
    let value = (trigger as EMGridPowerDrawCronTrigger)[1];
    const states: [string, string][] = [
        ['0', __('energy_manager.cron.feeding')],
        ['1', __('energy_manager.cron.drawing')],
    ];

    return [
        <FormRow label={__('energy_manager.cron.power')}>
            <InputSelect
                value={value.drawing_power == true ? '1' : '0'}
                items = {states}
                onValue={(v) => {
                    value.drawing_power = v === '1';
                    cron.setTriggerFromComponent(trigger);
                }}
            />
        </FormRow>
    ]
}

function new_em_grid_power_draw_config(): CronTrigger {
    return [
        CronTriggerID.EMGridPowerDraw,
        {
            drawing_power: false
        }
    ];
}

export function init() {
    return {
        trigger_components: {
            [CronTriggerID.EMInputThree]: {
                new_config: new_em_input_three_config,
                get_table_children: get_em_input_three_table_children,
                get_edit_children: get_em_input_three_edit_children,
                name: __('energy_manager.cron.input')(3),
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger
            },
            [CronTriggerID.EMInputFour]: {
                new_config: new_em_input_four_config,
                get_table_children: get_em_input_four_table_children,
                get_edit_children: get_em_input_four_edit_children,
                name: __('energy_manager.cron.input')(4),
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger
            },
            [CronTriggerID.EMPhaseSwitch]: {
                new_config: new_em_phase_switch_config,
                get_table_children: get_em_phase_switch_table_children,
                get_edit_children: get_em_phase_switch_edit_children,
                name: __('energy_manager.cron.phase_switch'),
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger
            },
            [CronTriggerID.EMContactorMonitoring]: {
                new_config: new_em_contactor_monitoring_config,
                get_table_children: get_em_contactor_monitoring_table_children,
                get_edit_children: get_em_contactor_monitoring_edit_children,
                name: __('energy_manager.cron.contactor_monitoring'),
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger
            },
            [CronTriggerID.EMPowerAvailable]: {
                new_config: new_em_power_available_config,
                get_table_children: get_em_power_available_table_children,
                get_edit_children: get_em_power_available_edit_children,
                name: __('energy_manager.cron.power_available'),
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger
            },
            [CronTriggerID.EMGridPowerDraw]: {
                new_config: new_em_grid_power_draw_config,
                get_table_children: get_em_grid_power_draw_table_children,
                get_edit_children: get_em_grid_power_draw_edit_children,
                name: __('energy_manager.cron.grid_power_draw'),
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger
            },
        }
    }
}
