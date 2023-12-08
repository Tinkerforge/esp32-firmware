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
import { CronAction } from "../cron/types";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import { InputFloat } from "../../ts/components/input_float";
import * as API from "../../ts/api";
import * as util from "../../ts/util";

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
    },
];

export type EMContactorCronAction = [
    CronActionID.EMRelaySwitch,
    {
        state: boolean;
    },
];

export type EMLimitMaxCurrentCronAction = [
    CronActionID.EMLimitMaxCurrent,
    {
        current: number;
    },
];

export type EMBlockChargeCronAction = [
    CronActionID.EMBlockCharge,
    {
        slot: number;
        block: boolean;
    },
];

function get_em_phase_switch_table_children(action: EMPhaseSwitchCronAction) {
    return __("energy_manager.cron.cron_action_text")(action[1].phases_wanted);
}

function get_em_phase_switch_edit_children(action: EMPhaseSwitchCronAction, on_action: (action: CronAction) => void) {
    const phases: [string, string][] = [
        ['1', __("energy_manager.cron.single_phase")],
        ['3', __("energy_manager.cron.three_phase")],
    ]
    return [
        <FormRow label={__("energy_manager.cron.phases_wanted")}>
            <InputSelect
                items={phases}
                value={action[1].phases_wanted.toString()}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {phases_wanted: parseInt(v)}));
                }} />
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

function get_em_charge_mode_switch_table_children(action: EMChargeModeSwitchCronAction) {
    return __("energy_manager.cron.charge_mode_switch_action_text")(action[1].mode, API.get("energy_manager/config").default_mode);
}

function get_em_charge_mode_switch_edit_children(action: EMChargeModeSwitchCronAction, on_action: (action: CronAction) => void) {
    const modes: [string, string][] = [
        ['0', __("energy_manager.cron.fast")],
        ['1', __("energy_manager.cron.disabled")],
        ['2', __("energy_manager.cron.pv_excess")],
        ['3', __("energy_manager.cron.guaranteed_power")],
    ]

    modes.push(['4', __("energy_manager.cron.charge_mode_default") + " (" + modes[API.get("energy_manager/config").default_mode][1] + ")"])

    return [
        <FormRow label={__("energy_manager.cron.charge_mode")}>
            <InputSelect
                items={modes}
                value={action[1].mode.toString()}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {mode: parseInt(v)}));
                }} />
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

function get_em_contactor_table_children(action: EMContactorCronAction) {
    return __("energy_manager.cron.relay_action_text")(action[1].state);
}

function get_em_contactor_edit_children(action: EMContactorCronAction, on_action: (action: CronAction) => void) {
    const states: [string, string][] = [
        ['1', __("energy_manager.cron.relay_state_closed")],
        ['0', __("energy_manager.cron.relay_state_open")],
    ]

    return [
        <FormRow label={__("energy_manager.cron.relay_state")}>
            <InputSelect
                items={states}
                value={action[1].state ? '1' : '0'}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {state: v == '1'}));
                }} />
        </FormRow>
    ]
}

function new_em_contactor_config(): CronAction {
    return [
        CronActionID.EMRelaySwitch,
        {
            state: true,
        },
    ];
}

function get_em_limit_max_current_table_children(action: EMLimitMaxCurrentCronAction) {
    return __("energy_manager.cron.cron_limit_max_current_action_text")(action[1].current, API.get("charge_manager/config").maximum_available_current);
}

function get_em_limit_max_current_edit_children(action: EMLimitMaxCurrentCronAction, on_action: (action: CronAction) => void) {
    const items:[string, string][] = [
        ['0', __("energy_manager.cron.limit_max_current")],
        ['1', __("energy_manager.cron.reset_limit_max_current") + " (" + API.get("charge_manager/config").maximum_available_current / 1000 + "A)"]
    ]

    return [
        <FormRow label={__("energy_manager.cron.limit_mode")}>
            <InputSelect
                items={items}
                value={action[1].current == -1 ? '1' : '0'}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {current: '1' ? -1 : 0}));
                }} />
        </FormRow>,
        <FormRow label={__("energy_manager.cron.max_current")} hidden={action[1].current == -1}>
            <InputFloat
                value={action[1].current}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {current: v}));
                }}
                min={0}
                unit="A"
                digits={3} />
        </FormRow>
    ];
}

function new_em_limit_max_current_config(): CronAction {
    return [
        CronActionID.EMLimitMaxCurrent,
        {
            current: 0,
        },
    ];
}

function get_em_block_charge_table_children(action: EMBlockChargeCronAction) {
    return __("energy_manager.cron.cron_block_charge_action_text")(action[1].slot, action[1].block);
}

function get_em_block_charge_edit_children(action: EMBlockChargeCronAction, on_action: (action: CronAction) => void) {
    const items:[string, string][] = [
        ['0', __("energy_manager.cron.unblock_charge")],
        ['1', __("energy_manager.cron.block_charge")],
    ]

    const slot_items: [string, string][] = [];
    for (let i = 0; i < 4; i++) {
        slot_items.push([i.toString(), __("energy_manager.cron.slot") + ' ' + i.toString()]);
    }

    return [
        <FormRow label={__("energy_manager.cron.slot")}>
            <InputSelect
                items={slot_items}
                value={action[1].slot.toString()}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {slot: parseInt(v)}));
                }}
            />
        </FormRow>,
        <FormRow label={__("energy_manager.cron.block_mode")}>
            <InputSelect
                items={items}
                value={action[1].block ? '1' : '0'}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {block: v == '1'}));
                }} />
        </FormRow>
    ];
}

function new_em_block_charge_config(): CronAction {
    return [
        CronActionID.EMBlockCharge,
        {
            slot: 0,
            block: true,
        },
    ];
}

export function init() {
    return {
        action_components: {
            [CronActionID.EMPhaseSwitch]: {
                name: __("energy_manager.cron.set_phases"),
                new_config: new_em_phase_switch_config,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                get_table_children: get_em_phase_switch_table_children,
                get_edit_children: get_em_phase_switch_edit_children,
            },
            [CronActionID.EMChargeModeSwitch]: {
                name: __("energy_manager.cron.charge_mode_switch"),
                new_config: new_em_charge_mode_switch_config,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                get_table_children: get_em_charge_mode_switch_table_children,
                get_edit_children: get_em_charge_mode_switch_edit_children,
            },
            [CronActionID.EMRelaySwitch]: {
                name: __("energy_manager.cron.switch_relay"),
                new_config: new_em_contactor_config,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                get_table_children: get_em_contactor_table_children,
                get_edit_children: get_em_contactor_edit_children,
            },
            [CronActionID.EMLimitMaxCurrent]: {
                name: __("energy_manager.cron.limit_max_current"),
                new_config: new_em_limit_max_current_config,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                get_edit_children: get_em_limit_max_current_edit_children,
                get_table_children: get_em_limit_max_current_table_children,
            },
            [CronActionID.EMBlockCharge]: {
                name: __("energy_manager.cron.block_charge"),
                new_config: new_em_block_charge_config,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                get_edit_children: get_em_block_charge_edit_children,
                get_table_children: get_em_block_charge_table_children,
            }
        }
    }
}
