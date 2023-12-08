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
import { CronTrigger } from "../cron/types";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import * as util from "../../ts/util";

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
    {
        contactor_okay: boolean;
    },
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

function get_em_input_three_table_children(trigger: EMInputThreeCronTrigger) {
    return __("energy_manager.cron.cron_input_text")(3, trigger[1].state);
}

function get_em_input_three_edit_children(trigger: EMInputThreeCronTrigger, on_trigger: (trigger: CronTrigger) => void) {
    return [
        <FormRow label={__("energy_manager.cron.state")}>
            <InputSelect
                value={trigger[1].state == true ? '1' : '0'}
                items = {[
                    ['0', __("energy_manager.cron.open")],
                    ['1', __("energy_manager.cron.closed")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {state: v === '1'}));
                }}
            />
        </FormRow>,
    ];
}

function new_em_input_three_config(): CronTrigger {
    return [
        CronTriggerID.EMInputThree,
        {
            state: false,
        },
    ];
}

function get_em_input_four_table_children(trigger: EMInputFourCronTrigger) {
    return __("energy_manager.cron.cron_input_text")(4, trigger[1].state);
}

function get_em_input_four_edit_children(trigger: EMInputFourCronTrigger, on_trigger: (trigger: CronTrigger) => void) {
    return [
        <FormRow label={__("energy_manager.cron.state")}>
            <InputSelect
                value={trigger[1].state ? '1' : '0'}
                items = {[
                    ['0', __("energy_manager.cron.open")],
                    ['1', __("energy_manager.cron.closed")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {state: v === '1'}));
                }} />
        </FormRow>
    ];
}

function new_em_input_four_config(): CronTrigger {
    return [
        CronTriggerID.EMInputFour,
        {
            state: false,
        },
    ];
}

function get_em_phase_switch_table_children(trigger: EMPhaseSwitchCronTrigger) {
    return __("energy_manager.cron.cron_phase_switch_text")(trigger[1].phase);
}

function get_em_phase_switch_edit_children(trigger: EMPhaseSwitchCronTrigger, on_trigger: (trigger: CronTrigger) => void) {
    return [
        <FormRow label={__("energy_manager.cron.phase")}>
            <InputSelect
                value={trigger[1].phase.toString()}
                items = {[
                    ['1', __("energy_manager.cron.single_phase")],
                    ['3', __("energy_manager.cron.three_phase")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {phase: parseInt(v)}));
                }} />
        </FormRow>
    ];
}

function new_em_phase_switch_config(): CronTrigger {
    return [
        CronTriggerID.EMPhaseSwitch,
        {
            phase: 1,
        },
    ];
}

function get_em_contactor_monitoring_table_children(trigger: EMContactorMonitoringCronTrigger) {
    return __("energy_manager.cron.cron_contactor_monitoring_text")(trigger[1].contactor_okay);
}

function get_em_contactor_monitoring_edit_children(trigger: EMContactorMonitoringCronTrigger, on_trigger: (trigger: CronTrigger) => void) {
    return [
        <FormRow label={__("energy_manager.cron.contactor_monitoring_state")}>
            <InputSelect
                value={trigger[1].contactor_okay ? '1' : '0'}
                items = {[
                    ['0', __("energy_manager.cron.contactor_error")],
                    ['1', __("energy_manager.cron.contactor_okay")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {contactor_okay: v === '1'}));
                }} />
        </FormRow>
    ];
}

function new_em_contactor_monitoring_config(): CronTrigger {
    return [
        CronTriggerID.EMContactorMonitoring,
        {
            contactor_okay: false,
        },
    ];
}

function get_em_power_available_table_children(trigger: EMPowerAvailableCronTrigger) {
    return __("energy_manager.cron.cron_power_available_text")(trigger[1].power_available);
}

function get_em_power_available_edit_children(trigger: EMPowerAvailableCronTrigger, on_trigger: (trigger: CronTrigger) => void) {
    return [
        <FormRow label={__("energy_manager.cron.power")}>
            <InputSelect
                value={trigger[1].power_available ? '1' : '0'}
                items = {[
                    ['0', __("energy_manager.cron.not_available")],
                    ['1', __("energy_manager.cron.available")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {power_available: v === '1'}));
                }} />
        </FormRow>
    ]
}

function new_em_power_available_config(): CronTrigger {
    return [
        CronTriggerID.EMPowerAvailable,
        {
            power_available: false,
        },
    ];
}

function get_em_grid_power_draw_table_children(trigger: EMGridPowerDrawCronTrigger) {
    return __("energy_manager.cron.cron_grid_power_draw_text")(trigger[1].drawing_power);
}

function get_em_grid_power_draw_edit_children(trigger: EMGridPowerDrawCronTrigger, on_trigger: (trigger: CronTrigger) => void) {
    return [
        <FormRow label={__("energy_manager.cron.power")}>
            <InputSelect
                value={trigger[1].drawing_power ? '1' : '0'}
                items = {[
                    ['0', __("energy_manager.cron.feeding")],
                    ['1', __("energy_manager.cron.drawing")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {drawing_power: v === '1'}));
                }} />
        </FormRow>
    ]
}

function new_em_grid_power_draw_config(): CronTrigger {
    return [
        CronTriggerID.EMGridPowerDraw,
        {
            drawing_power: false,
        },
    ];
}

export function init() {
    return {
        trigger_components: {
            [CronTriggerID.EMInputThree]: {
                name: __("energy_manager.cron.input")(3),
                new_config: new_em_input_three_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_table_children: get_em_input_three_table_children,
                get_edit_children: get_em_input_three_edit_children,
            },
            [CronTriggerID.EMInputFour]: {
                name: __("energy_manager.cron.input")(4),
                new_config: new_em_input_four_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_table_children: get_em_input_four_table_children,
                get_edit_children: get_em_input_four_edit_children,
            },
            [CronTriggerID.EMPhaseSwitch]: {
                name: __("energy_manager.cron.phase_switch"),
                new_config: new_em_phase_switch_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_table_children: get_em_phase_switch_table_children,
                get_edit_children: get_em_phase_switch_edit_children,
            },
            [CronTriggerID.EMContactorMonitoring]: {
                name: __("energy_manager.cron.contactor_monitoring"),
                new_config: new_em_contactor_monitoring_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_table_children: get_em_contactor_monitoring_table_children,
                get_edit_children: get_em_contactor_monitoring_edit_children,
            },
            [CronTriggerID.EMPowerAvailable]: {
                name: __("energy_manager.cron.power_available"),
                new_config: new_em_power_available_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_table_children: get_em_power_available_table_children,
                get_edit_children: get_em_power_available_edit_children,
            },
            [CronTriggerID.EMGridPowerDraw]: {
                name: __("energy_manager.cron.grid_power_draw"),
                new_config: new_em_grid_power_draw_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_table_children: get_em_grid_power_draw_table_children,
                get_edit_children: get_em_grid_power_draw_edit_children,
            },
        },
    };
}
